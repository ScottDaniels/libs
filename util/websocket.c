/*
=================================================================================================
	(c) Copyright 1995-2013 By E. Scott Daniels. All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, are
	permitted provided that the following conditions are met:
	
   		1. Redistributions of source code must retain the above copyright notice, this list of
      		conditions and the following disclaimer.
		
   		2. Redistributions in binary form must reproduce the above copyright notice, this list
      		of conditions and the following disclaimer in the documentation and/or other materials
      		provided with the distribution.
	
	THIS SOFTWARE IS PROVIDED BY E. Scott Daniels ``AS IS'' AND ANY EXPRESS OR IMPLIED
	WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
	FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL E. Scott Daniels OR
	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
	SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
	ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
	
	The views and conclusions contained in the software and documentation are those of the
	authors and should not be interpreted as representing official policies, either expressed
	or implied, of E. Scott Daniels.
=================================================================================================
*/
/*
 ---------------------------------------------------------------------------------------------
*	Abstract: 	websocket
*	Mnemonic:	Functions to support a websocket server.
*				coded based on RFC 6455 (http://datatracker.ietf.org/doc/rfc6455/)
*				Globally unique id comes from RFC4122.  This library module requiers
*				sdaniels/si as that provides the underlying IP interface.
*
*				Some information about frames is included here:
*					http://lucumr.pocoo.org/2012/9/24/websockets-101/
*
*	Author:		E. Scott Daniels
*	Date:		22 November 2012
*
*
*	Mods:		06 Feb 2013 : Added some bullet proofing to reject odd session connection attempts.
*
 ---------------------------------------------------------------------------------------------
*/
#include <openssl/sha.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include <sdaniels/socket_if.h>		/* socket library  */
#include <sdaniels/sissl.h>			/* ssl overlay of the socket library stuff */

#include "ut.h"

#define GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"			/* from rfc 4122 */

/* current state of a session */
#define WSST_DEAD		0			/* just that, dead */
#define WSST_HANDSHAKE	1			/* building handshake */
#define WSST_COMPLETE	2			/* complete message received */
#define	WSST_LIVE		3			/* session established and is live */
#define WSST_CLOSING	4			/* session is being taken down */

#define MAX_SESSIONS	1025

typedef struct ws_session {
	int		flags;			/* WSFL_ constants */
	int		sid;			/* session id */
	int		state;
	char	*key;
	char	*rkey;			/* response key computed from the key received */
	char	*conn;
	char	*host;
	char	*orig;
	char	*ver;
	char	*cprotocol;		/* client rpotocol list */
	char	*sprotocol;		/* protocol we selected */
	char	*upgrade;
	char	*uagent;
	int 	error;
	int		nalloc;			/* number of bytes allocated in nalloc */
	char	*buf;			/* inbound message buffer */
	char	*ip;			/* insertion pointer */
	unsigned char *partial;	/* buffer to hold partial frame waiting for next input */
	int		pused;			/* number of bytes occupied in partial */
	int		plen;			/* length allocated to partial */
} WS_session;

typedef struct listen_data {
	WS_session *sessions[MAX_SESSIONS];
} Listen_data;

typedef struct frame {
	int	len;
	unsigned char *data;
} Frame;

static int (*usr_cb_msg)( int sid, unsigned char *buf, int len, void *data) = NULL;
static int (*usr_cb_conn)( int sid, unsigned char *buf ) = NULL;
static int (*usr_cb_disc)( int sid ) = NULL;
static int (*usr_cb_closing)( int sid ) = NULL;
static void *usr_cb_data = NULL;
static int cb_disc( void *data, int sid );

/* --------------- static/private functions ---------------------------- */
/* 
	create a partial buffer and hold it
*/
static void queue_partial( WS_session *sp, unsigned char *buffer, int len )
{
	sp->plen = len + 1024;
	sp->partial = (unsigned char *) malloc( sizeof( unsigned char ) * sp->plen );
	if( ! sp->partial )
	{
		fprintf( stderr, "[ABORT]\tunable to allocate partial buffer %d char\n", len );
		exit( 1 );	
	}

	
	sp->pused = len;
	memcpy( sp->partial, buffer, len );
}

/*
	add a buffer to existing partial buffer	
*/
static int add2partial( WS_session *sp, unsigned char *buf, int len )
{
	if( sp->pused + len > sp->plen ) 
	{
		sp->plen += len + 1;
		sp->partial = (char *) realloc( sp->partial, sp->plen );
	}

	memcpy( sp->partial + sp->pused, buf, len );
	sp->pused += len;

	return sp->pused; 
}

/*
	append the magic goo to the client key, compute the sha hash on it and then base64 
	encode it making the response key. Return the output. 
*/
static char *mk_respkey( char  *ckey )
{
	char	in[1024];
	char	sha[1024];

	if( !ckey )
		return NULL;			/* ckey is null if other side isn't a websock or buggered the handshake */

	strcpy( in, ckey );		/* smash togher their key and the global constant then b64 encode it */
	strcat( in, GUID );
	
	sha1_raw( in, strlen( in ), sha );							/* then take the sha1 of the combined stuff */
	return buf2base64( sha, NULL, SHA_DIGEST_LENGTH, 0 );		/* finally encode using base64 */
}

/*
	given a session block, build a handshake response. 
		HTTP/1.1 101 Switching Protocols
        Upgrade: websocket
        Connection: Upgrade
        Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=
*/
static char *mk_handshake( WS_session *sp )
{
	char	msg[1024];
	char	*rkey;

	if( (rkey = mk_respkey( sp->key )) == NULL )
		return NULL;

	if( ! sp->sprotocol )
		snprintf( msg, sizeof( msg ), "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept:%s\r\n\r\n", rkey );
	else
		snprintf( msg, sizeof( msg ), "HTTP/1.1 101 Switching Protocols\nUpgrade: websocket\nConnection: Upgrade\nSec-WebSocket-Accept:%s\nSec-WebSocket-Protocol: %s\n\n", rkey, sp->sprotocol );
	
	return strdup( msg );
}

/*
	parse the buffer for recognised handshake stuff:  <tag>: <value tokens>
	These are expected in any order:
		GET http/1.1 .... (ignored)
        Host: server.example.com
        Upgrade: websocket
        Connection: Upgrade
        Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==
        Origin: http://example.com
        Sec-WebSocket-Protocol: chat, superchat
        Sec-WebSocket-Version: 13
*/
static WS_session *add2header( WS_session *sp, char *buf )
{
	char	*value;
	
	while( isspace( *buf ) )
		buf++;

	if( ! *buf )
		return sp;

	value = strchr( buf, ':' );
	if( ! value )
		return sp;

	sp->error = 0;
	*(value++) = 0;
	while( isspace( *value ) )
		value++;

	//fprintf( stderr, "adding: (%s) (%s)\n", buf, value );
	switch( *buf )
	{
		case 'C':
				if( strcmp( buf, "Connection" ) == 0 )
				{
					sp->conn = strdup( value );
				}
				break;

		case 'G':			/* get -- we ignore */
				break;

		case 'H':
				if( strcmp( buf, "Host" ) == 0 )
				{
					sp->host = strdup( value );
				}
				break;

		case 'O':
				if( strcmp( buf, "Origin" ) == 0 )
				{
					sp->orig = strdup( value );
				}
				break;

		case 'S':
				if( strcmp( buf, "Sec-WebSocket-Protocol" ) == 0 )
				{
					sp->cprotocol = strdup( value );
				}
				else
				//if( strcmp( buf, "Sec-WebSocket-Version" ) == 0 )
				//{
					//sp->version = strdup( value );
				//}
				//else
				if( strcmp( buf, "Sec-WebSocket-Key" ) == 0  )
				{
					sp->key = strdup( value );
					sp->rkey = mk_respkey( sp->key );		/* build the reponse key */
				}
				break;

		case 'U':
				if( strcmp( buf, "Upgrade" ) == 0 )
				{
					sp->upgrade = strdup( value );
				}
				else
				if( strcmp( buf, "User-Agent" ) == 0 ) 
				{
					sp->uagent = strdup( value );
				}
				break;

		default:	
				break;
	}
}

/*
	take buffer from the session and parse it out capturing header things.
	if we encounter a null string (\n\n or \r\n\r\n) then we assume end of
	the header/handshake message and return 1 to indicate this. If we don't
	detect the end, then return 0.
*/
static int parse_for_handshake( WS_session *sess, unsigned char *buffer, int len, Listen_data *ldp )
{
	unsigned char	*bp;		/* pointer into the buffer of received stuff */
	unsigned char	*sp;		/* pointer at the start of the segment */
	unsigned char	hold;

	bp = buffer;
	while( 	bp < buffer + len )
	{
		sp = bp;
		while( bp < buffer + len && *bp >= 0x20 )		/* stop on 0d 0a or 00 */
			bp++;

		if( *bp < 0x20 )
			switch( *bp )			/* if termination point isn't an expected character; bail the session */
			{
				case 0:
				case 0x0d:
				case 0x0a:
					//fprintf( stderr, "parse_for: allow buffer to continue \n\n" );
					break;
	
				default:
					sess->error = 7;
					sess->key = strdup( "failure-force-disc" );			/* bogus should cause other side to disconnect */
					cb_disc( ldp, sess->sid );		/* this will drive user's disc callback since handshake has failed */
					fprintf( stderr, "parse_for: (len=%d) session error: bad/unexpected byte at %d 0x%02x\n", len, bp-buffer, (unsigned int) *bp );
					return 1;
					break;
			}

		if( !sess->error  &&  bp == sp )			/* null line we assume end of the message */
			return 1;								/* indicate complete handshake received */

		hold = *bp;
		*bp = 0;

		if( ! sess->error )
			add2header( sess, sp );
		if( sess->error )
		{
			cb_disc( ldp, sess->sid );		/* this will drive user's disc callback since handshake has failed */
			fprintf( stderr, "parse_for: (len=%d) session error: err=%d bp=%p sp=%p\n", len, sess->error, bp, sp );
			return 0;
		}

		bp++;						/* best case, just \n */
		if( hold == '\r' )
		{
			if( *bp	== '\n' )		/* allow single \r or \r\n */
				bp++;
		}	
	}

	return 0;
}

static int shake_back( int sid, WS_session *sp )
{
	char	*buf;		/* our response to the hand shake */
	
	if( (buf = mk_handshake( sp )) == NULL )
		return WSST_DEAD;

	SIsendt( sid, buf, strlen( buf ) );

	return WSST_LIVE;
}

/* ------------------ socket callback functions ---------------------------- */
static int cb_tdata( void *data, int sid, unsigned char *dgbuffer, int dglen )
{
	WS_session *sp;
	Listen_data	*ldp;
	unsigned char	*bp;
	unsigned char	*buffer;	/* current portion of datagram buffer we are working with */
	int		opcode;
	int		final;			/* indicator that this is the final frame in a multi frame message */
	int		mask;
	char 	*mkey;
	int		i;
	int		flen;			/* frame length */
	int		iflen;			/* frame len initially specified in the buffer */
	unsigned char	*next_frame;		/* if multiple frames in the datagram */
	unsigned char	*opartial = NULL;	/* previously allocated partial that must be freed before return */

	ldp = (Listen_data *) data;

	if( !ldp || (sp = ldp->sessions[sid]) == NULL ||  sp->state == WSST_DEAD  )
		return SI_OK;

	if( sp->state == WSST_HANDSHAKE )
	{
		if( parse_for_handshake( sp, dgbuffer, dglen, ldp ) )	/* returns true when we've received enough to shake back */
			sp->state = shake_back( sid, sp );					/* build and send response */
		if( sp->state == WSST_DEAD )
		{
			cb_disc( ldp, sid );		/* this will drive user's disc callback since handshake has failed */
			SIclose( sid );
		}
		return SI_OK;
	}

	if( sp->partial )			/* if a previous partial buffer exists, append this buffer before starting */
	{
		dglen = add2partial( sp, dgbuffer, dglen );			/* add and get the total len */
		opartial = dgbuffer = sp->partial;					/* pick it up as the inbound buffer; opartial mars it as deletable */
		sp->partial = NULL;							/* session doesn't need it any longer */
		sp->pused = sp->plen = 0;
	}

	next_frame = buffer = dgbuffer;
	while( next_frame < dgbuffer + dglen )
	{
		buffer = next_frame;

		if( buffer + 6 > dgbuffer + dglen )		/* minimal header and no data is 6 bytes - if this is beyond end, save as partial and go */
		{
			queue_partial( sp, buffer, (dgbuffer + dglen) - buffer );
			if( opartial )
				free( opartial );
			return SI_OK;
		}

		/* we've got a solid connection (we assume) after we respond to the handshake, "unpack" the frame */
		final = *buffer & 0x80;
		mask = *(buffer+1) & 0x80; 		/* data is masked -- most likely */
		bp = buffer;
		opcode = *bp & 0x0f;
		iflen = flen = (unsigned char) *(bp+1) & 0x7f;
	
		bp += 2;					/* point at mask key or the extended length */
		if( iflen > 125 )			/* extended length bits -- pick up first two (needed 125 and 126) */
		{
			flen = (*bp << 8) | *(bp+1);
			bp += 2;				/* point at mask key, or the extended, extended length */
		}

		if( iflen > 126 )			/* pick up last two  (126 only)  */
		{
			flen <<= 16;
			flen |= (*bp << 8) | (*(bp+1) << 8);
			if( flen == 127 )		/* super extended length bits */
				bp +=2;					/* point at masking key */
		}

		iflen = flen;
	
		if( mask )
		{
			mkey = bp;
			bp += 4;
		}

		if( buffer + ((bp - buffer) + flen) > dgbuffer + dglen )		/* not all data is in the buffer */
		{
			queue_partial( sp, buffer, (dgbuffer + dglen) - buffer );
			if( opartial )
				free( opartial );
			return SI_OK;
		}

	
		next_frame = bp + flen;
		//fprintf( stderr, ">>>websock/tdata: processing frame flen=%d  dglen=%d > tlen=%d\n", iflen, dglen, (bp - buffer) + flen );

		switch( opcode )
		{
			case 0x01:	/* utf-8 data -- decode it */
				if( flen + (sp->ip - sp->buf) > sp->nalloc )
				{
					int offset = sp->ip - sp->buf;			/* current offset to reset ip after realloc */
	
					sp->nalloc += flen * 2;
					sp->buf = realloc( sp->buf, sp->nalloc );
	
					if( sp->buf == NULL )
					{
						fprintf( stderr, "out of memory; cannot realloc buffer size %d\n", sp->nalloc );
						exit( 1 );
					}
	
					sp->ip = sp->buf + offset;
				}
	
				i = 0;
				for( ; flen > 0; flen-- )			/* we can make thid more efficient if its the final frame, and only frame, xlate it in place rather than copy */
				{
					*(sp->ip++) = *(bp++) ^ *(mkey+i);
					if( ++i > 3 )
						i = 0;	
				}
				*sp->ip = 0;
	
				if( final )
				{
					if( usr_cb_msg != NULL )
					{
						//fprintf( stderr, "ws_tcp driving callback with data received from %d, %d bytes (%s)\n", sp->sid, iflen, sp->buf );
						usr_cb_msg( sp->sid, sp->buf, iflen, usr_cb_data );		/* drive user callback */
					}
					sp->ip = sp->buf;				/* reset if this was last */
				}
				break;
	
			case 0x02:	/* binary data included ??? */
				break;
	
			case 0x08:	/* terminates connection */
				if( usr_cb_closing )
					usr_cb_closing( sp->sid );				/* disconnect pending, but the other side might take  a while to do it */
				//fprintf( stderr, "session termination message received\n" );
				ws_close( sid );
				break;
	
			case 0x09:	/* ping */
				break;
	
			case 0x10:	/* pong */
				break;
	
			default:		/* undefined */
				break;
		}
	}				/* end while there is a frame to process */
	
	return SI_OK;
}

static int cb_conn( unsigned char *data, int sid, unsigned char *buf )
{
	Listen_data	*ldp;
	WS_session	*sp;

	if( (ldp = (Listen_data *) data) == NULL )
		return SI_OK;

	if( sid >= MAX_SESSIONS )
	{
		fprintf( stderr, "abort: max sessions exceeded: %d\n", sid );
	}

	sp = ldp->sessions[sid] = (WS_session *) malloc( sizeof( WS_session ) ); 
	memset( sp, 0, sizeof( WS_session ) );
	sp->state = WSST_HANDSHAKE;
	sp->sid = sid;
	sp->nalloc = 4;		
	sp->buf = malloc( sizeof( char ) * sp->nalloc );
	sp->ip = sp->buf;
	sp->error = 0;

	if( usr_cb_conn != NULL )
	{
		//fprintf( stderr, "ws_tcp driving callback with data received from %d, %d bytes (%s)\n", sp->sid, iflen, sp->buf );
		usr_cb_conn( sp->sid, buf );
	}

	//fprintf( stderr, "websocket/conn: session accepted from: %s\n", buf );
	return SI_OK;
}

static int cb_disc( void *data, int sid )
{
	Listen_data *ldp;
	WS_session *sp;

	if( usr_cb_disc != NULL )			/* let the user know it went bye bye */
		usr_cb_disc( sid );

	if( (ldp = (Listen_data *) data) == NULL )
	{
		//fprintf( stderr, "error: disc without data\n" );
		return SI_OK;
	}

	if( (sp = ldp->sessions[sid]) == NULL )
	{
		//fprintf( stderr, "error: disc sessions[%d] is null\n", sid );
		return SI_OK;
	}

	//fprintf( stderr, "disc: session disconnected: %d\n", sid );
	ldp->sessions[sid] = NULL;
	if( sp->buf )
		free( sp->buf );
	free( sp );

	return SI_OK;
}

/*
	add a callback; the message callback is added on the 
	invocation of the listener. 
*/
extern void ws_add_cb( int type, int ((*usr_cb)( )) )
{
	switch( type )
	{
		case WSTY_CLOSING:
			usr_cb_closing = usr_cb;
			break;

		case WSTY_CONN:
			usr_cb_conn = usr_cb;
			break;

		case WSTY_DISC:
			usr_cb_disc = usr_cb;
			break;
	}
}

/* ------------------ websocket specific stuff ------------------------ */
/* 
	create a frame given a buffer. If len is 0
	then a null terminated string is assumed and this 
	will do a strlen to determine the number of characters
	to send. 

	Frame layout from standard: http://datatracker.ietf.org/doc/rfc6455/?include_text=1
  	     0           |   1           |       2       |           3
      0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
     +-+-+-+-+-------+-+-------------+-------------------------------+
     |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
     |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
     |N|V|V|V|       |S|             |   (if payload len==126/127)   |
     | |1|2|3|       |K|             |                               |
     +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
     |     Extended payload length continued, if payload len == 127  |
     + - - - - - - - - - - - - - - - +-------------------------------+
     |                               |Masking-key, if MASK set to 1  |
     +-------------------------------+-------------------------------+
     | Masking-key (continued)       |          Payload Data         |
     +-------------------------------- - - - - - - - - - - - - - - - +
     :                     Payload Data continued ...                :
     + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
     |                     Payload Data continued ...                |
     +---------------------------------------------------------------+
*/
static Frame *mk_frame( unsigned char *buf, int len, int type )
{
	Frame *frame;
	unsigned char *pd;			/* payload data insert point */
	int i;
	
	if( len <= 0 )
		len = strlen( buf );

	if( len > 4096 )
	{
		fprintf( stderr, "websocket/mk_frame: buffer too large, not sent: %d > 4096", len );
		return NULL;			/* fail for now */
	}
	
	frame = (Frame *) malloc( sizeof( Frame ) );
	frame->data = (unsigned char *) malloc( sizeof( unsigned char) * (len + 16) );		/* enough to hold full header */

	frame->data[0] = 0x81;		/* final frame, text data */
	if( len < 126 )
	{
		frame->data[1] = len & 0x7f;		/* ensure mask bit if off */
		pd = frame->data + 2;
	}
	else
	{
		frame->data[1] = 126;				/* right now we support only the 126 (not 127) two byte length */
		pd = &frame->data[4];
		frame->data[3] = len & 0xff;
		//fprintf( stderr, ">>>  len=%d >>8=%d  >>*&0xff=%d\n", len, len >>8, (len >>8) & 0xff );
		frame->data[2] = (len >> 8) & 0xff;
	}
	
	frame->len = len + (pd - frame->data);
	if( len )
		memcpy( pd, buf, len );
	return frame;
}

/* ------------------ ws interface functions ------------------------------- */

/*
	start a listener on the indicated port and register callbacks to 
	handle connections and messages.
*/
//void SIcbreg( int type, int ((*fptr)()), void * dptr )
extern void ws_listener( char *port, int ((*usr_cb)( int, unsigned char *, int, void *)), void *usr_env )
{
	Listen_data	*ldp;

	ldp = (Listen_data *) malloc( sizeof( *ldp ) );
	if( !ldp )
	{
		fprintf( stderr, "[ABORT]\tws_listen: cannot get memory for main listen data\n" );
		exit( 1 );
	}

	SIinitialise( SI_OPT_FG );
	SIlistener( TCP_DEVICE, port );
	SIcbreg( SI_CB_CONN, &cb_conn, ldp );    /* register callbacks */
	SIcbreg( SI_CB_DISC, &cb_disc, ldp );    
	//SIcbreg( SI_CB_KDATA, &cbkey, ldp );
	SIcbreg( SI_CB_CDATA, &cb_tdata, ldp );		/* tcp session data */
	//SIcbreg( SI_CB_RDATA, &cbudata, ldp );

	usr_cb_msg = usr_cb;
	usr_cb_data = usr_env;
	SIwait( );
}

extern int ws_send( int sid, unsigned char *buf, int len )
{
	Frame	*frame;

	//fprintf( stderr, "websocket/ws_send: sending %d bytes to session %d (%s)\n", len, sid, buf );

	if( (frame = mk_frame( buf, len, WSFT_TEXT )) == NULL )
		return 0;

	//fprintf( stderr, "websocket/listener: sending %d bytes to session %d (%s) (frame-len=%d) [2]=%02x\n", len, sid, buf, frame->len, frame->data[2] );
	SIsendt( sid, frame->data, frame->len );
	free( frame->data );
	free( frame );

	return len;
}

extern void ws_close( int sid )
{
	Frame	*frame;

	//fprintf( stderr, "sending close on sid: %d\n", sid );

	if( (frame = mk_frame( "", 0, WSFT_CLOSE )) == NULL )
		return;

	SIsendt( sid, frame->data, frame->len );
	SIclose( sid );
	free( frame->data );
	free( frame );

	return;
}

/* ----------------------------------------------------------------------------- */
#ifdef SELF_TEST
/* 
	This self test implements a simple repeater.  Sessions are accepted and 
	registered (up to 255) and everything that is received on one session is
	repeated onto all of the other sessions.
*/


#define MAX_SESSIONA 256

typedef struct group {		/* a group of sessions we repeat to */
	struct group *next;
	char *name;
	int	slist[MAX_SESSIONS];
	int	nsess;
} Group;


Group *glist = NULL;
Group *gidx[256];			/* index from session to group */


/*
	Msg_cb is driven when a mesage is received on the session. In this simple example
	it will send the received message out on all other sessions.
*/
int msg_cb( int sid, unsigned char *buf, int len, void *data )
{
	Group 	*gp;
	int		i;

	//fprintf( stderr, "msg_cb: got: %d bytes (%s)\n", len, buf );

	if( (gp = gidx[sid]) == NULL )
	{
		fprintf( stderr, "[WARN] websocket/msg_cb: group not mapped for sid %d\n", sid );
		return 0;
	}

	for( i = 0; i < MAX_SESSIONS; i++ )
	{
		if(  gp->slist[i]  &&  i != sid )
		{
			fprintf( stderr, "repeat message from %d to sid %d\n", sid, i );
			ws_send( i, buf, len );				/* repeat it */
		}
	}
	fprintf( stderr, "\n" );
}

/*
	Cv_disconnected is driven when a session is disconnected.
*/
int cb_disconnected( int sid )
{
	if( sid < 0 || sid > MAX_SESSIONS | gidx[sid] == NULL ) {
		return 0;
	}

	gidx[sid]->nsess--;				/* knock the session off of the group */
	fprintf( stderr, "session %d removed from group %s: group now has %d\n", sid, gidx[sid]->name, gidx[sid]->nsess );
	gidx[sid]->slist[sid] = 0;		/* deactivate session in the group */
	gidx[sid] = NULL;
	return 0;
}

/*
	Driven when a new session is accepted.  
*/
int cb_connected( int sid, char *buf )
{
	if( sid >= 0 && sid < MAX_SESSIONS ) {
		gidx[sid] = glist;				/* direct ref sid to group; default group initially */
		gidx[sid]->slist[sid] = 1;		/* activate session in the group */
		gidx[sid]->nsess++;		
		fprintf( stderr, "new session (%d) added to group %s: group now has %d\n", sid, gidx[sid]->name, gidx[sid]->nsess );
	} else {
		fprintf( stderr, "new session (%d) NOT added to group %s: exceeds limi.\n", sid, gidx[sid]->name );
		
	}
	
	return 0;
}

int main( int argc, char **argv )
{
	char	*out;

	glist = (Group *) malloc( sizeof( Group ) );
	glist->next = NULL;
	glist->name = strdup( "default" );
	memset( glist->slist, 0, sizeof( int ) * 256 );
	glist->nsess = 0;

	if( argc < 2  || !isdigit( *(argv[1]) )) {
		fprintf( stderr, "must enter listen port\n" );
		exit( 1 ); 
	}
	fprintf( stderr, "websocket self test v 1.2\n%d starting listener....\n", getpid() );

	ws_add_cb( WSTY_CONN, cb_connected );					// add our callback functions to the websocket interface
	ws_add_cb( WSTY_DISC, cb_disconnected );

	ws_listener(  argv[1], msg_cb, NULL );			// start the websocket listener
	fprintf( stderr, "finished listener....\n" );
}
#endif

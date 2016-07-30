/*
=================================================================================================
	(c) Copyright 1995-2011 By E. Scott Daniels. All rights reserved.

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


#ifndef __ut_h_
#define __ut_h_

#include <time.h>
#include <stdio.h>

/* ------------- base64 encoding -------------------- */
char *buf2base64( unsigned char *src, unsigned char *dest, int len, int dlen );

/* -------------- sha -----------------------------*/
extern unsigned char *sha1_buf( unsigned char *src, unsigned long len, unsigned char *hash );
extern unsigned char *sha1_raw( unsigned char *src, unsigned long len, unsigned char *hash );
extern unsigned char *sha256_mk_buffer( );
extern unsigned char *sha256_buf( unsigned char *src, unsigned long len, unsigned char *hash );
extern unsigned char *sha256_raw( unsigned char *src, unsigned long len, unsigned char *hash );

// ------------- tokenise -------------------------------------------------------------------------
extern int tokenise( unsigned char *buf, unsigned char sep, char **tokens, int maxtokens );

/* ------------- websocket -------------------------- */
#define WSTY_CONN		1			/* callback types -- connection */
#define WSTY_DISC		2			/* disc callback */
#define WSTY_MSG		3			/* message callback */
#define WSTY_CLOSING	4			/* farside has sent a close; disconnection expected */

#define WSFT_TEXT		0x81		/* normal text frame */
#define WSFT_TEXT_MORE	0x80		/* text, but not final */
#define WSFT_CLOSE		0x88		/* close frame */

extern void ws_add_cb( int type, int ((*usr_cb)( )) );
extern	void ws_close( int sid );
extern void ws_listener( char *port, int ((*usr_cb)( int, unsigned char *, int, void *)), void *usr_env );
extern int ws_send( int sid, unsigned char *buf, int len );

#endif


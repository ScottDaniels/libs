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
/*
***************************************************************************
*
*  Mnemonic: 	SIconnect
*  Abstract: 	Start a TCP/IP session with another process.
*  Parms:    
*            	abuf - 	Pointer to a string containing the process' address
*                   	The address is either ipv4 or ipv6 formmat with the
*						port number separated with a semicolon (::1;4444,
*						localhost;4444 climber;4444 129.168.0.4;4444).
*
*				conn_tolerence - a bit mask flag to indicate caller's level of tolerence
*						with the remote side certificate doesn't validate. This is set
*						by SIssl_initialise() so that the connect() parms are the 
*						same for both SI and SIssl libraries.
*							SISSL_NO_TOLERENCE -- fail connection on any error
*							SISSL_TOLERATE_INV -- tolerate an invalid cert
*							SISSL_TOLERATE_NAME - tolerate a name mismatch
*
*						If an issue that is tolerated is notied, we write a warning
*						to stderr. 
*				
*  Returns:  	The session number if all goes well, SI_ERROR if not.
*  Date:     	26 March 1995
*  Author:   	E. Scott Daniels
*
*  Mod:		08 Mar 2007 - conversion of sorts to support ipv6.
*			29 Jan 2013 - Added support for SSL sessions.
*			26 Jul 2016 - Remove need for global.
******************************************************************************
*/
#include "sisetup.h"

static int conn_tolerence = 0;			/* set by SIssl_initialise() */

/*
	Allows the value to be set in our static environment without the need for a 
	global.
*/
extern void SIssl_set_conn_tolerence( int val ) {
	conn_tolerence = val;
}

extern int SIconnect( char *abuf )
{
	extern struct ginfo_blk *gptr;
	int status;
	struct tp_blk *tpptr;       	/* pointer to new block */
	struct sockaddr *taddr;     	/* addr converted from abuf passed in */
	int alen = 0;					/* len of address struct */
	int fd = ERROR;             	/* file descriptor to return to caller */
	X509	*farside;				/* certificate details from the farside */
	char	far_name[1024];			/* name from the farside */
	char	*cp;					/* pointer into buffer */

	SIerrno = SI_ERR_HANDLE;
	if( gptr->magicnum != MAGICNUM )
		return( ERROR );                      /* no cookie - no connection */

	SIerrno = SI_ERR_ADDR;

	alen = SIgenaddr( abuf,  IPPROTO_TCP, 0, SOCK_STREAM, &taddr );		/* convert target addr to get family */
	if( alen <= 0 )
		return ERROR;

	SIerrno = SI_ERR_TPORT;
	tpptr = SIestablish( TCP_DEVICE, ";0", taddr->sa_family ); 	/* open new fd on any port, suitable to comm w/ abuf */

	if( tpptr != NULL )
	{
		SIerrno = SI_ERR_TP;
		if( connect( tpptr->fd, taddr, alen ) < OK )
		{
			close( tpptr->fd );         /* clean up fd and tp_block */
			free( tpptr );              /* connect error clean things up */
			free( taddr );
			fd = ERROR;                 /* send bad session id num back */
		}
		else                           /* connect ok */
		{
			tpptr->ssl = SSL_new( gptr->cctx );			/* create the ssl for the session using the connection context */
			SSL_set_fd( tpptr->ssl, tpptr->fd );		/* and attach it to the socket */
			fd = tpptr->fd;                 			/* save for return value */

			if( SSL_connect( tpptr->ssl ) <= 0 )				/* oops, something didn't go well */
			{
				//fprintf( stderr, "ERROR: sissl_connect: handshake error\n" );
				fd = ERROR;
			}


			if( fd != ERROR )		/* connection good, verify things */
			{
				if( SSL_get_verify_result( tpptr->ssl ) != X509_V_OK )
				{
					if( conn_tolerence & SISSL_TOLERATE_INV )
						fprintf( stderr, "WARNING: sissl_connect: remote process certificate does not verify\n" );
					else
					{
						fd = ERROR;
						fprintf( stderr, "ERROR: sissl_connect: remote process certificate does not verify\n" );
					}
				}
	
				farside = SSL_get_peer_certificate( tpptr->ssl );
				X509_NAME_get_text_by_NID( X509_get_subject_name( farside ), NID_commonName, far_name, sizeof( far_name ) );

				if( (cp = strchr( abuf, ';' )) )
					*cp = 0;
				if( strcasecmp( far_name, abuf ) )	/* not the same */
				{
					if( conn_tolerence & SISSL_TOLERATE_NAME )		/* but that is ok */
						fprintf( stderr, "WARNING: sissl_connect: remote certificate name mismatch: expected %s  found %s\n", abuf, far_name );
					else
					{
						fd = ERROR;
						fprintf( stderr, "ERROR: sissl_connect: remote certificate name mismatch: expected %s  found %s\n", abuf, far_name );
					}
				}
				if( cp )
					*cp = ';';
			}

			if( fd == ERROR )				/* some kind of ssl error -- cleanup and bail out now */
			{
				close( tpptr->fd );
				free( taddr );
				free( tpptr );
				return ERROR;
			}

			SIerrno = SI_RET_OK;				/* connected and passed cert check -- add to the list */
			tpptr->flags != TPF_SESSION;    		/* indicate we have a session here */
			tpptr->paddr = (struct sockaddr *) taddr;    	/* at partner addr */
			tpptr->next = gptr->tplist;     		/* add block to the list */
			if( tpptr->next != NULL )
				tpptr->next->prev = tpptr;      		/* if there - point back at new */
			gptr->tplist = tpptr;           		/* point at new head */
		}
	}                                  

 	return( fd );                    
}                                 

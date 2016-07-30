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
* Mnemonic: SIestablish
* Abstract: This routine will open and bind a file descriptor to a socket. 
*		the value of type indicates whether a TCP or UDP 
*		socket is created. abuf contains the ascii string of the 
*		desired address.  It is of the form:
*			localhost;port	to open a listen port (4)
*			::1;port	to open a listen port (6)
*			name;port	to open a port for connection (4or6)
*		A transport provider block is allocated, initialised
*		and returned to the caller.
* Parms:    
* 		type    - TCP_DEVICE or UDP_DEVICE constant,  converted to SOCK_STREAM or SOCK_DGRAM
* 		abuf    - Address buffer
*		family  - 0 when establishing an fd for listening
*				AF_INET or AF_INET6 when establishing an fd
*				that will be used to connect or send udp 
*				traffic through.  In the latter case, where the 
*				caller has the address of the target, it should:
*					1) convert target address to sockaddr
*					2) pull the family from the sockaddr
*					3) pass that family to this function
*
*			When establishing an fd for lisstening, the address
*			in abuf is used to determine the family. A listen 
*			address should be either 0.0.0.0;port or ::0;port.
*
* Returns:  	Pointer to a tp_blk structure, NULL if error.
* Date:     	26 March 1995
* Author:   	E. Scott Daniels
* Modified: 	19 Apr 1995 - To keep returned address of the port.
*				08 Mar 2007 - conversion for ipv6.
*
* NOTE:     It is assumed that the bind call will fail if it is unable to
*           allocate the requested, non-zero, port. This saves us the
*           trouble of checking the assigned port against the requested one.
***************************************************************************
*/
#include "sisetup.h"       /* include the necessary setup stuff */
#include <errno.h>

extern struct tp_blk *SIestablish( int type, char *abuf, int family )
{
	extern struct ginfo_blk *gptr;
	struct tp_blk *tptr;         /* pointer at new tp block */
	int status = OK;             /* processing status */
	struct sockaddr *addr;    	/* IP address we are requesting */
	int protocol;                /* protocol for socket call */
	char buf[256];               /* buffer to build request address in */
	int optval = 0;
	int alen = 0;

	tptr = (struct tp_blk *) SInew( TP_BLK );     /* new transport info block */

	if( tptr != NULL )
	{
		addr = NULL;

		switch( type )			/* things specifc to tcp or udp */
		{
			case UDP_DEVICE:            
				tptr->type = SOCK_DGRAM;
				protocol = IPPROTO_UDP;
				break;

			case TCP_DEVICE:
			default:                     
				tptr->type = SOCK_STREAM;  
				protocol = IPPROTO_TCP;   
		}     

		alen = SIgenaddr( abuf, protocol, family, tptr->type, &addr );	/* family == 0 for type that suits the address passed in */
		if( alen <= 0 )
		{
			fprintf( stderr, "siestablish: error generating an address struct for %s(abuf) %d(proto) %d(type): %s\n", abuf, protocol, tptr->type, strerror( errno ) );
			return NULL;
		}

		tptr->family = addr->sa_family;

   		if( (tptr->fd = socket( tptr->family, tptr->type, protocol )) >= OK )
		{                                          		/* socket established ok */
			optval = 1;
#ifdef SO_REUSEPORT
			setsockopt(tptr->fd, SOL_SOCKET, SO_REUSEPORT, (char *)&optval, sizeof( optval) ) ;
#endif

			status = bind( tptr->fd, (struct sockaddr *) addr, alen );
			if( status == OK )
				tptr->addr = addr;         	/* save address */
		}
		else
			status = ! OK;       		/* force bad return later */

		if( status != OK )    			/* socket or bind call failed - clean up stuff */
		{
			free( addr );
			SItrash( TP_BLK, tptr );       	/* free the trasnsport block */
			tptr = NULL;        		/* set to return nothing */
		}
	}

 	return tptr;        
}                       

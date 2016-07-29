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
/* X
**************************************************************************
*
*  Mnemonic: SIaddress
*  Abstract: This routine will convert a sockaddr_in structure to a
*            dotted decimal address, or visa versa.
*            If type == AC_TOADDR the src string may be: 
*            xxx.xxx.xxx.xxx.portnumber or host-name.portnumber
*            xxx.xxx.xxx.xxx.service[.protocol] or hostname;service[;protocol]
*            if protocol is not supplied then tcp is assumed.
*            hostname may be something like godzilla.moviemania.com
*  Parms:    src - Pointer to source buffer
*            dest- Pointer to dest buffer pointer 
*            type- Type of conversion AC_TODOT converts sockaddr to human readable. AC_TOADDR 
*		converts character buffer to sockaddr.
*  Returns:  Nothing.
*  Date:     19 January 1995
*  Author:   E. Scott Daniels
*
*  Modified: 22 Mar 1995 - To add support for ipx addresses.
*
*  CAUTION: The netdb.h header file is a bit off when it sets up the 
*           hostent structure. It claims that h_addr_list is a pointer 
*           to character pointers, but it is really a pointer to a list 
*           of pointers to integers!!!
*       
***************************************************************************
*/
#include "sisetup.h"      /* get necessary defs and other stuff */
#include <netdb.h>
#include <stdio.h>

/* 
	target: buffer with address  e.g.  192.168.0.1;4444  ;4444 (listen) ::1;4444
	family: PF_INET[6]  (let it be 0 to select based on addr in buffer 
	proto: IPPROTO_TCP IPPROTO_UDP
	type:   SOCK_STREAM SOCK_DGRAM

	returns length of struct pointed to by rap (return addr blockpointer)
*/
extern int SIgenaddr( char *target, int proto, int family, int socktype, struct sockaddr **rap )
{
	struct addrinfo hint;			/* hints to give getaddrinfo */
	struct addrinfo *list = NULL;		/* list of what comes back */
	int	ga_flags = 0;			/* flags to pass to getaddrinfo in hints */
	int	error = 0;
	int	rlen = 0;			/* length of the addr that rap points to on return */
	char	*tok;				/* token in buffer */
	char	*pstr;				/* port string */
	char	*dstr;				/* a copy of the users target that we can destroy */

    	dstr = strdup( (char *) target );	/* copy so we can destroy it with strtok */
	*rap = NULL;				/* ensure null incase something breaks */

	if( *dstr == ';' )		/* user passed in ;port -- so we assume this is for bind */
	{
		tok = NULL;
      		pstr = strtok( dstr, "; " );              	/* point at the host name */
		ga_flags = AI_PASSIVE;
	}
	else
	{
      		tok = strtok( dstr, "; " );              	/* point at the host name */
		pstr = strtok( NULL,"; " );			/* at service name or port number */
	}

	memset( &hint, 0, sizeof( hint  ) );
	hint.ai_family = family;			/* AF_TCP AF_TCP6...  let this be 0 to select best based on addr */
	hint.ai_socktype = socktype;			/* SOCK_DGRAM SOCK_STREAM */
	hint.ai_protocol = proto;			/* IPPORTO_TCP IPPROTO_UDP */
	hint.ai_flags = ga_flags;

#ifdef DEBUG
fprintf( stderr, "siaddress: calling getaddrinfo flags=%x proto=%d family=%d tok=%s pstr=%s\n", ga_flags, proto, family,  tok, pstr );
#endif
	if( (error = getaddrinfo( tok, pstr, &hint, &list )) )
	{
		fprintf( stderr, "error from getaddrinfo: target=%s(h) %s(port): error=%d=%s\n", tok, pstr, error, gai_strerror( error ) );
	}
	else
	{
		*rap = (struct sockaddr *) malloc(  list->ai_addrlen );		/* alloc a buffer and give address to caller */
		memcpy( *rap, list->ai_addr, list->ai_addrlen  );

		rlen = list->ai_addrlen;
		
		freeaddrinfo( list );		/* ditch system allocated memory */
	}

	free( dstr );
	return rlen;
}

/* 
	Given a source address convert from one form to another based on type constant.
	Type const == AC_TODOT   Convert source address structure to human readable string.
	Type const == AC_TOADDR6 Convert source string (host:port or ipv6 address [n:n...:n]:port) to an address struct
	Type const == AC_TOADDR  Convert source string (host:port or ipv4 dotted decimal address) to an address struct
*/
int SIaddress( void *src, void **dest, int type )
{
	struct sockaddr_in *addr;       /* pointer to the address */
	char *dstr;                     /* pointer to decimal string */
	unsigned char *num;             /* pointer at the address number */
	char wbuf[256];                 /* work buffer */
	int i;         
	char *tok;                      /* token pointer into dotted list */
	char *proto;                    /* pointer to protocol string*/
	struct hostent *hip;            /* host info from name server */
	struct servent *sip;            /* server info pointer */
	int	rlen = 0;		/* return len - len of address struct */

	switch( type )
	{
		case AC_TODOT:					/* convert from a struct to human readable */
   			addr = (struct sockaddr_in *) src;
   			num = (char *) &addr->sin_addr.s_addr;    /* point at the long */

			if( addr->sin_family == AF_INET6 )
			{
     				sprintf( wbuf, "%u:%u:%u:%u:%u:%u;%d", *(num+0), *(num+1), *(num+2), *(num+3), *(num+4), *(num+5) , (int) ntohs( addr->sin_port ) );
			}
			else
     				sprintf( wbuf, "%u.%u.%u.%u;%d", *(num+0), *(num+1), *(num+2), *(num+3), (int) ntohs(addr->sin_port) );

			*dest = (void *) strdup( wbuf );
			rlen = strlen( *dest );
			break;

  		case AC_TOADDR6:         		/* from hostname;port string to address for send etc */
			return SIgenaddr( src, PF_INET6, IPPROTO_TCP, SOCK_STREAM, (struct sockaddr **) dest );
			break; 

  		case AC_TOADDR:         		/* from dotted decimal to address struct ip4 */
			return SIgenaddr( src, PF_INET, IPPROTO_TCP, SOCK_STREAM, (struct sockaddr **) dest );
			break;
	}

	return rlen;
}


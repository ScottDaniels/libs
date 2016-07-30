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
******************************************************************************
*
*  Mnemonic: 	SIlistener
*  Abstract: 	Open a port on which connection requests (TCP) or datagrams (UDP)
*		can be received. SIlistener will open the ipv4/6 port based on
*		the address buffer passed in. The listener() obsoletes SIopen()
*		with regard to opening udp ports. 
*		Allows the user to open multiple secondary listening ports
*  Parms:   	type - TCP_DEVICE or UDP_DEVICE
*		abuf - buffer containing either 0.0.0.0;port or ::1;port
*
*  Returns: 	The file descriptor of the port, <0 if error
*  Date:    	26 March 1995 -- revised 13 Mar 2007 to support both ipv4 and 6
*  Author:  	E. Scott Daniels
*
*  Modified: 	10 May 1995 - To change SOCK_RAW to SOCK_DGRAM
*		14 Mar 2007 - To enhance for ipv6
******************************************************************************
*/
#include "sisetup.h"

extern int SIlistener( int type, char *abuf )
{
	extern struct ginfo_blk *gptr;
	struct tp_blk *tpptr;      		/* pointer into tp list */
	int status = SI_ERROR;     		/* status of processing */

	SIerrno = SI_ERR_HANDLE;
	if( gptr->magicnum == MAGICNUM )   			/* good cookie at the gptr address? */
	{
		SIerrno = SI_ERR_TP;
		tpptr = SIestablish(  type, abuf, 0 );
  
		if( tpptr != NULL )                          /* established a fd bound to the port ok */
		{                   	                        /* enable connection reqs */
			if( type == TCP_DEVICE )
			{
				if( (status = listen( tpptr->fd, 1 )) < OK )
					return ERROR;

				tpptr->flags |= TPF_LISTENFD;          /* flag it so we can search it out if needed */
			}

			tpptr->next = gptr->tplist;  		/* add to the list */
			if( tpptr->next != NULL )
				tpptr->next->prev = tpptr;
			gptr->tplist = tpptr;
			status = tpptr->fd;			/* return the fd of the listener */
		}        
	} 

	return status;
}                     /* SIclose */

#ifdef KEEP
/* deprecated. defaults to opening a v4 tcp port */
int SIlisten( int port )
{
	char buf[256];

	sprintf( buf, "0.0.0.0;%d", port );		/* by default -- ipv4 */
	return SIlistener( TCP_DEVICE, buf );
}
#endif

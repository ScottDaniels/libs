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
*            	addr - Pointer to a string containing the process' address
*                   	The address is either ipv4 or ipv6 formmat with the
*			port number separated with a semicolon (::1;4444,
*			localhost;4444 climber;4444 129.168.0.4;4444).
*  Returns:  	The session number if all goes well, SI_ERROR if not.
*  Date:     26 March 1995
*  Author:   E. Scott Daniels
*
*  Mod:		08 Mar 2007 - conversion of sorts to support ipv6
******************************************************************************
*/
#include "sisetup.h"

extern int SIconnect( char *abuf )
{
	extern struct ginfo_blk *gptr;
	int status;
	struct tp_blk *tpptr;       	/* pointer to new block */
	struct sockaddr *taddr;     	/* addr converted from abuf passed in */
	int alen = 0;			/* len of address struct */
	int fd = ERROR;             	/* file descriptor to return to caller */

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
			SIerrno = 0;
			tpptr->flags != TPF_SESSION;    		/* indicate we have a session here */
			tpptr->paddr = (struct sockaddr *) taddr;    	/* at partner addr */
			tpptr->next = gptr->tplist;     		/* add block to the list */
			if( tpptr->next != NULL )
			tpptr->next->prev = tpptr;      		/* if there - point back at new */
			gptr->tplist = tpptr;           		/* point at new head */
			fd = tpptr->fd;                 		/* save for return value */
		}
	}                                  

 	return( fd );                    
}                                 

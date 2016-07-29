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
******************************************************************************
*
*  Mnemonic: SIclose
*  Abstract: This routine allows the user application to close a port
*            associated with a file descriptor. The port is unbound from
*            the transport providor even if it is marked as a listen
*            port. If the fd passed in is less than 0 this routine assumes
*            that the UDP port opened during init is to be closed (user never
*            receives a fd on this one).
*  Parms:    gptr - The pointer to the ginfo block (SIHANDLE to the user)
*            fd   - FD to close.
*  Returns:  SI_OK if all goes well, SI_ERROR with SIerrno set if there is
*            a problem.
*  Date:     3 February 1995
*  Author:   E. Scott Daniels
*
*  Modified: 19 Feb 1995 - To set TP blk to drain if output pending.
*            10 May 1995 - To change SOCK_RAW to SOCK_DGRAM
*		22 Feb 2002 - To accept TCP_LISTEN_PORT or UDP_PORT as fd
******************************************************************************
*/
#include "sisetup.h"

extern int SIclose( int fd )
{
 extern struct ginfo_blk *gptr;

 struct tp_blk *tpptr;      /* pointer into tp list */
 int status = SI_ERROR;     /* status of processing */

 SIerrno = SI_ERR_HANDLE;
 if( gptr->magicnum == MAGICNUM )   /* good cookie at the gptr address? */
  {
   SIerrno = SI_ERR_SESSID;

   if( fd >= 0 )     /* if caller knew the fd number */
    {
     for( tpptr = gptr->tplist; tpptr != NULL && tpptr->fd != fd;
          tpptr = tpptr->next );   /* find the tppblock to close */
    }
   else  /* user did not know the fd - find first Listener or UDP tp blk */
   {
	if( fd == TCP_LISTEN_PORT )			/* close first tcp listen port; else first udp */
     		for( tpptr = gptr->tplist; tpptr != NULL && !(tpptr->flags&& TPF_LISTENFD); tpptr = tpptr->next );   
	else
    		for( tpptr = gptr->tplist; tpptr != NULL && tpptr->type != SOCK_DGRAM; tpptr = tpptr->next );
   }

   if( tpptr != NULL )
    {
     SIerrno = SI_ERR_TP;

     if( tpptr->squeue == NULL )   /* if nothing is queued to send... */
      {
       tpptr->flags |= TPF_UNBIND;   /* ensure port is unbound from tp */
	tpptr->flags |= TPF_DELETE;
	{
		int x = 1;

		setsockopt(tpptr->fd, SOL_SOCKET, SO_LINGER, (char *)&x, sizeof( x ) ) ;
	}
	close( tpptr->fd );
	tpptr->fd = -1;
	tpptr->type = -1;
			/* siterm now called in build poll if tp is marked delete */
       /*SIterm( tpptr );*/        /* cleanup and remove from the list */
      }
     else                       	/* stuff queued to send - mark port to drain */
      tpptr->flags |= TPF_DRAIN;   /* and we will term the port when q empty */

     status = SI_OK;               /* give caller a good status */
    }                              /* end if we found a tpptr */
  }                                /* end if the handle was good */

 return( status );                 /* send the status back to the caller */
}                                  /* SIclose */

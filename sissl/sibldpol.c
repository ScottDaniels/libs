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
*****************************************************************************
*  Mnemonic: SIbldpoll
*  Abstract: This routine will fill in the read and write fdsets in the
*            general info struct based on the current transport provider
*            list. Those tb blocks that have something queued to send will
*            be added to the write fdset. The fdcount variable will be set to
*            the highest sid + 1 and it can be passed to the select system
*            call when it is made.
*  Parms:    gptr  - Pointer to the general info structure
*  Returns:  Nothing
*  Date:     26 March 1995
*  Author:   E. Scott Daniels
*
*  Mods:	27 Jan 2013 -- changes to support ssl 
***************************************************************************
*/
#include "sisetup.h"      /* get definitions etc */

extern void SIbldpoll(  void )
{
 extern struct ginfo_blk *gptr;
 struct tp_blk *tpptr;             /* pointer into tp list */
 struct tp_blk *nextb;             /* pointer into tp list */


 gptr->fdcount = -1;               /* reset largest sid found */

 FD_ZERO( &gptr->readfds );        /* reset the read and write sets */
 FD_ZERO( &gptr->writefds );
 FD_ZERO( &gptr->execpfds );

 for( tpptr = gptr->tplist; tpptr != NULL; tpptr = nextb )
  {
	nextb = tpptr->next;
	if( tpptr->flags & TPF_DELETE )
	{
		SIterm( tpptr );
	}
	else
	{
   		if( tpptr->fd >= OK )                       /* if valid file descriptor */
    		{
     			if( tpptr->fd >= gptr->fdcount )	
      				gptr->fdcount = tpptr->fd + 1;     /* save largest fd (+1) for select */

     			// we ignore this because of sunos issues, so don't set it FD_SET( tpptr->fd, &gptr->execpfds );     /* set all fds for execpts */

     			if( !(tpptr->flags & TPF_DRAIN) )				/* if not draining */
     		 		FD_SET( tpptr->fd, &gptr->readfds );		/* set to test for input */

     			if( (tpptr->flags & TPF_SSL_HANDSHAKE)  ||  tpptr->squeue != NULL )                  /* stuff pending to send or handshake needs to write */
      				FD_SET( tpptr->fd, &gptr->writefds );   /* set flag to see if writable */
    		}
  	}
 }

if( gptr->kbfile >= 0 )     /* if we have input open */
	  FD_SET( 0, &gptr->readfds );    /* set to check stdin for read stuff */
}                                 /* SIbldpoll */

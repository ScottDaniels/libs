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
****************************************************************************
*
*  Mnemonic: SIsendt
*  Abstract: This routine will send a datagram to the TCP session partner
*            that is connected via the FD number that is passed in.
*            If the send would cause the process to block, the send is
*            queued on the tp_blk for the session and is sent later as
*            a function of the SIwait process.  If the buffer must be
*            queued, a copy of the buffer is created such that the
*            user program may free, or reuse, the buffer upon return.
*  Parms:    gptr - The pointer to the global info structure (SIHANDLE)
*            fd   - File descriptor (session number)
*            ubuf - User buffer to send.
*            ulen - Lenght of the user buffer.
*  Returns:  SI_OK if sent, SI_QUEUED if queued for later, NI_ERROR if error.
*  Date:     27 March 1995
*  Author:   E. Scott Daniels
*  Mod:		22 Feb 2002 - To better process queued data 
*
*****************************************************************************
*/
#include "sisetup.h"     /* get setup stuff */

goobers // error to see what we get in ci
extern int SIsendt( int fd, char *ubuf, int ulen )
{
	extern struct ginfo_blk *gptr;

	int status = SI_OK;         /* status of processing */
	fd_set writefds;            /* local write fdset to check blockage */
	fd_set execpfds;            /* exception fdset to check errors */
	struct tp_blk *tpptr;       /* pointer at the tp_blk for the session */
	struct ioq_blk *qptr;       /* pointer at i/o queue block */
	struct timeval time;        /* delay time parameter for select call */

	SIerrno = SI_ERR_HANDLE;

	if( gptr->magicnum == MAGICNUM )     /* ensure cookie is good */
	{                                   /* mmmm oatmeal, my favorite */
		SIerrno = SI_ERR_SESSID;

		for( tpptr = gptr->tplist; tpptr != NULL && tpptr->fd != fd; tpptr = tpptr->next ); /* find the block */

		if( tpptr != NULL )            /* found block for fd */
		{
			FD_ZERO( &writefds );       /* clear for select call */
			FD_SET( fd, &writefds );    /* set to see if this one was writable */
			FD_ZERO( &execpfds );       /* clear and set execptions fdset */
			FD_SET( fd, &execpfds );

			time.tv_sec = 0;            /* set both to 0 as we only want a poll */
			time.tv_usec = 0;

			/* poll fd - if write will not block then send immed */
			if( select( fd + 1, NULL, &writefds, &execpfds, &time ) > 0 )		/* see if it would block */
			{									/* nope - send data */
				SIerrno = SI_ERR_TP;
				if( FD_ISSET( fd, &execpfds ) )   /* error? */
				{
					SIterm( tpptr );   			/* clean up our portion of the session */
					status = SI_ERROR;       		/* and signal an error to caller */
				}
				else                       		/* no err, no blockage - send something */
				{
					if( tpptr->squeue )		
						SIsend( tpptr );	/* send the top queued block */
					else
					{
						return send( tpptr->fd, ubuf, (unsigned int) ulen, 0 );   /* done after send */
					}
				}
			}

			SIerrno = SI_ERR_NOMEM;

			if( (qptr = (ioq_blk *)SInew( IOQ_BLK )) != NULL )   /* alloc a queue block */
			{
				if( tpptr->sqtail == NULL )    /* if nothing on the queue */
	 			{
					tpptr->squeue = qptr;         /* simple add to the tp blk q */
					tpptr->sqtail = qptr;
	 			}
				else                           		/* else - add at end of the q */
				{
	 				tpptr->sqtail->next = qptr;		
					tpptr->sqtail = qptr;	
					qptr->next = NULL;		/* new block is the last one now */
				}    		                       /* end add block at end of queue */

				qptr->dlen = ulen;           /* copy info to queue block */
				qptr->data = (char *) malloc( ulen );  /* get buffer */
				memcpy( qptr->data, (const char*) ubuf, ulen );
	
				SIerrno = SI_QUEUED;                /* indicate queued to caller */
				status = SI_ERROR;		/* return value */
			}
		}							/* end if tpptr was not found */
	}								/* ginfo pointer was corrupted */

	return( status );
}                           /* SIsendt */

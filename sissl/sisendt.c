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

extern int SIsendt( int fd, char *ubuf, int ulen )
{
	extern struct ginfo_blk *gptr;
	static struct ioq_blk *qptr;       /* i/o queue block -- we keep if we allocated and didn't use */

	int status = SI_OK;         /* status of processing */
	fd_set writefds;            /* local write fdset to check blockage */
	fd_set execpfds;            /* exception fdset to check errors */
	struct tp_blk *tpptr;       /* pointer at the tp_blk for the session */
	struct timeval time;        /* delay time parameter for select call */

	SIerrno = SI_ERR_HANDLE;

	if( gptr->magicnum == MAGICNUM )     /* ensure cookie is good */
	{                                   /* mmmm oatmeal, my favorite */
		SIerrno = SI_ERR_SESSID;

		for( tpptr = gptr->tplist; tpptr != NULL && tpptr->fd != fd; tpptr = tpptr->next ); /* find the block */

		if( tpptr != NULL )            /* found block for fd */
		{
			SIerrno = SI_ERR_NOMEM;

			if( qptr != NULL  ||  (qptr = SInew( IOQ_BLK )) != NULL )   /* alloc a queue block */
			{
				SIerrno = SI_RET_OK;                /* assume all goes well */
				if( ulen > qptr->nalloc )
				{
					qptr->nalloc = ulen + 1024;           					/* copy info to queue block so if we must call ssl_send again we have exact same parameter */
					qptr->data = (char *) realloc( qptr->data, qptr->nalloc );			/* get/extend buffer */
				}
				qptr->dlen = ulen;									/* data length may be smaller if we're reusing the block */
				memcpy( qptr->data, (const char *) ubuf, ulen );

				if( tpptr->sqtail == NULL )    			/* if nothing on the queue -- send it */
	 			{
					if( send_ssl( tpptr, qptr ) )		/* returns true if ssl needs another write with same data */
					{
						tpptr->squeue = qptr;         /* simple add to the tp blk q */
						tpptr->sqtail = qptr;
						qptr = NULL;					/* we'll need to alloc one next time */
					}
	 			}
				else                           		/* else - add at end of the q */
				{
	 				tpptr->sqtail->next = qptr;		
					tpptr->sqtail = qptr;	
					qptr->next = NULL;				/* new block is the last one now */
					qptr = NULL;					/* we'll need a new one next time */
					SIerrno = SI_QUEUED;                /* indicate queued to caller */
					status = SI_ERROR;		/* return value */
				}    		                       /* end add block at end of queue */
			}
		}							/* end if tpptr was not found */
	}								/* ginfo pointer was corrupted */

	return( status );
}                

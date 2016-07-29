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
*  Mnemonic: SIsendu
*  Abstract: This routine will send a UDP datagram to the IP address
*            that is passed in with the buffer. The datagram is sent
*            on the first raw FD in the tp list. If sending the datagram
*            would cause the process to block, the datagram is queued
*            and will be sent as a part of the SIwait loop when the FD
*            unblocks. If the datagram is queued, a copy of the caller's
*            buffers is made such that the caller is free to reuse the
*            buffers upon return from this routine.
*  Parms:    gptr - The pointer to the global info structure (SIHANDLE)
*            abuf - Pointer to the dotted decimal address to send to
*            ubuf - User buffer to send.
*            ulen - Length of the user buffer.
*  Returns:  SI_OK if sent, SI_QUEUED if queued for later, SI_ERROR if error.
*  Date:     27 March 1995
*  Author:   E. Scott Daniels
*  Mod:		09 Mar 2007 - conversion to ipv6.
*
*****************************************************************************
*/
#include "sisetup.h"     /* get setup stuff */


extern int SIsendu( char *abuf, char *ubuf, int ulen )
{
	extern struct ginfo_blk *gptr;
	int	fd;                     /* file descriptor to send to */
	int	status = SI_OK;         /* status of processing */
	int	alen; 			/* length of addres structure returned by genaddr */
	struct	sockaddr *addr;      /* address to send to */
	struct	tp_blk *tpptr;       /* pointer at the tp_blk for the session */
	struct	ioq_blk *qptr;       /* pointer at i/o queue block */
	fd_set	writefds;            /* fdset for select call */
	struct	timeval time;        /* time to fill in for select call */

	SIerrno = SI_ERR_NOMEM;     /* set status incase of failure */

	addr = NULL;
	alen = SIgenaddr( abuf,  IPPROTO_UDP, 0, SOCK_DGRAM, &addr );	/* get address */

	SIerrno = SI_ERR_ADDR;
	if( alen <= 0 )
		return ERROR;

	SIerrno = SI_ERR_HANDLE;        /* next possible failure is bad cookie */

	if( gptr->magicnum == MAGICNUM && addr != NULL )  /* ensure cookie is good */
	{
		SIerrno = SI_ERR_SESSID;

		for( tpptr = gptr->tplist; tpptr != NULL && tpptr->type != SOCK_DGRAM; tpptr = tpptr->next ) ;    /* find the first udp (raw) block */

		if( tpptr != NULL )            /* found block for udp */
		{
			fd = tpptr->fd;           /* make easy access to fd */
	
			FD_ZERO( &writefds );     /* clear */
			FD_SET( fd, &writefds );  /* set for select to look at our fd */
			time.tv_sec = 0;          /* set time to 0 to force poll - no wait */
			time.tv_usec = 0;
		
			if( select( fd + 1, NULL, &writefds, NULL, &time ) > 0 ) /* write ok ? */
			{
				SIerrno = SI_ERR_TP;
				status = sendto( fd, ubuf, ulen, 0, addr, alen );
				free( addr );
				if( status == ulen )
					status = SI_OK;
			}
			else               /* write would block - build queue block and queue */
			{
				SIerrno = SI_ERR_NOMEM;
				
				if( (qptr = SInew( IOQ_BLK )) != NULL )   /* alloc a queue block */
				{
					if( tpptr->squeue == NULL )    		/* if nothing on the queue */
						tpptr->squeue = qptr;         	/* simple add to the tp blk q */
					else                           		/* else - add at end of the q */
					{
						for( qptr->next = tpptr->squeue; qptr->next->next != NULL;
							qptr->next = qptr->next->next );  /* find last block */
						qptr->next->next = qptr;   /* point the current last blk at new */
						qptr->next = NULL;         /* new block is the last one now */
					}                           /* end add block at end of queue */
				
					qptr->dlen = ulen;                    /* copy user's information  */
					qptr->data = (char *) malloc( ulen );    /* get buffer */
					memcpy( qptr->data, (const char*) ubuf, ulen );
					qptr->addr = addr;                             /* save address too */
					qptr->alen = alen;
					status = SI_QUEUED;                /* indicate queued to caller */
				}
				else
					status = SI_ERROR;
			}                           /* end else need to queue block */
		}                             /* end if tpptr was not found */
		else
			status = SI_ERROR;
	}                         /* end if a valid ginfo pointer passed in */
	else
		status = SI_ERROR;

	return status;
}


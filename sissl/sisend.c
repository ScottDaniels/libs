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
*  Mnemonic: SIsend
*  Abstract: This routine is called to send a buffer of data to a partner
*            if the buffer has been queued waiting for the session to
*            unblock. The top qio block is removed from the tp block's
*            queue and is sent. It is assumed that for UDP data the
*            unit data structure was created and contains the buffer and
*            address and is pointed to by the qio block. The block and
*            associated buffers are then freed.
*  Parms:    gptr - POinter to the global information block
*            tpptr- Pointer to the tp block
*  Returns:  Nothing.
*  Date:     27 March 1995
*  Author:   E. Scott Daniels
*  Mod: 	22 Feb 2002 - To support sendqueue tail 
*
******************************************************************************
*/
#include "sisetup.h"      /* get include files etc */

/*
	actually send a buffer to ssl for encryption and transmission on the 
	socket.  We return 1 if ssl wants the buffer to be written again 
	with the exact same parameters. 
*/
extern int send_ssl( struct tp_blk *tp, struct ioq_blk *qp )
{
	int	rc = 1;				/* assume retry */
	int	sslrc;

	sslrc = SSL_write( tp->ssl, qp->data, qp->dlen );
	switch( SSL_get_error( tp->ssl, sslrc ) )
	{
		case SSL_ERROR_NONE:				/* things written successfully; return 0 and let the buffer dequeue */
				rc = 0;
				break;

		case SSL_ERROR_WANT_WRITE:			/* would have blocked, need to try again later */
				break;

		case SSL_ERROR_WANT_READ:			/* handshake in process, set flag in the tp block and return retry */
				tp->flags |= TPF_SSL_HANDSHAKE;
				break;

		default:
				fprintf( stderr, "sisend/send_ssl: write error of some sort: fd= %d\n", tp->fd );
				/* close ? */
				break;
	}
	
	return rc;
}


extern void SIsend( struct tp_blk *tpptr )
{
	extern struct ginfo_blk *gptr;
	struct t_unitdata *udata;      /* pointer at UDP unit data */
	struct ioq_blk *qptr;          /* pointer at qio block for free */
	int status;

	if( tpptr->squeue == NULL )    		/* who knows why we were called */
		return;							/* nothing queued - just leave */

	if( tpptr->type == SOCK_DGRAM )                                /* udp send? */
		sendto( tpptr->fd, tpptr->squeue->data, tpptr->squeue->dlen, 0, tpptr->squeue->addr, sizeof( struct sockaddr ) );
	else                                                            /* else tcp*/
		if( send_ssl( tpptr, tpptr->squeue ) )				/* returns true if we must keep the block and try again */
			return;											/* leave it on the queue */

	if( tpptr->squeue->addr != NULL )
		free( tpptr->squeue->data );          /* loose address if udp */

	free( tpptr->squeue->data );           /* trash buffer or the udp block */
	qptr = tpptr->squeue;                  /* hold pointer for free */
	tpptr->squeue = tpptr->squeue->next;   /* next in queue becommes head */
	if( !tpptr->squeue )
		tpptr->sqtail = NULL;		/* no tail left either */

	free( qptr );

	if( (tpptr->flags & TPF_DRAIN) && tpptr->squeue == NULL )  /* done w/ drain? */
	{
		SIterm( tpptr );     /* trash the tp block */
	}
}

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
*******************************************************************************
*
*  Mnemonic: SInew
*  Abstract: This routine is responsible for alocating a new block based on
*            the block type and initializing it.
*  Parms:    type - Block id to create
*  Returns:  Pointer to the new block or NULL if not successful
*  Date:     26 March 1995
*  Author:   E. Scott Daniels
*  Mod:		22 Feb 2002 - To ensure new field in tp block is initialised 
*
******************************************************************************
*/
#include "sisetup.h"

extern void *SInew( int type )
{
	void *retptr;                  /* generic pointer for return */
	struct tp_blk *tpptr;          /* pointer at a new tp block */
	struct ginfo_blk *gptr;        /* pointer at gen info blk */
	struct ioq_blk *qptr;          /* pointer to an I/O queue block */

	switch( type )
	{
		case IOQ_BLK:              /* make an I/O queue block */
			if( (qptr = (struct ioq_blk *) malloc( sizeof( struct ioq_blk) )) != NULL )
			{
				qptr->addr = NULL;
				qptr->next = NULL;
				qptr->data = NULL;
				qptr->dlen = 0;
				qptr->nalloc = 0;
			}
			retptr = (void *) qptr;    /* set pointer for return */
			break;

		case TP_BLK:
			if( (tpptr = (struct tp_blk *) malloc( sizeof( struct tp_blk ) )) != NULL )
			{
				memset( tpptr, 0, sizeof( *tpptr ) );
				tpptr->fd = -1;
				tpptr->type = -1;
				tpptr->flags = TPF_UNBIND;   /* default to unbind on termination */
				tpptr->rbufsize = 4096;
				tpptr->rbuf = (char *) malloc( sizeof( char ) * tpptr->rbufsize );
				if( ! tpptr->rbuf )
				{
					fprintf( stderr, "sissl/sinew: unable to allocate memory: %d, buffer\n", type );
					exit( 1 );
				}
			}

			retptr = (void *) tpptr;   /* setup for later return */
			break;

		case GI_BLK:                /* create global info block */
			if( (gptr = (struct ginfo_blk *) malloc( sizeof( struct ginfo_blk ) )) != NULL )
			{
				gptr->magicnum = MAGICNUM;   /* inidicates valid block */
				gptr->childnum = 0;
				gptr->flags = 0;
				gptr->tplist = NULL;
				FD_ZERO( &gptr->readfds);      /* clear the fdsets */
				FD_ZERO( &gptr->writefds) ;
				FD_ZERO( &gptr->execpfds );
				gptr->kbfile = ERROR;          /* no keyboard file initially */
				gptr->rbuf = NULL;             /* no read buffer */
				gptr->cbtab = NULL;
				gptr->rbuflen = 0;
			}
			retptr = (void *) gptr;    /* set up for return at end */
			break;

		default:
			retptr = NULL;           /* bad type - just return null */
			break;
	}                          /* end switch */

	if( !retptr )
	{
		fprintf( stderr, "sissl/sinew: unable to allocate memory: %d\n", type );
		exit( 1 );
	}
	return retptr;           /* send back the new pointer */
}                            /* SInew */


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
*  Mnemonic: SIterm
*  Abstract: This routine will terminate a session based on the tp_blk
*            that is passed into the routine. The transport session block
*            is released and removed from the ginfo list. The session is
*            terminated by issuing a t_unbind call (if the unbind flag is
*            on in the tpptr block), and then issuing a t_close.
*  Parms:    gptr - Pointer to the global information block
*            tpptr - Pointer to tp block that defines the open fd.
*  Returns:  Nothing.
*  Date:     18 January 1995
*  Author:   E. Scott Daniels
*
**************************************************************************
*/
#include  "sisetup.h"     /* get the setup stuff */

extern void SIterm( struct tp_blk *tpptr )
{
 extern struct ginfo_blk *gptr;

 if( tpptr != NULL )
  {
	if( tpptr->fd >= 0 )
   		close( tpptr->fd );    

   if( tpptr->prev != NULL )            /* remove from the list */
    tpptr->prev->next = tpptr->next;    /* point previous at the next */
   else
    gptr->tplist = tpptr->next;        /* this was head, make next new head */
   if( tpptr->next != NULL )   
    tpptr->next->prev = tpptr->prev;  /* point next one back behind this one */

   free( tpptr->addr );             /* release the address bufers */
   free( tpptr->paddr );
   free( tpptr );                   /* and release the block */
  }
}                         /* SIterm */

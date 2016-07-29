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
*  Mnemonic: sitrash
*  Abstract: Delete all things referenced by a struct and then free the memory.
*    
*  Returns:  Nothing
*  Date:     08 March 2007
*  Author:   E. Scott Daniels                            
*
******************************************************************************   
*/

#include        "sisetup.h"
                 
extern void SItrash( int type, void *bp )
{
        struct tp_blk *tp = NULL;
        struct ioq_blk *iptr;
        struct ioq_blk *inext;

        switch( type )
        {
                case IOQ_BLK:
                        iptr = (struct ioq_blk *) bp;
                        free( iptr->data );
                        free( iptr->addr );
                        free( iptr );
                        break;
            
                case TP_BLK:                                            /* we assume its off the list */
                        tp = (struct tp_blk *) bp;
                        for( iptr = tp->squeue; iptr; iptr = inext )            /* trash any pending send buffers */
                        {
                                inext = iptr->next;
                                free( iptr->data );          /* we could recurse, but that seems silly */
                                free( iptr->addr );
                                free( iptr );
                        }
     
                        free( tp->addr );             /* release the address bufers */
                        free( tp->paddr );        
                        free( tp );                   /* and release the block */
                        break;
        }
}


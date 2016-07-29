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
*****************************************************************************
*
*  Mnemonic: SIcbstat
*  Abstract: This routine is responsible for the generic handling of
*            the return status from a call back routine.
*  Parms:    gptr - pointer to the ginfo block
*            status - The status that was returned by the call back
*            type   - Type of callback (incase unregister)
*  Returns:  Nothing.
*  Date:     23 January 1995
*  Author:   E. Scott Daniels
*
*****************************************************************************
*/
#include "sisetup.h"     /* get necessary defs etc */

extern void SIcbstat( int status, int type )
{
 extern struct ginfo_blk *gptr;

 switch( status )
  {
   case SI_RET_UNREG:                   /* unregister the callback */
    gptr->cbtab[type].cbrtn = NULL;     /* no pointer - cannot call */
    break;

   case SI_RET_QUIT:                 /* callback wants us to stop */
    gptr->flags |= GIF_SHUTDOWN;    /* so turn on shutdown flag */
    break;

   default:                 /* ignore the others */
    break;
  }   /* end switch */
}         /* SIcbstat */

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
*  Mnemonic: SIdokey
*  ABstract: This routine is called by siwait or sipoll to handle a key
*            that has been entered at the keyboard. We will do minor amounts
*            of "editing" control to be somewhat friendly. The key handeling
*            callback is driven when a complete message is received.
*            DOS Support only.
*  Parms:    gptr - Pointer to the master information block
*  Returns:  Nothing.
*  Date:     12 April 1995
*  Author:   E. Scott Daniels
*
******************************************************************************
*/
#include "sisetup.h"                /* various include files etc */

#define RETURN_KEY 0x0d             /* carriage return */
#define NEWLN_KEY  0x0a             /* line feed */
#define BACKSP_KEY 0x08             /* backspace key */

extern void SIdokey( )
{
 extern struct ginfo_blk *gptr;
 static char kbuf[256];                 /* keyboard type ahead buffer */
 static char *kidx = kbuf;              /* pointer at next spot for key */

 int ((*cbptr)());                      /* pointer to the callback function */
 int status;                            /* callback status processing */

 *kidx = getch( );                     /* get the key */

 switch( *kidx )                  /* see if we need to act on key value */
  {
   case RETURN_KEY:               /* buffer termination - make callback */
   case NEWLN_KEY:                /* who knows what the return generates */
     *kidx = EOS;                     /* terminate for callback */
     kidx = kbuf;                  /* reset for next pass */
     if( (cbptr = gptr->cbtab[SI_CB_KDATA].cbrtn) != NULL )
      {
       status = (*cbptr)( gptr->cbtab[SI_CB_KDATA].cbdata, kbuf );
       SIcbstat( gptr, status, SI_CB_KDATA );    /* handle status */
      }                                 /* end if call back was defined */
    break;

   case 0:             /* some alternate/function/arrow key pressed */
    getch( );          /* ignore it */
    break;

   case BACKSP_KEY:                 /* allow ctlh to edit buffer */
    putch( *kidx );                 /* echo the back sp */
    putch( ' ' );                   /* blank what was there */
    putch( *kidx );                 /* move back */
    kidx--;                         /* and back up the pointer */
    break;

   default:                        /* not specially processed */
    if( *kidx >= 0x20 )            /* if it is printable */
     {
      putch( *kidx );              /* echo the keystroke */
      kidx++;                      /* save it and move on to next */
     }
    break;
   }                                /* end switch */

}                                   /* SIdokey */

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
****************************************************************************
*
*  Mnemonic: SIcbreg
*  Abstract: This routine will register a callback in the table. Callbacks
*            are "unregistered" by passing a null function pointer.
*  Parms:    gptr - pointer to the general information block (SIHANDLE)
*            type - Type of callback to register (SI_CB_xxxxx)
*            fptr - Pointer to the function to register
*            dptr - Pointer that the user wants the callback function to get
*  Returns:  Nothing.
*  Date:     23 January 1995
*  Author:   E. Scott Daniels
*
****************************************************************************
*/
#include "sisetup.h"     /* get defs and stuff */

extern void SIcbreg( int type, int ((*fptr)()), void * dptr )
{
 extern struct ginfo_blk *gptr;

 if( gptr->magicnum == MAGICNUM )    /* valid block from user ? */
  {
   if( type >= 0 && type < MAX_CBS )   /* if cb type is in range */
    {
     gptr->cbtab[type].cbdata = dptr;   /* put in data */
     gptr->cbtab[type].cbrtn = fptr;    /* save function ptr */ 
    }
  }      /* endif valid block from user */
}        /* SIcbreg */

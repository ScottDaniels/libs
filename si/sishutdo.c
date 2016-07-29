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
******************************************************************************
*
*  Mnemonic: SIshutdown
*  Abstract: This routine will ensure that all tp blocks have been closed
*            with the transport provider and removed from the list. The
*            shutdown flag is set in addition.
*  Parms:    gptr - pointer to the ginfo structure (SIHANDLE)
*  Retrns:   Nothing.
*  Date:     23 March 1995
*  Author:   E. Scott Daniels
*
*****************************************************************************
*/
#include "sisetup.h"                   /* get includes and defines */

// it may seem silly tp pass the gpointer into this function since it is, after all, 
// a global, and even sillier since most of the other functions in the SI package
// assume it is a global. However, to support the sissl overlay libaray, this 
// function must accept the pointer because the sissl functions don't make the same
// global assumption and may be supporting multiple global information structures.

extern void SIshutdown( struct ginfo_blk *gptr )
{
	SIerrno = SI_ERR_HANDLE;
	if( gptr != NULL && gptr->magicnum == MAGICNUM )
	{
 		gptr->flags |=  GIF_SHUTDOWN;    /* signal shutdown */
		while( gptr->tplist != NULL )
		{
			gptr->tplist->flags |= TPF_UNBIND;    /* force unbind on session */
			SIterm(  gptr->tplist );         /* and drop the session */
		}                                      /* end while */
		SIerrno = 0;
	}
}            

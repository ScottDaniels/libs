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
*  Mnemonic: SIgetaddr
*  Abstract: This routine will get the address of the first listening
*            block on the tp list and return it in ASCII format to the
*            caller.
*  Parms:    gptr - Pointer to the global information block
*            buf  - Pointer to the buffer to hold the ascii string
*  Returns:  NI_OK if block found, NI_ERROR if no listen block exists
*  Date:     18 April 1995
*  Author:   E. Scott Daniels
*
******************************************************************************
*/
#include "sisetup.h"        /* get the standard include stuff */

extern int SIgetaddr( char *buf )
{
	extern struct ginfo_blk *gptr;
	struct tp_blk *tpptr;       /* Pointer into tp list */
	int status = SI_ERROR;       /* return status */
	char	*ibuf;		/* SIaddr now points us at a string, rather than filling ours */

 	for( tpptr = gptr->tplist; tpptr != NULL && !(tpptr->flags & TPF_LISTENFD);
      		tpptr = tpptr->next );

 	if( tpptr != NULL )
  	{
   		SIaddress( tpptr->addr, (void *) &ibuf, AC_TODOT );   /* convert to dot fmt */
		strcpy( buf, ibuf );				/* copy into caller's buffer */
		free( ibuf );
   		status = SI_OK;                               /* ok status for return */
  	}

 	return status;
}          

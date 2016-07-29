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
**************************************************************************
*  Mnemonic: SIreinit
*  Abstract: This routine is called to re-initialize the Network Interface
*            routines after the user process has execed another SI based
*            process over the parent that accepted the session. We will 
*            rebuild the global data structure, and tp block for the fd 
*            passed in. This routine will also reset the signals. 
*            It is assumed that the process is executing in the background.
*            If the caller needs to have the SI routines trap the ALARM
*            signal it will need to call the SIsetsig routine directly with
*            the SI_SIG_ALRM flag as the parameter as there is no way for
*            this routine to receive a parameter to indicate that it is to 
*            be set. 
*  Parms:    gptr - Pointer to general info block - NULL on first call. 
*            fd - The file descriptro that is mapped to a session.
*                    requested. 
*  Returns:  0 if something fails, else 1.
*  Date:     15 February 1995 
*  Author:   E. Scott Daniels
*
*  Modified: 21 Feb 1995 - To allow multiple calls if more than 1 fd open.
*             6 Mar 1995 - To pass setsig a signal mask parameter
*            17 Feb 2002 - To allow for global pointer, not passed on all calls 
**************************************************************************
*/
#include  "sisetup.h"     /* get the setup stuff */

extern int SIreinit( int fd )
{
 extern struct ginfo_blk *gptr;
 int status = OK;                /* status of internal processing */
 struct tp_blk *tpptr;           /* pointer at tp stuff */
 int i;                          /* loop index */

 SIerrno = SI_ERR_NOMEM;     /* setup incase alloc fails */

 if( gptr == NULL )          /* first call to init */
  {
   gptr = SInew( GI_BLK );                /* get a gi block */
   if( gptr != NULL )            /* was successful getting storage */
    {
     gptr->rbuf = (char *) malloc( MAX_RBUF );   /* get rcv buffer*/
     gptr->rbuflen = MAX_RBUF;

     SIsetsig( SI_DEF_SIGS );   /* set up default sigs that we want to catch */

     gptr->cbtab = (struct callback_blk *) malloc(
                           (sizeof( struct callback_blk ) * MAX_CBS ) );
     if( gptr->cbtab != NULL )
      {
       for( i = 0; i < MAX_CBS; i++ )     /* initialize call back table */
        {
         gptr->cbtab[i].cbdata = NULL;
         gptr->cbtab[i].cbrtn = NULL;
        }
      }          /* end if gptr->ctab ok */
     else        /* if call back table allocation failed - error off */
      {
       free( gptr );
       gptr = NULL;        /* dont allow them to continue */
      }
    }                 /* end if gen infor block allocated successfully */
  }                   /* end if gptr was null when called */

 if( gptr != NULL  &&  (tpptr = (struct tp_blk *) SInew( TP_BLK )) != NULL )
  {
   SIerrno = SI_ERR_TP;            /* if bad status then its a tp error */
   tpptr->fd = fd;                 /* assign the session id */
   tpptr->type = SOCK_DGRAM;       /* assume TCP session */
   tpptr->next = gptr->tplist;     /* add to head of the list */
   if( tpptr->next != NULL )
     tpptr->next->prev = tpptr;     /* back chain if not only one */
   gptr->tplist = tpptr; 
  }
 else 
  gptr = NULL;              /* allocation problem - ensure return is null */

 return gptr == NULL ? 0 : 1;        /* ok return if gptr was set */
}     /* SIreinit */

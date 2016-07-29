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
*  Mnemonic: SIsignal
*  Abstract: This routine is called to process signals. It will set a flag
*            in the global var sigflags indication action is necessary in
*            the application. In the case of a child's death signal, the
*            deaths variable is incremented so that the parent knows the
*            number of waits to issue to clear all zombies.
*  Parms:    sig - The signal we are being invoked to handle.
*  Returns:  Nothing.
*  Date:     19 January 1995
*  Author:   E. Scott Daniels
*
******************************************************************************
*/
#include  "sisetup.h"     /* get the setup stuff */

extern void SIsignal( int sig )
{
 extern int deaths;      /* number of deaths parent is not aware of */
 extern int sigflags;    /* flags that indicate to program what happened */

 switch( sig )
  {
   case SIGCHLD:      /* child has passed to the great beyond */
    deaths++;        /* let parent (us) know we must burry it (wait) */
    break;

   case SIGTERM:
   case SIGQUIT:                /* set the termination flag */
    sigflags |= SI_SF_QUIT;     /* set the quit flag */
    break;

   case SIGUSR1:              /* parent has sent us a request for status */
    sigflags |= SI_SF_USR1;    /* so let the main routine (wait) know */
    break;

   case SIGUSR2:
    sigflags |= SI_SF_USR2;
    break;

   case SIGALRM:
    sigflags |= SI_SF_ALRM;
   default:          /* no action if we dont recognize the signal */
    break;
  }                   /* end switch */
}                 /* SIsignal */

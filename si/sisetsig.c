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
*****************************************************************************
*
*  Mnemonic: SIsetsig
*  Abstract: This routine will inform the operating system of the singals
*            that we need to catch. This routine is broken out to provide
*            the user program the ability to reset the ni signal handler
*            should a DB (or other unfriendly rtl) stomp on the signal
*            trapping that we established during initialization.
*  Parms:    sigs - Bit mask indicating signals to catch.
*  Returns:  Nothing.
*  Date:     9 February 1995
*  Author:   E. Scott Daniels
*
*  Modified:  6 Mar 1995 - To accept parm indicating signal(s) to be trapped
*****************************************************************************
*/
#include "sisetup.h"         /* get the necessary start up stuff */

extern void SIsetsig( int sigs )
{
 struct sigaction sact;                /* signal block to pass to os */

 memset( &sact, 0, sizeof( sact ) );
 sact.sa_handler = &SIsignal;         /* setup our signal trap */

 if( sigs & SI_SIG_TERM )
  sigaction( SIGTERM, &sact, NULL );    /* catch term signals */

 if( sigs & SI_SIG_HUP )
  sigaction( SIGHUP, &sact, NULL );     /* catch hup signals */

 if( sigs & SI_SIG_QUIT )
  sigaction( SIGQUIT, &sact, NULL );    /* catch quit signals */

 if( sigs & SI_SIG_USR1 )
  sigaction( SIGUSR1, &sact, NULL );    /* catch user1 signals */

 if( sigs & SI_SIG_USR2 )
  sigaction( SIGUSR2, &sact, NULL );    /* catch user2 signals */

 if( sigs & SI_SIG_ALRM )
  sigaction( SIGALRM, &sact, NULL );   /* catch alarm signals */
}                  /* NIsetsig */

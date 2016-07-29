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
*  Mnemonic: SIexamp
*  Abstract: This is an example program using the SI run-time library to
*            communicate with another process. The foreign process' IP
*            address and port number is assumed to be passed in from the
*            command line. Once the connection is made the user is prompted
*            for input and the line is echoed to the foreign process.
*            This program is implemented using SIrcv and not the callback
*            driver functions of the SI run-time library.
*  Parms:    argc, argv
*              argv[1] = xx.xx.xx.xx.port  (IP address of foreign process)
*  Returns:  Nothing.
*  Date:     30 March 1995
*  Author:   E. Scott Daniels
*
*****************************************************************************
*/
#include <stdio.h>
#include <string.h>

#include "socket_if.h"                 /* get defines for SI calls */

#define TRUE 1
#define FALSE 0
#define EOS '\000'

int main( int argc, char **argv )
{
 int sid;            /* session id of foriegn process session */
 int done = FALSE;   /* flag to indicate we are done with the session */
 int kstat;          /* keyboard status */
 int rlen;           /* length of received string */
 char buf[256];      /* buffer to receive information in */
 char kbuf[256];     /* keyboard buffer */
 int s;
 extern int errno;

 if( argc < 2 )
  printf( "Missing arguments - enter IP.port address of foreign process\n" );
 else
  {
   if( (SIinit( SI_OPT_FG, -1, -1 )) != 0 )
    {
     if( (sid = SIconnect( argv[1] )) >= SI_OK )
      {
       printf( "Connection to %s was successful. \n", argv[1] );
       while( ! done )       /* loop until we are finished */
        {
         kstat = 0;
         rlen = 0;
         while( !(kstat = kbhit( )) &&               /* wait for key or msg */
                 (rlen = SIrcv(  sid, buf, 255, 100 )) == 0 );

         if( kstat )         /* keyboard data waiting */
          {
           gets( kbuf );                        /* read and send to partner */
           if( strcmp( "stop", kbuf ) == 0 )    /* user say were done? */
            {
             printf( "User requested termination of sample program.\n" );
             done = TRUE;
            }

           if( SIsendt(  sid, kbuf, strlen( kbuf )+1 ) == SI_ERROR )
            {
             printf( "Error sending. Session lost?\n" );
             done = TRUE;
            }
          }
         else                   /* not keyboard - check for received data */
          {
           if( rlen > 0 )
             printf( "From partner: %s\n", buf );  /* print buffer */
           else
            if( rlen < 0 )          /* assume error on the session */
             {
              done = TRUE;          /* force session to stop */
              printf( "Session error - Session lost?\n" );
             }
          }
        }            /* end while not done */

       SIshutdown(  );        /* were done - clean things up */
      }
     else
      printf( "Unable to connect to: %s\n", argv[1] );
    }
   else
    printf( "Unable to initialize the SI package. SIerrno = %d\n", SIerrno );
  }

}         /* end main */

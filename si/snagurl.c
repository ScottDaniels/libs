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
#include "siconst.h"
#include "socket_if.h"

#define NULL (void *) 0 
#define EOS '\000'

int cbconn( );   /* simple predeclaration of callback routines */
int cbkey( );
int cbcook( );
int cbdisc( );

int fd = -1;
int echo = 0;

void helpem( )
{
 printf( "Test: a simple little example to demonstrate the si library\n" );
 printf( "Anything not recognised as a command will be sent if there\n" );
 printf( "is a session established.\n" );
 printf( "Commands:\n" );
 printf( "  conn establish connection to: {ip-address:port | hostname[.domainname]:port}\n" );
 printf( "  disc disconnect the session)\n" );
 printf( "  echo echo data received back on session)\n" );
 printf( "  stop exit the utility)\n" );
}

int main( int argc, char **argv )
{
 extern int SIerrno;
 int i;
 int port = 4360;
 int status = 0;

 if( argc > 1 )
  port = atoi( argv[1] );     /* convert the passed in port number */

 printf( "Calling init port = %d\n", port );
 status  = SIinit( SI_OPT_FG | SI_OPT_TTY, port, -1 );
 printf( "test: After initialization status=%p\n", status );
 if( ! status )
  {
   printf( "Could not initialize SIerr=%d err=%d\n", SIerrno, errno );
   return( 1 );
  }

 printf( "registering callbacks\n" );
 SIcbreg( SI_CB_CONN, &cbconn, NULL );    /* register connect cb */
 SIcbreg( SI_CB_DISC, &cbdisc, NULL );    /* register connect cb */
 SIcbreg( SI_CB_KDATA, &cbkey, NULL );
 SIcbreg( SI_CB_CDATA, &cbcook, NULL );

 printf( "test: Waiting....\n" );
 status = SIwait(  );

 SIshutdown(  gptr );          /* for right now just shutdown */
 printf( "returned from siwait status = %d sierr =%d errno=%d(%s)\n", status, SIerrno, errno, strerror( errno ) );
}              /* main */

cbconn( char *data, int f, char *buf )
{
 fd = f;
 printf( "Connection received fd=%d\n", f );
 return( SI_RET_OK );
}

cbdisc( char *data, int f )
{
 fd = -1;
 printf( "test: session disconnected: %d\n", f );
}

cbkey( void *data, char *buf )
{
 int status = SI_RET_OK;     /* address buffer */
 char abuf[100];
 extern int SIerrno;
 char *p;

 for( p = buf; *p && *p != '\n'; p++ );    /* stop at first new line */
 *p = 0;

 if(  strncmp( buf, "stop", 4 ) == 0 )
  {
   printf( "Stopping\n" );
   status = SI_RET_QUIT;
  }
 else
  if( strncmp( buf, "conn ", 5 ) == 0 )
   {
    if( fd < 0 )
     {
      SIerrno = 0;
      sprintf( abuf, "%s", &buf[5] );
      printf( "Requesting connection with: %s\n", abuf );
      fd = SIconnect( abuf );          /* attempt connection */
      printf( "CBKEY: connect status: fd=%d SIerr=%d err=%d\n", 
               fd, SIerrno, errno );
     }
   }
  else
   if( strncmp( buf, "disc", 4 ) == 0 )
    {
     if( fd > 0 )
      {
       SIclose( fd );
       printf( "session %d disconnected\n", fd );
      }
     fd = -1;
    }
   else
   if( strncmp( buf, "echo", 4 ) == 0 )
    echo = (echo +1) %2;
   else
   if( strncmp( buf, "help", 5 ) == 0 )
    helpem( );
   else
   if( fd >= 0 ) 
    SIsendt( fd, buf, strlen( buf )+1 );
   else  
    printf( "Command (%s) not recognised\n", buf );

 return( status );
}    /* cbkey */

cbcook( void *data, int fd, char *buf, int len )
{
 if( len > 0 )
  {
   printf( "CBCOOK: received %d bytes on session %d.\n", len, fd );
   buf[len] = EOS;
   printf( "CBCOOK: (%s)\n", buf );
   if( echo )
    SIsendt( fd, buf, strlen( buf ) );
  }
 return 0;
}

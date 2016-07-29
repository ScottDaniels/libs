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
//#include "siconst.h"
#include "socket_if.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define EOS '\000'

int cbconn( );   /* simple predeclaration of callback routines */
int cbkey( );
int cbcook( );
int cbraw( );
int cbdisc( );

int	fd = -1;		/* connected fd */
int	lfd_tcp4 = -1;		/* listen fds */
int	lfd_udp4 = -1;
int	lfd_tcp6 = -1;		/* ipv6 listen fds */
int	lfd_udp6 = -1;
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
 	extern int errno;
 	int i;
 	int port = 4360;
	int uport = -1;
 	int status = 0;
	char abuf[128];

 	if( argc > 1 )
  		port = atoi( argv[1] );     /* convert the passed in port number */
	if( argc > 2 )
		uport = atoi( argv[2] );

 	errno = 0;
 	/*status  = SIinit( SI_OPT_FG | SI_OPT_TTY, port, port ); */
 	status = SIinitialise( SI_OPT_FG | SI_OPT_TTY );		/* new method - 3/13/07 */
	printf( "initialised, status = %d\n", status );

 	if( ! status )
  	{
   		printf( "could not initialize SIerr=%d err=%d %s\n", SIerrno, errno, strerror( errno ) );
   		return( 1 );
  	}

 	printf( "setting TCP listeners (v4 and v6) on port = %d\n", port );
	sprintf( abuf, "0.0.0.0;%d", port );
 	lfd_tcp4 = SIlistener( TCP_DEVICE, abuf );	/* get the v4 listener */
	sprintf( abuf, "::1;%d", port );
 	lfd_tcp6 = SIlistener( TCP_DEVICE, abuf );	/* get the v4 listener */

	if( uport > 0 )
	{
		printf( "opening udp listeners for port %d\n", uport );
		sprintf( abuf, "0.0.0.0;%d", port );
		lfd_udp4 = SIlistener( UDP_DEVICE, abuf );
		sprintf( abuf, "::1;%d", port );
		lfd_udp6 = SIlistener( UDP_DEVICE, abuf );
	}
	else
		printf( "udp listeners not started; no port supplied on command line\n" );

	printf( "listen fds: %d(t4) %d(t6) %d(u4) %d(u6)\n", lfd_tcp4, lfd_tcp6, lfd_udp4, lfd_udp6 );

	if( lfd_tcp4 < 0 || lfd_tcp6 < 0  )
	{
		printf( "no open listen ports; aborting\n" );
		exit( 1 );
	}


 printf( "test: registering callbacks\n" );
 SIcbreg( SI_CB_CONN, &cbconn, NULL );    /* register callbacks */
 SIcbreg( SI_CB_DISC, &cbdisc, NULL );    
 SIcbreg( SI_CB_KDATA, &cbkey, NULL );
 SIcbreg( SI_CB_CDATA, &cbcook, NULL );
 SIcbreg( SI_CB_RDATA, &cbraw, NULL );

 printf( "test: Waiting....\n" );
 while( (status = SIpoll( 500 )) >= 0 );
 /*status = SIwait(  );*/

 SIshutdown(  );          /* for right now just shutdown */
 printf( "returned from siwait status = %d sierr =%d errno=%d(%s)\n", status, SIerrno, errno, errno > 0 ? strerror( errno ) : "[OK]" );
}              /* main */

int cbconn( char *data, int f, char *buf )
{
	fd = f;
	printf( "Connection received fd=%d from %s\n", f, buf );
	return( SI_RET_OK );
}

int cbdisc( char *data, int f )
{
	fd = -1;
	printf( "test: session disconnected: %d\n", f );
}

int cbkey( void *data, char *buf )
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
 if(  strncmp( buf, "quit", 4 ) == 0 )
  {
   printf( "Stopping\n" );
   status = SI_RET_QUIT;
  }
 else
 if(  strncmp( buf, "exit", 4 ) == 0 )
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

      printf( "CBKEY: connect status: fd=%d SIerr=%d err=%d %s\n", fd, SIerrno, errno, errno > 0 ? strerror( errno ) : "[OK]" );
     }
	else
		fprintf( stderr, "cannot connect already connected fd=%d\n", fd );
   }
  else
   if( strncmp( buf, "close", 5 ) == 0 )
    {
     if( fd > 0 )
      {
       SIclose( fd );
       printf( "session %d disconnected\n", fd );
      }
     fd = -1;
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
	if( strncmp( buf, "udp ", 4 ) == 0 ) 
	{
		char *tok;
		char *msg; 

		tok = strchr( buf, ' ' );
		tok++;
		msg =  strchr( tok, ' ' );
		if( msg )
		{
			*msg = 0;
			msg++;

			printf( "sending: (%s) %s\n", tok, msg );
			if( SIsendu( tok, msg, strlen(msg ) ) < 0 )
				printf( "udp send failed\n" );
		}
	}
	else
	if( fd >= 0 ) 
	{
		SIsendt( fd, buf, strlen( buf ) );
		SIsendt( fd, "\n", 1 );
	}
	else  
		printf( "Command (%s) not recognised\n", buf );

 return( status );
}    /* cbkey */

int cbraw( void *data, char *buf, int len, char *from )
{
	if( len > 0 )
	{
		printf( "CBRAW: received %d bytes on session %s.\n", len, from );
		buf[len] = EOS;
		printf( "CBRAW: (%s)\n", buf );
	}
	else
		printf( "CBRAW: received data len= 0\n" );

	return 0;
}

int cbcook( void *data, int fd, char *buf, int len )
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


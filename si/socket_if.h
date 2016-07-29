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
*  Mnemonic: sidefs.h
*  Abstract: This file contains the definitions that a "user" application
*            will include. It is also included by the siconst.h file. We 
#		install this into ../include/socket_if.h
*  Date:     26 March 1995
*  Author:   E. Scott Daniels
*
*****************************************************************************
*/

#ifndef _SOCKET_IF_H
#define _SOCKET_IF_H

#define TCP_DEVICE	0     	/* device type of socket */
#define UDP_DEVICE	1	

/* these are for SIclose, must be negative so as to be distinguished from real fd values */
#define TCP_LISTEN_PORT	(-1)	/* close first listen port found */
#define UDP_PORT	(-2)	/* close first udp port found */

#define SI_BAD_HANDLE  ((void *) 0)

#define SI_OPT_FORK    0x01      /* initialization options - fork sessions */
#define SI_OPT_FG      0x02      /* keep process in the "foreground" */
#define SI_OPT_TTY     0x04      /* processes keyboard interrupts if fg */
#define SI_OPT_ALRM    0x08      /* cause setsig to be called with alarm flg */

                                 /* offsets of callbacks in table */
                                 /* used to indentify cb in SIcbreg */
#define SI_CB_SIGNAL   0         /* usr signal/alarm received */
#define SI_CB_RDATA    1         /* handles data arrival on raw interface */
#define SI_CB_CDATA    2         /* handles data arrival on cooked interface */
#define SI_CB_KDATA    3         /* handles data arrival from keyboard */
#define SI_CB_SECURITY 4         /* authorizes acceptance of a conect req */
#define SI_CB_CONN     5         /* called when a session is accepted */
#define SI_CB_DISC     6         /* called when a session is lost */
#define SI_CB_POLL     7

                                 /* return values from callback */
#define SI_RET_OK      0         /* processing ok */
#define SI_RET_ERROR   (-1)      /* processing not ok */
#define SI_RET_UNREG   1         /* unregester (never call again) the cb */
#define SI_RET_QUIT    2         /* set the shutdown flag */

                                 /* values returned to user by SI routines */
#define SI_ERROR       (-1)      /* unable to process */
#define SI_OK          0         /* processing completed successfully */
#define SI_QUEUED      1         /* send messages was queued */

                                  /* flags passed to signal callback */
#define SI_SF_QUIT     0x01      /* program should terminate */
#define SI_SF_USR1     0x02      /* user 1 signal received */
#define SI_SF_USR2     0x04      /* user 2 signal received */
#define SI_SF_ALRM     0x08      /* alarm clock signal received */

                                 /* signal bitmasks for the setsig routine */
#define SI_SIG_QUIT    0x01      /* please catch quit signal */
#define SI_SIG_HUP     0x02      /* catch hangup signal */
#define SI_SIG_TERM    0x04      /* catch the term signal */
#define SI_SIG_USR1    0x08      /* catch user signals */
#define SI_SIG_USR2    0x10
#define SI_SIG_ALRM    0x20      /* catch alarm signals */
#define SI_DEF_SIGS    0x1F      /* default signals to catch */

                                 /* SIerrno values set in public rtns */
#define SI_ERR_NONE     0        /* no error as far as we can tell */
#define SI_ERR_QUEUED   1	/* must be same as queued */
#define SI_ERR_TPORT    2        /* could not bind to requested tcp port */
#define SI_ERR_UPORT    3        /* could not bind to requested udp port */
#define SI_ERR_FORK     4        /* could not fork to become daemon */
#define SI_ERR_HANDLE   5        /* global information pointer went off */
#define SI_ERR_SESSID   6        /* invalid session id */
#define SI_ERR_TP       7        /* error occured in transport provider */
#define SI_ERR_SHUTD    8        /* cannot process because in shutdown mode */
#define SI_ERR_NOFDS    9        /* no file descriptors are open */
#define SI_ERR_SIGUSR1  10       /* signal received data not read */
#define SI_ERR_SIGUSR2  11       /* signal received data not read */
#define SI_ERR_DISC     12       /* session disconnected */
#define SI_ERR_TIMEOUT  13       /* poll attempt timed out - no data */
#define SI_ERR_ORDREL   14       /* orderly release received */
#define SI_ERR_SIGALRM  15       /* alarm signal received */
#define SI_ERR_NOMEM    16       /* could not allocate needed memory */
#define SI_ERR_ADDR    	17       /* address conversion failed */


#ifndef _SI_ERRNO
extern int SIerrno;               /* error number set by public routines */
#endif


/* -------------------------- prototypes ---------------------------- */

extern void SIcbreg( int type, int ((*fptr)()), void * dptr );
extern int SIclose( int fd );
extern int SIconnect( char *abuf );
extern void SIdokey(  void );
extern int SIgetaddr( char* buf );
extern int SIgetfd( int sid );
extern char *sigetname( int sid );
extern int SIinitialise( int opts );
extern int SIinit( int opts, int tport, int uport );
extern int SIlistener( int type, char *abuf );
extern int SIlisten( int port );
extern int SImcast_join(  char *group, int iface, int ttl );
extern int SImcast_leave( char *group, int iface );
extern int SIpoll( int msdelay );
extern int SIrcv( int sid, char *buf, int buflen, char *abuf, int delay );
extern int SIreinit( int fd );
extern int SIsendt( int fd, char *ubuf, int ulen );
extern int SIsendu( char *abuf, char *ubuf, int ulen );
extern void SIsetsig( int sigs );
extern void SItrash( int type, void *bp );
extern int SIwait( void );

#endif

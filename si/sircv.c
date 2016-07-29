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
*****************************************************************************
*
*  Mnemonic: SIrcv
*  Abstract: This routine allows the user program to receive data on a
*            session without using the callback structure of the library.
*            It is the caller's responsibility to provide a buffer large
*            enough to handle the received data.
*  Parms:    gptr - The SIHANDLE that the user received on init call
*            sid  - The session id that the user wants to check
*            buf  - Pointer to buffer to receive data in
*            abuf - Pointer to buffer to return address of UDP sender in (!null)
*            buflen-Length of the receive buffer
*            delay- Value to pass to poll (time out) -1 == block until data
*  Returns:  SI_ERROR - (SIerrno will contain reason) if failure, else the
*            number of bytes read. If the number read is 0 SIerrno will indicate
*            why: time out exceeded, signal received.
*  Date:     26 March 1995
*  Author:   E. Scott Daniels
*  Mods:     26 Mar 20001 - Changed to support UDP reads
*
******************************************************************************
*/
#include "sisetup.h"    /* get start up stuff */

extern int SIrcv( int sid, char *buf, int buflen, char *abuf, int delay )
{
 extern struct ginfo_blk *gptr;
 extern int sigflags;           /* signal flags */
 int status = SI_ERROR;         /* assume the worst to return to caller */
 struct tp_blk *tpptr;          /* pointer to transport provider info */
 int flags = 0;                 /* receive flags */
 int remainder;                 /* # of bytes remaining after rcv if more */
 fd_set readfds;                /* special set of read fds for this call */
 fd_set execpfds;               /* special set of read fds for this call */
 struct timeval *tptr = NULL;   /* time info for select call */
 struct timeval time;
 struct sockaddr *uaddr;       /* pointer to udp address */
	char 	*acbuf;		/* pointer to converted address */
 int addrlen;

 SIerrno = SI_ERR_HANDLE;              /* set errno before we fail */
 if( gptr->magicnum != MAGICNUM )     /* if not a valid ginfo block */
  return( ERROR );

 SIerrno = SI_ERR_SESSID;             /* set errno before we fail */
 for( tpptr = gptr->tplist; tpptr != NULL && tpptr->fd != sid;
      tpptr = tpptr->next );      /* find transport block */
 if( tpptr == NULL )
  return( ERROR );                      /* signal bad block */

 uaddr = (struct sockaddr *) malloc( sizeof( struct sockaddr ) );
 addrlen = sizeof( *uaddr );

 SIerrno = SI_ERR_SHUTD;               /* set errno before we fail */
 if( ! (gptr->flags & GIF_SHUTDOWN)  &&  ! (sigflags) )
  {                        /* if not in shutdown and no signal flags  */
   FD_ZERO( &readfds );               /* clear select info */
   FD_SET( tpptr->fd, &readfds );     /* set to check read status */

   FD_ZERO( &execpfds );               /* clear select info */
   FD_SET( tpptr->fd, &execpfds );     /* set to check read status */

   if( delay >= 0 )                /* user asked for a fininte time limit */
    {
     tptr = &time;                 /* point at the local struct */
     tptr->tv_sec = 0;             /* setup time for select call */
     tptr->tv_usec = delay;
    }

   SIerrno = SI_ERR_TP;
   if( (select( tpptr->fd + 1, &readfds, NULL, &execpfds, tptr ) < 0 ) ||
       (sigflags & SI_SF_QUIT) )
    gptr->flags |= GIF_SHUTDOWN;     /* we must shut on error or signal */
   else
    {                                /* poll was successful - see if data ? */
     SIerrno = SI_ERR_TIMEOUT;
     if( FD_ISSET( tpptr->fd, &execpfds ) )   /* session error? */
      {
       SIterm( tpptr );                 /* clean up our end of things */
       SIerrno = SI_ERR_SESSID;               /* set errno before we fail */
      }
     else
      {
       if( (FD_ISSET( tpptr->fd, &readfds )) &&  ! (sigflags ) )
        {                                       /* process data if no signal */
		SIerrno = SI_ERR_TP;
		if( tpptr->type == SOCK_DGRAM )        /* raw data received */
		{
			status = recvfrom( sid, buf, buflen, 0, uaddr, &addrlen );
			if( abuf )
			{
				SIaddress( uaddr, (void **) &acbuf, AC_TODOT );	/* address returns pointer to buf now rather than filling */
				strcpy( abuf, acbuf );			/* must be back compat with old versions */
				free( acbuf );
			}
			if( status < 0 )                        /* session terminated? */
				SIterm( tpptr );                 /* so close our end */
          	}
         	else                                      /* cooked data received */
          	{
           		status = recv( sid, buf, buflen, 0 );   /* read data into user buf */
           		if( status < 0 )                        /* session terminated? */
            			SIterm( tpptr );                 /* so close our end */
          	}
        }                                         /* end event was received */
       else                                       /* no event was received  */
        status = 0;                               /* status is just ok */
      }                       /* end else - not in shutdown mode after poll */
    }                     /* end else pole was successful */
  }                                 /* end if not already signal shutdown */

 if( gptr->flags & GIF_SHUTDOWN  &&  gptr->tplist != NULL )
  {             /* shutdown received but sessions not cleaned up */
   SIshutdown( );
   status = SI_ERROR;                /* indicate failure on return */
  }                                  /* end if shut but not clean */
 else                                /* if not in shutdown see if */
  {
   if( sigflags )                   /* a signal was received? */
    {                              /* return = 0, SIerrno indicates why */
     status = 0;                   /* no data received */
     if( sigflags & SI_SF_USR1 )   /* set return code based on the flag */
      {
       SIerrno = SI_ERR_SIGUSR1;
       sigflags &= ~SI_SF_USR1;    /* turn off flag */
      }
     else
      if( sigflags & SI_SF_USR2 )   /* siguser 2 received */
       {
        SIerrno = SI_ERR_SIGUSR2;
        sigflags &= ~SI_SF_USR2;
       }
      else
       if( sigflags & SI_SF_ALRM )    /* alarm signal received */
        {
         SIerrno = SI_ERR_SIGALRM;
         sigflags &= ~SI_SF_ALRM;     /* put down the flag */
        }
    }                        /* end if sigflag was on */
  }                         /* end else not in shutdown mode */

 free( uaddr );
 return( status );          /* send back the status */
}                           /* SIrcv */

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
*  Mnemonic: SIpoll
*  Abstract: This routine will poll the sockets that are open for
*            an event and return after the delay period has expired, or
*            an event has been processed.
*  Parms:    gptr   - Pointer to the global information block
*            msdelay- 100ths of seconds to delay
*  Returns:  OK if the caller can continue, ERROR if all sessions have been
*            stopped, or the interface cannot proceed. When ERROR is
*            returned the caller should cleanup and exit immediatly (we
*            have probably received a sigter or sigquit.
*  Date:     10 April 1995
*  Author:   E. Scott Daniels
*
**************************************************************************
*/
#include  "sisetup.h"     /* get the setup stuff */
#include <wait.h>


extern int SIpoll( int msdelay )
{
 extern struct ginfo_blk *gptr;
 extern int deaths;       /* number of children that died and are zombies */
 extern int sigflags;     /* flags set by the signal handler routine */

 int fd;                       /* file descriptor for use in this routine */
 int ((*cbptr)());             /* pointer to callback routine to call */
 int status = OK;              /* return status */
 int addrlen;                  /* length of address from recvfrom call */
 char *buf; 	               /* work buffer pointer */
 char ibuf[1025];
 int i;                        /* loop index */
 struct tp_blk *tpptr;         /* pointer at tp stuff */
 struct tp_blk *nextone;        /* pointer at next block to process */
 int pstat;                    /* poll status */
 int kstat;                    /* keyboard status */
 struct timeval  delay;        /* delay to use on select call */
 struct sockaddr *uaddr;       /* pointer to udp address */

 SIerrno = SI_ERR_SHUTD;

 if( gptr->flags & GIF_SHUTDOWN )     /* cannot do if we should shutdown */
  return( ERROR );                    /* so just get out */


 SIerrno = SI_ERR_HANDLE;

 if( gptr->magicnum != MAGICNUM )     /* if not a valid ginfo block */
  return( ERROR );

   delay.tv_sec = msdelay/100;                /* user submits 100ths, cvt to seconds and milliseconds */
   delay.tv_usec = (msdelay%100) * 10;       


   SIbldpoll( gptr );                 /* build the fdlist for poll */
   pstat = 0;                         /* ensure good code */

   if( gptr->fdcount > 0 )
    pstat = select( gptr->fdcount, &gptr->readfds, &gptr->writefds,
                               &gptr->execpfds, &delay );

   if( (pstat < 0 && errno != EINTR) || (sigflags & SI_SF_QUIT) )
    {                             /* poll fail or termination signal rcvd */
     gptr->fdcount = 0;           /* prevent trying to look at a session */
     gptr->flags |= GIF_SHUTDOWN; /* cause cleanup and exit at end */
     deaths = 0;                  /* dont need to issue waits on dead child */
     sigflags = 0;                /* who cares about signals now too */
    }

   while( deaths > 0 )  /* there have been death(s) - keep the dead */
    {                   /* from being zombies - send them to heaven */
     wait( NULL );                       /* issue wait on child */
     deaths--;
    }                   /* end while dead children to send to heaven */

  if( sigflags &&        /* if signal received and processing them */
     (cbptr = gptr->cbtab[SI_CB_SIGNAL].cbrtn) != NULL )
   {
    while( sigflags != 0 )
     {
      i = sigflags;                  /* hold for call */
      sigflags = 0;                  /* incase we are interrupted while away */
      status = (*cbptr)( gptr->cbtab[SI_CB_SIGNAL].cbdata, i );
      SIcbstat( status, SI_CB_SIGNAL );    /* handle status */
     }                                           /* end while */
   }

   if( pstat > 0  &&  (! (gptr->flags & GIF_SHUTDOWN)) )
    {
     if( FD_ISSET( 0, &gptr->readfds ) )       /* check for keybd input */
      {
       fgets( ibuf, 1024, stdin );   /* get the stuff from keyboard */
       if( (cbptr = gptr->cbtab[SI_CB_KDATA].cbrtn) != NULL )
        {
         status = (*cbptr)( gptr->cbtab[SI_CB_KDATA].cbdata, ibuf );
         SIcbstat(  status, SI_CB_KDATA );    /* handle status */
        }                                 /* end if call back was defined */
      }

     /*for( tpptr = gptr->tplist; tpptr != NULL; tpptr = tpptr->next )*/
     for( tpptr = gptr->tplist; tpptr != NULL; tpptr = nextone )
      {
	nextone = tpptr->next;					/* prevent coredump if we delete the session */

       if( tpptr->squeue != NULL && (FD_ISSET( tpptr->fd, &gptr->writefds )) )
        SIsend( tpptr );              /* send if clear to send */

       if( FD_ISSET( tpptr->fd, &gptr->execpfds ) )
        {
         ; /* sunos seems to set except for unknown reasons; ignore */
        }
       else
       if( FD_ISSET( tpptr->fd, &gptr->readfds ) )  /* read event pending? */
        {
         fd = tpptr->fd;                     /* quick ref to the fd */

         if( tpptr->flags & TPF_LISTENFD )     /* listen port setup by init? */
          {                                    /* yes-assume new session req */
           status = SInewsession( tpptr );    /* make new session */
          }
         else                              /* data received on a regular port */
          if( tpptr->type == SOCK_DGRAM )          /* udp socket? */
           {
            uaddr = (struct sockaddr *) malloc( sizeof( struct sockaddr ) );
            status = recvfrom( fd, gptr->rbuf, MAX_RBUF, 0, uaddr, &addrlen );
            if( status >= 0 && ! (tpptr->flags & TPF_DRAIN) )
             {					                         /* if good status call cb routine */
              if( (cbptr = gptr->cbtab[SI_CB_RDATA].cbrtn) != NULL )
               {
                SIaddress( uaddr, (void **) &buf, AC_TODOT );
                status = (*cbptr)( gptr->cbtab[SI_CB_RDATA].cbdata, gptr->rbuf, status, buf );
                SIcbstat( status, SI_CB_RDATA );    /* handle status */
		free( buf );
               }                              /* end if call back was defined */
             }                                /* end if status was ok */
            free( uaddr );
           }                                  /* end if udp */
          else
           {                                /* else receive on tcp session */
            status = recv( fd, gptr->rbuf, MAX_RBUF, 0 );    /* read data */

            if( status > OK  &&  ! (tpptr->flags & TPF_DRAIN) )
             {
              if( (cbptr = gptr->cbtab[SI_CB_CDATA].cbrtn) != NULL )
               {
                status = (*cbptr)( gptr->cbtab[SI_CB_CDATA].cbdata, fd, gptr->rbuf, status );
                SIcbstat( status, SI_CB_CDATA );   /* handle cb status */
               }                            /* end if call back was defined */
             }                                     /* end if status was ok */
            else   /* sunos seems to send 0 bytes as indication of disc */
             {
              if( (cbptr = gptr->cbtab[SI_CB_DISC].cbrtn) != NULL )
               {
                status = (*cbptr)( gptr->cbtab[SI_CB_DISC].cbdata, tpptr->fd );
                SIcbstat( status, SI_CB_DISC );    /* handle status */
               }
              SIterm( tpptr );
            }
           }                                                /* end tcp read */
        }                    /* end if event on this fd */
      }                      /* end for each fd in the list */
    }                        /* end if not in shutdown */


 if( gptr->flags & GIF_SHUTDOWN )      /* we need to stop for some reason */
  {
   SIerrno = SI_ERR_SHUTD;        /* indicate error exit status */
   status = ERROR;                /* status should indicate to user to die */
   SIshutdown( );            /* clean things up */
  }
 else
  status = OK;                    /* user can continue to process */

 return( status );                /* send status back to caller */
}                                 /* SIwait */

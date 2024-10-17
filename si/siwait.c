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
*  Mnemonic: SIwait
*  Abstract: This  routine will wait for an event to occur on the
*            connections in tplist. When an event is received on a fd
*            the status of the fd is checked and the event handled, driving
*            a callback routine if necessary. The system call poll is usd
*            to wait, and will be interrupted if a signal is caught,
*            therefore the routine will handle any work that is required
*            when a signal is received. The routine continues to loop
*            until the shutdown flag is set, or until there are no open
*            file descriptors on which to wait.
*  Parms:    gptr - Pointer to the global information block
*  Returns:  OK if the caller can continue, ERROR if all sessions have been
*            stopped, or the interface cannot proceed. When ERROR is
*            returned the caller should cleanup and exit immediatly (we
*            have probably received a sigter or sigquit.
*  Date:     28 March 1995
*  Author:   E. Scott Daniels
*
*  Modified: 11 Apr 1995 - To pass execption to select when no keyboard
*            19 Apr 1995 - To call do key to do better keyboard editing
*            18 Aug 1995 - To init kstat to 0 to prevent key hold if
*                          network data pending prior to entry.
*			31 Jul 2016 - Major formatting clean up in the main while loop.
**************************************************************************
*/

#include  "sisetup.h"     // get the setup stuff 
#include	"sys/wait.h"


extern int SIwait( ) {
	extern struct ginfo_blk *gptr;
	extern int deaths;            // number of children that died and are zombies 
	extern int sigflags;          // flags set by the signal handler routine 
	int fd;                       // file descriptor for use in this routine 
	int (*cbptr)();             // pointer to callback routine to call 
	int status = OK;              // return status 
	socklen_t addrlen;                  // length of address from recvfrom call 
	struct tp_blk *tpptr;         // pointer at tp stuff 
	struct tp_blk *nextone;	// point at next block to process in loop 
	int pstat;                    // poll status 
	struct sockaddr *uaddr;       // pointer to udp address 
	char *buf;
	char *ibuf;

	ibuf = (char *) malloc( 2048 );

	SIerrno = SI_ERR_SHUTD;

	if( gptr->flags & GIF_SHUTDOWN ) {     // cannot do if we should shutdown 
		return( ERROR );                    // so just get out 
	}

	SIerrno = SI_ERR_HANDLE;

	if( gptr->magicnum != MAGICNUM ) {     // if not a valid ginfo block 
		return( ERROR );
	}

	uaddr = (struct sockaddr *) malloc( sizeof( struct sockaddr ) );

	do {                                      //  main wait/process loop 
		SIbldpoll( gptr );                 // build the fdlist for poll 
		pstat = select( gptr->fdcount, &gptr->readfds, &gptr->writefds, &gptr->execpfds, NULL );

		if( (pstat < 0 && errno != EINTR) || (sigflags & SI_SF_QUIT) ) { // poll fail or termination signal rcvd 
			gptr->fdcount = 0;           // prevent trying to look at a session 
			gptr->flags |= GIF_SHUTDOWN; // cause cleanup and exit at end 
			deaths = 0;                  // dont need to issue waits on dead child 
			sigflags = 0;                // who cares about signals now too 
		}

		while( deaths > 0 ) {	// there have been death(s) - keep the dead from being zombies - send them to heaven 
			while( waitpid( 0, NULL, WNOHANG ) > 0 ) {                // wait for all finished processes without blocking 
				deaths--;
			}                   // end while dead children to send to heaven 

			if( sigflags && (cbptr = gptr->cbtab[SI_CB_SIGNAL].cbrtn) != NULL ) { // if signal received and processing them 
				while( sigflags != 0 ) {
					sigflags = 0;                  // incase we are interrupted while away 
					//status = (*cbptr)( gptr->cbtab[SI_CB_SIGNAL].cbdata, i );
					status = (*cbptr)();
					SIcbstat( status, SI_CB_SIGNAL );    // handle status 
				}                                           // end while 
			}

			if( pstat > 0  &&  (! (gptr->flags & GIF_SHUTDOWN)) ) {
				if( FD_ISSET( 0, &gptr->readfds ) ) {       // check for keybd input 
					fgets( ibuf, 2000, stdin );   // get the stuff from keyboard 
					if( (cbptr = gptr->cbtab[SI_CB_KDATA].cbrtn) != NULL ) {
							//status = (*cbptr)( gptr->cbtab[SI_CB_KDATA].cbdata, ibuf );
							status = (*cbptr)( );
							SIcbstat( status, SI_CB_KDATA );    // handle status 
					}                                 // end if call back was defined 
				}

				for( tpptr = gptr->tplist; tpptr != NULL; tpptr = nextone ) {
					nextone = tpptr->next;				// prevent issues if we delete the block during loop 
					if( tpptr->fd >= 0 ) {
							if( tpptr->squeue != NULL && (FD_ISSET( tpptr->fd, &gptr->writefds )) ) {
								SIsend( tpptr );              // send if clear to send 
							}
				
						if( FD_ISSET( tpptr->fd, &gptr->execpfds ) ) {
							;  				// sunos seems to set the except flag for unknown reasons; ignore it
						} else {
							if( FD_ISSET( tpptr->fd, &gptr->readfds ) ) {  // read event pending? 
								fd = tpptr->fd;                     // quick ref to the fd 
				
									if( tpptr->flags & TPF_LISTENFD ) {    // new session request
										errno=0;
										status = SInewsession( tpptr );    // make new session 
									} else  {									// data received on a regular port 
										if( tpptr->type == SOCK_DGRAM ) {  		// udp socket? 
										addrlen = sizeof( *uaddr );
										status = recvfrom( fd, gptr->rbuf, MAX_RBUF, 0, uaddr, &addrlen );
										if( status > 0 && ! (tpptr->flags & TPF_DRAIN) ) {                         // if good status call cb routine 
											if( (cbptr = gptr->cbtab[SI_CB_RDATA].cbrtn) != NULL ) {
												SIaddress( uaddr, (void **) &buf, AC_TODOT );
												//status = (*cbptr)( gptr->cbtab[SI_CB_RDATA].cbdata, gptr->rbuf, status, buf );
												status = (*cbptr)( );
												SIcbstat( status, SI_CB_RDATA );    // handle status 
												free( buf );
											}
										}
									} else {										// tcp session
										status = recv( fd, gptr->rbuf, MAX_RBUF, 0 );    // read data 
										if( status > OK  &&  ! (tpptr->flags & TPF_DRAIN) ) {
											if( (cbptr = gptr->cbtab[SI_CB_CDATA].cbrtn) != NULL ) {
												//status = (*cbptr)( gptr->cbtab[SI_CB_CDATA].cbdata, fd, gptr->rbuf, status );
												status = (*cbptr)( );
												SIcbstat( status, SI_CB_CDATA );   // handle cb status 
											}
										} else { 								    // no bites, but read flagged indicates disconnect
											if( (cbptr = gptr->cbtab[SI_CB_DISC].cbrtn) != NULL ) {
												//status = (*cbptr)( gptr->cbtab[SI_CB_DISC].cbdata, tpptr->fd );
												status = (*cbptr)( );
												SIcbstat( status, SI_CB_DISC );    // handle status 
											}
											SIterm( tpptr );
										}
									} 				// end tcp read 
								}
							}						// end event on this fd
						}							// end for each fd in the list 
					}								// if still good fd 
				}									// end for all in list
			}									// end if not in shutdown 
		}										// end while we have had deaths to clean up
	} while( gptr->tplist != NULL && !(gptr->flags & GIF_SHUTDOWN) );

	free( ibuf );
	if( gptr->tplist == NULL ) {        // indicate all fds closed 
		SIerrno = SI_ERR_NOFDS;
	}

	if( gptr->flags & GIF_SHUTDOWN ) {      // we need to stop for some reason 
		SIerrno = SI_ERR_SHUTD;             // indicate error exit status 
		status = ERROR;                // status should indicate to user to die 
		SIshutdown( gptr );            // clean things up 
	} else {                          // end if shutdown 
		status = OK;               // user can continue to process 
	}

	free( uaddr );              // free allocated buffers 

	return( status );           // send status back to caller 
}                            // SIwait 


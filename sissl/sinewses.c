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
*  Mnemonic: SInewsession
*  Abstract: This routine can be called when a request for connection is
*            received. It will establish a new fd, create a new
*            transport provider block (added to the head of the list) and
*            will optionally fork a new child if the fork flag is set in the
*            general info block. The security callback and connection callback
*            routines are driven from this routine.
*  Parms:    gptr - Pointer to the general information block
*            tpptr- Pointer to the tp block that describes the fd that
*                   received the connection request.
*  Returns:  OK if all went well, ERROR if not.
*  Date:     26 March 1995
*  Author:   E. Scott Daniels
*
******************************************************************************
*/
#include "sisetup.h"          /* get necessary defs etc */

extern int SInewsession( struct tp_blk *tpptr )
{
	extern struct ginfo_blk *gptr;
	struct sockaddr *addr;             /* pointer to address of caller */
	struct spxopt_s *sopts;            /* pointer to spx options */
	struct tp_blk *newtp;              /* pointer at new tp block */
	int status = OK;                   /* processing status */
	int cpid;                          /* child process id after fork */
	int (*cbptr)();                    /* pointer to callback function */
	unsigned int addrlen;				/* length of address from accept */
	char *buf = NULL;					/* pointer to address */
	int		fdmode;						/* mode to make non-blocking */

	addr = (struct sockaddr *) malloc( sizeof( struct sockaddr_in ) );
	addrlen = sizeof( struct sockaddr_in );

	status = accept( tpptr->fd, addr, &addrlen );   /* accept and assign new fd */

	if( status >= OK )     							/* session accepted and new fd assigned (in status) */
	{
		newtp = SInew( TP_BLK );      				/* get a new tp block for the session */
		if( newtp != NULL )
		{
			newtp->next = gptr->tplist;  			/* add new block to the head of the list */
			if( newtp->next != NULL )
				newtp->next->prev = newtp;      	/* back chain to us */
			gptr->tplist = newtp;
			newtp->paddr = (struct sockaddr *) addr; /* partner address*/
			newtp->fd = status;                         /* save the fd from accept */

			fdmode = fcntl( newtp->fd, F_GETFL, 0 );		/* make the socket non-blocking so that ssl reads don't hang */
			fdmode |= O_NDELAY;
			if( fcntl( newtp->fd, F_SETFL, fdmode) )
			{
				fprintf( stderr, "sissl/newsess: cannot make socket non-blocking\n" );
				close( newtp->fd );
				SIterm( newtp );
				newtp = NULL;
				status = ERROR;
			}


			if( (cbptr = gptr->cbtab[SI_CB_SECURITY].cbrtn) != NULL )
			{												/*  call the security callback module if there */
				SIaddress( addr, (void **) &buf, AC_TODOT );
				status = (*cbptr)( gptr->cbtab[SI_CB_SECURITY].cbdata, buf );
				if( status == SI_RET_ERROR )    			 /* session to be rejected */
				{
 					close( newtp->fd );     
 					SIterm( newtp );               			/* terminate new tp block */
 					newtp = NULL;							/* indicates failure later */
				}
				else
					SIcbstat( status, SI_CB_SECURITY );    		/* handle other status */
				free( buf );
			}                                    			/* end if call back was defined */

			if( newtp != NULL )								/* accept and security callback successful */
			{
				newtp->flags |= TPF_SESSION;     			/* indicate a session here */
		
				if( gptr->flags & GIF_FORK )     			/* create a child to handle session? */
				{
 					gptr->childnum++;                      	/* increase to next value */
 					if((cpid = fork( )) < 0 )               /* fork error */
  						status = ERROR;                        /* indicate this */ 
 					else
  						if( cpid > 0 )                          /* this is the parent */
   						{
							gptr->tplist->flags &= ~TPF_UNBIND;   /* turn off unbind flag */
							SIterm( newtp );                	/* close child's fd */
   						}
  						else
   						{                                      	/* this is the child */
							while( gptr->tplist->next != NULL )   /* close parents fds */
	 						{
	  							if( gptr->kbfile >= OK )
	  	 							gptr->kbfile = -1;                 /* pseudo close the file */
	  							gptr->tplist->next->flags &= ~TPF_UNBIND;  /* just close */
	  							SIterm( gptr->tplist->next );        /* in term call */
	 						}
							gptr->flags |= GIF_AMCHILD;             /* set child indication */
   						}                        					/* end child setup */
				}                           						

				newtp->ssl = SSL_new( gptr->lctx );			/* create the ssl for the session */
				SSL_set_fd( newtp->ssl, newtp->fd );		/* and attach it to the socket */
				if( SSL_accept( newtp->ssl ) != 1 )			/* do the handshake */
				{
					/* do graceful shutdown */
				}

						/* if conn call back defined and child or not forking */
				if( (cbptr = gptr->cbtab[SI_CB_CONN].cbrtn) != NULL && ( (gptr->flags & GIF_AMCHILD) || ! (gptr->flags & GIF_FORK) ) )
				{                                               
 					SIaddress( addr, (void **) &buf, AC_TODOT );              /* convert address */
 					status=(*cbptr)( gptr->cbtab[SI_CB_CONN].cbdata, newtp->fd, buf );
 					SIcbstat(  status, SI_CB_CONN );				/* handle status */
					free( buf );
				}												
			}												
			else                                        			/* accept failed */
				if( newtp != NULL )
					SIterm(  newtp );      							/* trash the provider block, & close */
		}                              								/* end if established new fd ok */
		else
			status = ERROR;              							/* indicate problems */
	}                              									/* end if listen completed successfully */
	else                            								/* listen failed */
	{
		free( addr );
		status = ERROR;                								/* indicate errors */
	}

	return( status );
}                                


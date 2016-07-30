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
*  Mnemonic: SIinitialise (SIinit is deprecated)
*  Abstract: Initialise the Socket Interface environment. 
*            It does these things:
*		- allocate the master global information block 
*            This routine also owns the global definitons.
*  Parms:    opts - Options:
*              SI_OPT_FORK - Fork a new process for each session connected
*              SI_OPT_FG   - Keep task in foreground.
*              SI_OPT_TTY  - Open and poll the stdin device
*              SI_OPT_ALRM - Setu to trap the alarm signal
*
*  Returns:  1 for ok, and 0 if something failed 
*           
*          
*  Date:     26 March 1995
*  Author:   E. Scott Daniels
*
*  Mod:		17 Feb 2002 - To convert to a globally managed gpointer 
*			09 Mar 2007 - To allow for ipv6 (added SIinitialise() to replace SIinit())
*			03 Feb 2013 - To port to the sissl overlay library.
**************************************************************************
*/

#define _SI_ERRNO			// prevent errno from being declared external in setup.h
#include  "sisetup.h"     /* get the setup stuff */

static char	*cert_file = NULL;
static char *root_file = NULL;
static char *chain_file = NULL;
static char *key_file = NULL;
static int sissl_options = 0;

/*
	allow for setup such that SIinitialise and SIconnect calls remain 
	the same between SI and SIssl versions of the library.

	Cert and root files are the .pem files that are needed by ssl server. 
	tolerated should be one of the TOLERATE settings that connect() will
	use when deciding whether or not to reject a certificate. 

	key and chain files are optional, if null, we assume the cert file can be 
	used in their place (i.e. the cert file has the private key embedded). 
*/
extern void SIssl_initialise( char *cfile, char *rfile, char *kfile, char *chfile, int tolerated, int options )
{
	if( cert_file )
	{
		free( cert_file );
		cert_file = NULL;
	}

	if( root_file )
	{
		free( root_file );
		root_file = NULL;
	}

	if( chain_file )
	{
		free( chain_file );
		chain_file = NULL;
	}

	if( key_file )
	{
		free( key_file );
		key_file = NULL;
	}

	if( cfile) 							/* these could be null if just a client is being written */
		cert_file = strdup( cfile );

	if( rfile )
		root_file = strdup( rfile );

	if( kfile )
		key_file = strdup( kfile );
	else
		key_file = cert_file;

	if( chfile )
		chain_file = strdup( chfile );
	else
		chain_file = strdup( cert_file );

	SIssl_set_conn_tolerence( tolerated );			// set in global where needed
	//conn_tolerence = tolerated;
	sissl_options = options;
}

/* this routine "owns" the global variables */
int deaths = 0;              /* deaths that the parent is not aware of */
int sigflags = 0;            /* signal processing flags SF_ constants */
int SIerrno;                 /* external error number for caller to use */
struct ginfo_blk *gptr;      /* pointer at general info */

/* 	new -- just set up but dont open any listen ports. If user wants listen ports, 
	then they need to call SIlistener() on their own so that they can specify
	a target address that selects ipv4 or ipv6.
	Old progs using SIinit() get v4 ports by default.

*/
extern int SIinitialise( int opts )
{
	int 	status = OK;            /* status of internal processing */
	struct	tp_blk *tpptr;         	/* pointer at tp stuff */
	struct	sigaction sact;			/* signal action block */
	int	i;                      
	int	signals = SI_DEF_SIGS;  	/* signals to be set in SIsetsig */

	SIerrno = SI_ERR_NOMEM;       	/* setup in case alloc fails */

	gptr = SInew( GI_BLK );                /* get a gi block */
	if( gptr != NULL )                     /* was successful getting storage */
	{
		gptr->kbfile = -1;                   /* default to no keyboard */
		gptr->rbuf = (char *) malloc( MAX_RBUF );   /* get rcv buffer*/
		gptr->rbuflen = MAX_RBUF;

		if( opts & SI_OPT_ALRM )     		/* turn on alarm signal too? */
			signals |= SI_SIG_ALRM;     	/* add it to the list */
		SIsetsig( signals );         		/* set up signals that we want to catch */

		if( opts & SI_OPT_FORK )    		/* does caller want to fork for sessions? */
		{
			gptr->flags |= GIF_FORK;            /* yes - set flag to cause this */
	
			memset( &sact, 0, sizeof( sact ) );
			sact.sa_handler = &SIsignal;         /* setup our signal trap */
			sigaction( SIGCHLD, &sact, NULL );    /* also get signals on child deaths*/
		}

		SIerrno = SI_ERR_TPORT;

		SSL_library_init();

		//gptr->sctx = SSL_CTX_new( SSLv23_method() );			/* sslv2/3 or tls v1 protocols */

		gptr->lctx = SSL_CTX_new( TLSv1_server_method() );			/* set up contexes for linsten and connection generated sessions */
		gptr->cctx = SSL_CTX_new( TLSv1_client_method() );

		//SSL_CTX_set_options( tllctx, SSL_OP_SINGLE_DH_USE );

		if( cert_file != NULL  && root_file != NULL )
		{
			SSL_CTX_use_certificate_file( gptr->lctx, cert_file, SSL_FILETYPE_PEM);  /* hook in the certificate info */
			SSL_CTX_use_PrivateKey_file( gptr->lctx, key_file, SSL_FILETYPE_PEM);
			SSL_CTX_use_certificate_chain_file( gptr->lctx, chain_file );
			SSL_CTX_load_verify_locations(gptr->lctx, root_file, NULL );        /* the root.pem file */

			if( !(sissl_options & SISSL_OPT_NOCLCERT) )						/* if certs can be associated with client context (default unless opt set in ssl_init */
			{
				SSL_CTX_use_certificate_file( gptr->cctx, cert_file, SSL_FILETYPE_PEM);  /* hook in the certificate info */
				SSL_CTX_use_PrivateKey_file( gptr->cctx, key_file, SSL_FILETYPE_PEM);
				SSL_CTX_use_certificate_chain_file( gptr->cctx, chain_file );
				SSL_CTX_load_verify_locations(gptr->cctx, root_file, NULL );        /* the root.pem file */
			}
		}

		if( ! (opts & SI_OPT_FG) )    			/* user has not selected fground option */
		{		                            	/* so detach us from control terminal */
			SIerrno = SI_ERR_FORK;
			
			chdir( "/" );                    /* prevent causing unmounts to fail */
			status = fork( );                /* fork to ensure not process grp lead */
			if( status > 0 )                  /* if parent */
				exit( 1 );                         /* then just get out */
			if( status < 0 )                  /* fork failed */
			{
				free( gptr );                   /* so trash the block and */
				return 0;                 /* get out while the gettn's good */
			}
							/* we are the child */
			setpgrp( );                       /* set us as the process group leader */
			close( 0 );                       /* close the stdxxx files */
			close( 1 );
			close( 2 );
		}
		else                         /* if in foreground we can have a keyboard */
			if( opts & SI_OPT_TTY )                     /* does user want it ? */
				gptr->kbfile = 0;                   /* "open" the file */

		gptr->cbtab = (struct callback_blk *) malloc(
			(sizeof( struct callback_blk ) * MAX_CBS ) );
		if( gptr->cbtab != NULL )
		{
			for( i = 0; i < MAX_CBS; i++ )     /* initialize call back table */
			{
				gptr->cbtab[i].cbdata = NULL;    /* no data and no functions */
				gptr->cbtab[i].cbrtn = NULL;
			}
		}                   /* end if gptr->ctab ok */
		else                 /* if call back table allocation failed - error off */
		{
			SIshutdown( gptr );  /* clean up any open fds */
			free( gptr );
			gptr = NULL;       /* dont allow them to continue */
		}
	}                     /* end if gen infor block allocated successfully */

	return 1;        	/* all's well that ends well */
} 

#ifdef KEEP
/* not rolled over with this port */


/* deprecated
	if invoked (old code) we will init, and then open two ipv4 ports as requested by 
	tport and uport. 
*/
int SIinit( int opts, int tport, int uport,  )
{
	struct	tp_blk *tpptr;         /* pointer at tp stuff */
	int	status = OK;
	char	abuf[128];		/* buffer to build default listen address in */

	if( SIinitialise( opts ) )
	{
		if( tport > 0 )                      /* if user wants a TCP listen socket */
		{
			sprintf( abuf, "0.0.0.0;%d", tport );
			status = SIlistener( TCP_DEVICE, abuf );
		}

		if( uport >= 0 && status != ERROR )       /* does user want a UDP port? */
		{
			sprintf( abuf, "0.0.0.0;%d", uport );
			SIerrno = SI_ERR_UPORT;
			status = SIlistener( UDP_DEVICE, abuf );		/* open udp listen port */
		} 					                        /* end if specific udp port numer */

		if( status == ERROR )         /* if an establish failed or wrong port */
		{
			SIshutdown( gptr );           /* clean up the list */
			free( gptr );
			return 0;             /* get out now with an error to user */
		}

		return status == ERROR ? 0 : 1;
	}
	else
		fprintf( stderr, "siinit: failed to initialise\n" );

	return 0;
}
#endif

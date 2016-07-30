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
***************************************************************************
*
* Mnemonic: sistruct.h
* Abstract: This file contains the structure definitions ncessary to support
*           the SI (Socket interface) routines.
* Date:     26 March 1995
* Author:   E. Scott Daniels
*
******************************************************************************
*/

struct ioq_blk              /* block to queue on session when i/o waiting */
{
	struct ioq_blk *next;     /* next block in the queue */
	char *data;               /* pointer to the data buffer */
	unsigned int nalloc;		/* allocated length */
	unsigned int dlen;        /* data length */
	void *addr;               /* address to send to (udp only) */
	int alen;				/* size of address struct (udp) */
 };

struct callback_blk         /* defines a callback routine */
{
	void *cbdata;            /* pointer to be passed to the call back routine */
	int ((*cbrtn)( ));       /* pointer to the callback routine */
};

struct tp_blk               /* transmission provider info block */
{
	struct tp_blk *next;      	/* chain pointer */
	struct tp_blk *prev;      	/* back pointer */
	int fd;                   	/* open file descriptor */
	int flags;                	/* flags about the session / fd */
	int type;                 	/* connection type SOCK_DGRAM/SOCK_STREAM */
	int family;			/* address family */
	struct sockaddr *addr; 		/* connection local address */
	struct sockaddr *paddr; 	/* session partner's address */
	struct ioq_blk *squeue;   	/* queue to send to partner when it wont block */
	struct ioq_blk *sqtail;   	/* last in queue to eliminate the need to search */
	
	SSL	*ssl;					/* ssl for the session */
	char	*rbuf;				/* read buffer */
	int		rbufsize;
};

struct ginfo_blk            /* global - general info block */
{ 
	unsigned int magicnum;     /* magic number that ids a valid block */
	struct tp_blk *tplist;     /* pointer at tp block list */
	/*  struct pollfd *fdlist;*/     /* pointer at file desc list for poll */
	fd_set readfds;            /* set of flags indicating sids we want to test */
	fd_set writefds;           /* by the signal call for read or write */
	fd_set execpfds;           /* by the signal call for read or write */
	char *rbuf;                /* read buffer */
	struct callback_blk *cbtab; /* pointer at the callback table */
	int fdcount;               /* largest fd to select on in siwait */
	int flags;                 /* status flags */
	int childnum;              /* number assigned to next child */
	int kbfile;                /* AFI handle on stdin */
	int rbuflen;               /* read buffer length */

	SSL_CTX	*lctx;				/* context for ssl listen (server) oriented sessions */
	SSL_CTX	*cctx;				/* contex for ssl sessions that we establish with a connection (client) */
};

/* SIHANDLE is the type that the user declares to hold the pointer to the
   general info block. They have a void * type def in nidefs.h, however
   the real ni routines need the real definition for SIHANDLE... this is it */

typedef struct ginfo_blk * SIHANDLE;  /* real def of SIHANDLE for ni routines */


// ------ internal prototypes ------------------------

extern int SIaddress( void *src, void **dest, int );
extern int SIgenaddr( char *target, int proto, int family, int socktype, struct sockaddr **rap );
extern void SIbldpoll( void );
extern void SIcbstat( int status, int type );
extern struct tp_blk *SIestablish( int type, char *abuf, int family );
extern void *SInew( int type );
extern int SInewsession( struct tp_blk *tpptr );
extern void SIsend( struct tp_blk *tpptr );
extern void SIshutdown( struct ginfo_blk *gptr );
extern void SIsignal( int sig );
extern void SIterm( struct tp_blk *tpptr );

extern void SIssl_set_conn_tolerence( int val );
extern int send_ssl( struct tp_blk *tp, struct ioq_blk *qp );

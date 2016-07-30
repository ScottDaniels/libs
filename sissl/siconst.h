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
*****************************************************************************
*
*  Mnemonic:	siconst.h
*  Abstract:	This file contains the constants that are necessary to support
* 				The SIssl  (Socket Interface with ssl) library. This file is NOT
*				published and used only by the sissl build process.
*  Date:		26 March 1995
*  Author:		E. Scott Daniels
*  Mods:		03 Feb 2013  - ported in  from si package 
*				28 Jul 2016 - Added internal prototypes
*
****************************************************************************
*/
#define TRUE  1
#define FALSE 0

#define OK    0
#define ERROR (-1)

#define EOS   '\000'         /* end of string marker */

                              /* general info block flags */
#define GIF_SHUTDOWN   0x01   /* shutdown in progress */
#define GIF_FORK       0x02   /* fork a child for each session if set */
#define GIF_AMCHILD    0x04   /* am a child process */
#define GIF_NODELAY    0x08   /* set no delay flag on t_opens */

                              /* transmission provider block flags */
#define TPF_LISTENFD   0x01   /* set on tp blk that is fd for tcp listens */
#define TPF_SESSION    0x02   /* session established on this fd */
#define TPF_UNBIND     0x04   /* SIterm should unbind the fd */
#define TPF_DRAIN      0x08   /* session is being drained */
#define TPF_DELETE	0x10	/* block is ready for deletion -- when safe */

#define TPF_SSL_MUSTREAD 0x20	/* must issue a read on the ssl context (more network data needed) */
#define TPF_SSL_MUSTWRITE 0x40	/* must issue a write on the ssl context (more unencripted data needed) */
#define TPF_SSL_HANDSHAKE 0x80	/* handshake in progress, need to read but must wait for write to open */

#define MAX_CBS         8     /* number of supported callbacks in table */
#define MAX_FDS        16     /* max number open fds supported */
#define MAX_RBUF       2048   /* max size of receive buffer */

#define TP_BLK    0           /* block types for rsnew */
#define GI_BLK    1           /* global information block */
#define IOQ_BLK   2           /* input output block */

#define MAGICNUM   219        /* magic number for validation of things */

#define AC_TODOT  0           /* convert address to dotted decimal string */
#define AC_TOADDR 1           /* address conversion from dotted dec */
#define AC_TOADDR6 2		/* ipv6 address conversion from string to addr struct */
#define AC_TOADDR6_4BIND 3	/* ipv6 address conversion from string to addr struct suitible for bind */

#define NO_EVENT 0            /* look returns 0 if no event on a fd */



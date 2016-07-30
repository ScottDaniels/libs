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
*  Mnemonic: nisetup.h
*  Abstract: This file contains the necessary include statements for each
*            individually compiled module.
*  Date:     17 January 1995
*  Author:   E. Scott Daniels
*****************************************************************************
*/
#include <stdio.h>              /* standard io */
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>              /* error number constants */
#include <fcntl.h>              /* file control */
#include <netinet/in.h>         /* inter networking stuff */
#include <signal.h>             /* info for signal stuff */
#include <string.h>
#include <sys/types.h>          /* various system files - types */
#include <sys/socket.h>         /* socket defs */

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/rand.h>

/*#include <malloc.h>*/             /* malloc necessary for large model */

/* pure bsd supports SIGCHLD not SIGCLD as in bastard flavours */
#ifndef SIGCHLD
#define SIGCHLD SIGCLD
#endif

#define SI_RTN 1               /* prevent def of SIHANDLE in sidefs.h */

#include "siconst.h"          /* our constants */
#include "sistruct.h"         /* our structure definitions */

#include <sdaniels/socket_if.h>        // constants and prototypes from the si package (assumed published in /usr/local/lib/sdaniels or similar)
#include "sissl.h"				// ssl specific constants and prototypes	

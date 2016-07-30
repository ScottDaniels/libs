/*
=================================================================================================
	(c) Copyright 1995-2013 By E. Scott Daniels. All rights reserved.

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
 ---------------------------------------------------------------------------------------------
*	Abstract: 	websocket_ssl
*	Mnemonic:	These are functions that must be included with the websocket.c functions when 
*				building an ssl capable websocket interface. This library should be listed on 
*				the cc line ahead of the websocket reference. 
*
*	Author:		E. Scott Daniels
*	Date:		12 February 2012
*
*
*	Mods:		
*
 ---------------------------------------------------------------------------------------------
*/
#include <openssl/sha.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <sdaniels/socket_if.h>		/* my socket stuff */
#include <sdaniels/sissl.h>			/* ssl overlay of the silib stuff */
#include <ut.h>


/*
	supplies the cert, root, key and chain .pem files to the underlying SIssl socket interface.
	if we have been linked with the SIssl library, and if the user doesn't invoke this
	first, things will bugger up pretty quickly -- handshakes will fail.
*/
extern void ws_ssl_initialise( unsigned char *cfile, unsigned char *rfile, unsigned char *kfile, unsigned char *chfile )
{
	SIssl_initialise( cfile, rfile, kfile, chfile, 0, 0 );		
}

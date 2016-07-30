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
*	Abstract: 	base64
*	Mnemonic:	Functions to support encoding and decoding (someday) of a text buffer into 
*				base 64. 
*	Author:		E. Scott Daniels
*	Date:		22 November 2012
*
 ---------------------------------------------------------------------------------------------
*/
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

unsigned char soup[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 
	'w', 'x', 'y', 'z', '0', '1', '2', '3', 
	'4', '5', '6', '7', '8', '9', '+', '/' 
};


/*
	encode a complete buffer; 3 input bytes yield 4 output bytes
	If the caller has multiple buffers it must ensure that they are
	multiples of three bytes with the exception of the last one. 
	//1111 1122  2222 3333  3344 4444
	//.... ....  .... ....  .... ....	

	dlen is the length of the destination buffer. The return is a zero
	terminated ascii string of the base64 encoded source string. If the first
	byte of the returned buffer is zero, then the user's buffer was not large enough
	and processing aborted.  

	If the dest pointer is null, one will be allocated and dlen will be 
	ignored. 
*/
extern char *buf2base64( unsigned char *src, unsigned char *dest, int len, int dlen )
{
	unsigned char byte;
	int	i;
	int di = 0;

	if( ! dest )
	{
		dlen = sizeof( char ) * (((len/3)+1) * 4);
		dest = (unsigned char *) malloc( dlen + 1 );
		if( ! dest )
			return NULL;
	}

	while( len >= 3 )
	{
		if( di + 4 > dlen )		/* prevent overrun of caller's buffer */
		{
			*dest = 0;
			return dest;
		}

		for( i = 0; i < 4; i++ )
		{
			switch( i )
			{
				case 0: byte = *src >> 2;	/* left most 6 bits from byte 0 */
						break;

				case 1: byte = (*src & 0x03) << 4;	/* right 2 bits from byte 1 */
						src++;
						byte |= *src >> 4;		/* left 4 bits from byte 2 */
						break;

				case 2: byte = (*src & 0x0f) << 2;	/* right 4 bits from byte 2 */
						src++;
						byte |= *src >> 6;			/* two left bits from byte 3 */
						break;

				case 3: byte = *src & 0x3f;
						src++;
						break;
			}

			dest[di++] = soup[byte];
		}
		len -= 3;
	}
	
	if( len > 0 )
	{
		if( di + 4 > dlen )
		{
			*dest = 0;
			return dest;
		}

		byte = *src >> 2;				/* left most 6 bits from byte 0 */
		dest[di++] = soup[byte];
		if( len > 1 )
		{
			byte = (*src & 0x03) << 4;		/* right 2 bits from byte 1 */
			src++;
			byte |= *src >> 4;						/* left 4 bits from byte 2 */
			dest[di++] = soup[byte];

			byte = (*src & 0x0f) << 2;				/* right 4 bits from byte 2 nothing else cuz there's no byte 3*/
			dest[di++] = soup[byte];
		}
		else
		{
			byte = (*src & 0x03) << 4;		/* right 2 bits from byte 1 and nothing because there's not a byte 2*/
			dest[di++] = soup[byte];
			dest[di++] = '=';	
		}
		dest[di++] = '=';	
	}

	dest[di] = 0;
	return dest;
}

#ifdef XSELF_TEST
#include "sha1.c"

/*
	expected output when default value is used: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=
*/
int main( int argc, char **argv )
{
	int i;
	int len;
	unsigned char sha[1024];
	unsigned char out[1024];
	unsigned char *b64;
	unsigned char *in = "dGhlIHNhbXBsZSBub25jZQ==258EAFA5-E914-47DA-95CA-C5AB0DC85B11";		/* default -- websocket like key value */

	len = SHA_DIGEST_LENGTH;
	if( argc < 2 )
	{
		sha1_raw( in, strlen( in ), sha );
	}
	else	
	{
		if( strcmp( argv[1], "sha1" ) == 0 )
			sha1_raw( argv[2], strlen( argv[2] ), sha );	/* compute base64 on the sha1 value of parm */
		else
		{
			strcpy( sha, argv[1] );			/* just compute base64 on the parm */
			len = strlen( sha ); 
		}
	}
	memset( out, 'a', 1024 );

	buf2base64( sha, out, len, 1024 );
	fprintf( stdout, " static: %s\n", out );	

	b64 = buf2base64( sha, NULL, len,  0 );
	fprintf( stdout, "dynamic: %s\n", b64 );	
}
#endif

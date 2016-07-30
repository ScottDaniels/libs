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
 -----------------------------------------------------------------------
	Mnemonic:	sha1
	Abstract:	Interface to computing sha1 or sha256 on various things:
					individual buffer
					all records, individually, from a file
					an entire file
					a list of files

				Uses openssl to do the hard work :)
	Date: 		04 July 2012
	Author:		E. Scott Daniels
 -----------------------------------------------------------------------
*/

#include <openssl/sha.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

char *sha_version = "lib/sha1 version 1.0/17042";
/* --------------------- localised suppport ------------------------------- */
/*
	convert the right nibble of byte n into  a one byte ascii representation
*/
static unsigned char nibble2ascii( unsigned char n )
{
	return n > 9 ?  'a' + (n - 10) : '0' + n;
}




/* ------------------ sha1 (128 bit) things ------------------------------- */

/*
	allocate a buffer large enough to hold a hash value string (including final null)
*/
extern unsigned char *sha1_mk_buffer( )
{
	unsigned char *p;
	p =  (unsigned char *) malloc( sizeof( char ) * ((SHA_DIGEST_LENGTH * 2)+1) ); 

	if( ! p )
	{
		fprintf( stderr, "sha1_mk_buffer: malloc error allocating hash ascii buffer"  );
		abort( );
	}

	return p;
}


/*
	compute the sha1 on src and then 'unpack' it into hash; hash is allocated if NULL is passed
	pointer to new buf, or hash, is returned. 
*/
extern unsigned char *sha1_buf( unsigned char *src, unsigned long len, unsigned char *hash )
{
	unsigned char rhash[SHA_DIGEST_LENGTH];			/* raw hash */
	int i;
	int j = 0;

	if( hash == NULL )
		hash = sha1_mk_buffer( );

	SHA1( src, len, rhash );
	for( i = 0; i < SHA_DIGEST_LENGTH; i++ )
	{
		hash[j] = nibble2ascii( rhash[i] >> 4 );
		hash[j+1] = nibble2ascii( rhash[i] & 0x0f );
		j += 2;
	}

	hash[j] = 0;

	return hash;
}


/*
	same as sha1_buf, but does not unpack it into hash, returns raw bytes
*/
extern unsigned char *sha1_raw( unsigned char *src, unsigned long len, unsigned char *hash )
{
	if( hash == NULL )
	{
		if( (hash = (unsigned char *) malloc( sizeof( unsigned char ) * SHA_DIGEST_LENGTH )) == NULL )
		{
			fprintf( stderr, "sha1_raw: malloc error allocating hash buffer"  );
			abort( );
		}
	}

	SHA1( src, len, hash );
	return hash;
}


	//SHA_CTX ctx;
//	SHA1_Init( &ctx );
	//SHA1_Update( &ctx, src, len );
 	//SHA1_Final( hash, &ctx ); 


/* -------------- sha256 support ------------------------------------------------------- */

/*
	allocate a buffer large enough to hold a hash value string (including final null)
*/
extern unsigned char *sha256_mk_buffer( )
{
	unsigned char *p;
	p =  (unsigned char *) malloc( sizeof( char ) * ((SHA256_DIGEST_LENGTH * 2)+1) ); 

	if( ! p )
	{
		fprintf( stderr, "sha256_mk_buffer: malloc error allocating hash ascii buffer"  );
		abort( );
	}

	return p;
}

/*
	convert the raw hash into an ascii buffer
*/
extern unsigned char *sha256_hash2buf( unsigned char *hash, unsigned char *rhash )
{
	int i;
	int j;

	if( hash == NULL )
		hash = sha256_mk_buffer( );

	j = 0;
	for( i = 0; i < SHA256_DIGEST_LENGTH; i++ )
	{
		hash[j] = nibble2ascii( rhash[i] >> 4 );
		hash[j+1] = nibble2ascii( rhash[i] & 0x0f );
		j += 2;
	}

	hash[j] = 0;
	return hash;
}

/*
	compute the sha256 on src and then 'unpack' it into hash; hash is allocated if NULL is passed
	pointer to new buf, or hash, is returned. 
*/
extern unsigned char *sha256_buf( unsigned char *src, unsigned long len, unsigned char *hash )
{
	unsigned char rhash[SHA256_DIGEST_LENGTH];			/* raw hash */

	if( hash == NULL )
		hash = sha256_mk_buffer( );

	SHA256( src, len, rhash );

	return sha256_hash2buf( hash, rhash );
}
/*
	same as sha256_buf, but does not unpack it into hash, returns raw bytes
*/
extern unsigned char *sha256_raw( unsigned char *src, unsigned long len, unsigned char *hash )
{

	if( hash == NULL )
	{
		if( (hash = (unsigned char *) malloc( sizeof( unsigned char ) * SHA256_DIGEST_LENGTH )) == NULL )
		{
			fprintf( stderr, "sha256_raw: malloc error allocating hash buffer"  );
			abort( );
		}
	}

	SHA256( src, len, hash );
	return hash;
}

/*
	compute sha256 on the given file and return the binary hash (unsigned char)
*/
extern unsigned char *sha256_filer( char *fname, unsigned char *hash )
{
	SHA256_CTX ctx;
	unsigned char	*rbuf[8192];
	int	fd;
	int rlen;

	if( hash == NULL )
	{
		if( (hash = (unsigned char *) malloc( sizeof( unsigned char ) * SHA256_DIGEST_LENGTH )) == NULL )
		{
			fprintf( stderr, "sha256_file: malloc error allocating hash buffer"  );
			abort( );
		}
	}

	if( (fd = open( fname, O_RDONLY )) < 0 )
	{
			fprintf( stderr, "sha256_file: unable to open file: $s", strerror( errno )  );
			return NULL;
	}
	
	SHA256_Init( &ctx );

	while( (rlen = read( fd, rbuf, sizeof( rbuf ))) > 0 )
		SHA256_Update( &ctx, rbuf, rlen );

 	SHA256_Final( hash, &ctx ); 

	close( fd );

	return hash;
}

/*
	compute the sha356 on the file, and return an ascii hash
	fname -- file name to parse
	hash -- pointer to buffer (65 bytes min) or NULL; when NULL the buffer is allocated
*/
extern unsigned char *sha256_fileb( char *fname, unsigned char *hash )
{
	unsigned char *rhash = NULL; 

	if( hash == NULL )
		hash = sha256_mk_buffer( );

	rhash = sha256_filer( fname, NULL );			/* build the raw hash */
	sha256_hash2buf( hash, rhash );
	
	free( rhash );
	return hash;
}

/* -------------------------------------------------------------------------*/

#ifdef SELF_TEST
void dump( unsigned char *src, int len, char *tag )
{
	int i;

	if( tag )
		fprintf( stderr, "%s: ", tag );

	for( i = 0; i < len; i++ )
		fprintf( stderr, "%02x ", (unsigned int) *src++ );
	fprintf( stderr, "\n" );
}

int main( int argc, char **argv )
{
	unsigned char *hash; 

	if( argc <= 1 ) {
		fprintf( stderr, "usage: %s file filename\n       %s string\n", argv[0], argv[0] );
		exit( 0 );
	}

	if( strcmp( argv[1], "file" ) == 0 )
	{
		printf( "generating sha256 on file: %s\n", argv[2] );
		if( argv[2] != NULL )
		{
			hash = sha256_fileb( argv[2], NULL );
			fprintf( stderr, "%s\n", hash );
		}
		else
			fprintf( stderr, "missing file name\n" );

		exit( 0 );
	}

	hash = sha256_raw( argv[1], strlen( argv[1] ), NULL );
	dump( hash, SHA256_DIGEST_LENGTH, "raw>>" );
	free( hash );

	hash = sha256_buf( argv[1], strlen( argv[1] ), NULL );
	fprintf( stderr, "%s\n", hash );
	free( hash );

}


#endif

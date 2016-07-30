/*
 ---------------------------------------------------------------------------------------------
*	Monemonic: 	tokenise
*	Abstract:	yet another set of functions to tokenise a buffer.
*	Author:		E. Scott Daniels
*	Date:		08 February 2013
*
 ---------------------------------------------------------------------------------------------
*/
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>


/*
	Convert a string into tokens; trashes the string.
	This does return null tokens if there are two adjacent seperator characters
	If there are more tokens than max, the last token (max-1 in the array) is the 
	remaining portion of the string. 
*/
extern int tokenise( unsigned char *buf, unsigned char sep, char **tokens, int maxtokens )
{
	int		n = 0;
	char	*tp;		/* pointer at termination */
	char	*sp;		/* pointer at start */

	

	tokens[0] = NULL;			/* we guarentee this is NULL if string is empty */
	if( (sp = buf) == NULL )
		return 0;

	while( *sp && n < maxtokens-1 )
	{
		tokens[n++] = sp;
		for( tp = sp; *tp && *tp != sep; tp++ );	/* find end */
		if( *tp )
			*(tp++) = 0;			/* mark it and advance to next if this is sep */
		sp = tp;
	}
	if( *sp )
		tokens[n++] = sp;			/* the rest if we had more tokens than max */

	return n;
}


#ifdef SELF_TEST

main( int argc, char **argv )
{
	char	*tokens[1024];
	int		ntokens;
	int		i;

	if( argc < 3 )
	{
		fprintf( stderr, "usage: %s ntokens string\n", argv[0] );
		exit( 1 );
	}
	ntokens = tokenise( argv[2], ' ', tokens, atoi( argv[1] ) );

	printf( "%d tokens\n", ntokens );
	for( i = 0; i < ntokens; i++ )
		printf( "token[%d] = %s\n", i, tokens[i] );
}
#endif

/*
	These are sissl specific header needs. We will include socket_if.h if the user didn't
*/

#ifndef _SISSL_H
#define _SISSL_H

 										/* connection cert failure tolerence levels passed to siconnect() */
#define	SISSL_NO_TOLERENCE 0x00			/* no error tolerated;  fail connection on any error */
#define	SISSL_TOLERATE_INV 0x01			/* tolerate an invalid cert */
#define	SISSL_TOLERATE_NAME 0x02		/* tolerate a name mismatch */

									/* options passed on sissl_initialise() */
#define SISSL_OPT_NOCLCERT	0x0		/* certificates are NOT to be associated with the client ssl context */

// ------------- ssl specific prototypes ----------------------------------------------------------------------
void SIssl_initialise( char *cfile, char *rfile, char *kfile, char *chfile, int tolerated, int options ); /* call before initialise when using ssl lib */

#endif

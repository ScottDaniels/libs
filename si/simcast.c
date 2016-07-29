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
****************************************************************************
*
*  Mnemonic:	ng_mcast
*  Abstract:	multi cast oriented routines
*			SImcast_join() -- join a multicast group 
*			SImcast_leave() -- depart from the group 
*			SImcast_ttl() - set time to live for multi cast packets
*  Parms:    
*  Returns:  
*  Date:	23 Aug 2004
*  Author:	E. Scott Daniels
*
*  Mods:	
*****************************************************************************
*/
#include	<sys/types.h>          /* various system files - types */
#include 	"sisetup.h"     

/* bloody suns */
#ifdef OS_SOLARIS
typedef unsigned int u_int32_t;
#endif


/* rather than re-inventing the wheel, we rely on the ng_mcast functions implemented in the library */
extern int SImcast_join(  char *group, int iface, int ttl )
{
	extern struct ginfo_blk *gptr;
	struct	tp_blk *tpptr;       /* pointer at the tp_blk for the session */
	unsigned char opt = 0;
	u_int32_t uiface;

	if( iface < 0 )
		uiface = INADDR_ANY;
	else
		uiface = iface;

	
	for( tpptr = gptr->tplist; tpptr != NULL && tpptr->type != SOCK_DGRAM; tpptr = tpptr->next );    /* find the first udp (raw) block */
	if( tpptr != NULL )            /* found block for udp */
	{
		if( ng_mcast_join( tpptr->fd, group, uiface, (unsigned char) ttl ) )
		{
			if( ttl >= 0 );
			{
				opt = ttl;
				setsockopt( tpptr->fd, IPPROTO_IP, IP_MULTICAST_TTL, &opt, sizeof( opt ) );
			}
			return SI_OK;
		}
	}

	return SI_ERROR;
}

extern int SImcast_leave( char *group, int iface )
{
	extern struct ginfo_blk *gptr;
	struct	tp_blk *tpptr;       /* pointer at the tp_blk for the session */

	if( iface < 0 )
		iface = INADDR_ANY;

	for( tpptr = gptr->tplist; tpptr != NULL && tpptr->type != SOCK_DGRAM; tpptr = tpptr->next );    /* find the first udp (raw) block */
	if( tpptr != NULL )            /* found block for udp */
	{
		if( ng_mcast_leave( tpptr->fd, group, iface ) )
			return SI_OK;
	}

	return SI_ERROR;
}

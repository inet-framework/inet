
/*
	file: hook_types.h
	Purpose: constants definition of Message kinds that are used
		within DF-hooks and in the interfaces to DF-Hooks
	Usage:
		Messages of the class cPacket have
		automatically the kind MK_PACKET (-1) and can
		only take the values MK_PACKET (-1) or MK_INFO (-2).
		Other message kinds are free of choice.

		These kinds have only local validity in the DF-hooks
		and interfaces to DF-hooks!!!
	author: Jochen Reber
*/

#ifndef __HOOK_TYPES_H__
#define __HOOK_TYPES_H__

/*  -------------------------------------------------
        Constants
    -------------------------------------------------   */

// definition of message kind constants
const int	PACKET_ENQUEUED = 1,
			DISCARD_PACKET = 2,
			WAKEUP_PACKET = 3,
			REQUEST_PACKET = 10,
			NO_PACKET = 11,
			LB_WAKEUP = 12,
			NWI_IDLE = 13;

#endif

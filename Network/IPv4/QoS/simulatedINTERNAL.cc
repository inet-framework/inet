/***************************************************************************
                          simulatedINTERNAL.cc  -  description
                             -------------------
    begin                : Sat Sep 9 2000
    copyright            : (C) 2000 by Dirk Holzhausen
    email                : dholzh@gmx.de
 ***************************************************************************/

#include "omnetpp.h"
#include "hook_types.h"
#include "simulatedINTERNAL.h"


/*
		createINTERNALMessage:
				this method returns a pointer to a message object with the specified message kind.
				
				right now always a new message is created, in future message reusing is planned.
*/

cMessage *createINTERNALMessage ( int kind ) {

	const char *sKind;
	
	switch ( kind ) {
		case PACKET_ENQUEUED : sKind = "INTERNAL_Packet_Enqueued"; break;
		case DISCARD_PACKET  : sKind = "INTERNAL_Discard_Packet";  break;
		case REQUEST_PACKET  : sKind = "INTERNAL_Request_Packet";  break;
		case LB_WAKEUP       : sKind = "INTERNAL_LB_Wakeup";       break;
		case NO_PACKET       : sKind = "INTERNAL_No_Packet";			 break;
		default							 : sKind = "INTERNAL_Packet";				   break;
	
	}
	
	return new cMessage ( sKind, kind );
	
}


/*
		deleteINTERNALMessage:
				delete a message, in future: put message on a cHead queue and re-use message.
*/

void deleteINTERNALMessage ( cMessage *msg ) {

	delete msg;
	
}



/*
		isINTERNALDataPacket:
			check if this is a data packet or a status packet
*/

bool isINTERNALDataPacket ( cMessage *msg ) {

	return ( msg->kind () < 0 ? true : false );

}


/*
		findINTERNALQueue:
				check if the current module has a queue pointer parameter. if yes, convert queue pointer to
				a class pointer
	
*/

ANY_Queue *findINTERNALQueue ( cModule *startMod, const char *searchStr ) {

	// search module

	cModule *curMod = findINTERNALModule ( startMod, searchStr );					
	
	// get queue pointer	
	
	ANY_Queue *myQueue = NULL;
	
	if ( curMod ) {
		int parIndex = curMod->findPar ( PAR_QUEUE_POINTER );
		
		// if parIndex >= 0 then parameter was found
		
		if ( parIndex >= 0 ) myQueue = ( ANY_Queue * ) ( curMod->par ( parIndex ) ).pointerValue () ;
		
	}

	return myQueue;
}


/*
		findINTERNALModule:
				- search for a module by name.
				- return first found module.
				
*/


cModule *findINTERNALModule ( cModule *startMod, const char *modName ) {


	cModule *curMod = startMod;
	cModule *fndMod = NULL;
	
	do {
	
		fndMod = ( cModule * ) curMod->findObject ( modName );
		curMod = curMod->parentModule ();
	
	
	} while ( curMod && !fndMod );


	return fndMod;	
	
}

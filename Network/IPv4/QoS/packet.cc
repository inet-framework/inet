/***************************************************************************
                          packet.cc  -  description
                             -------------------
    begin                : Fri Aug 18 2000
    copyright            : (C) 2000 by Dirk Holzhausen
    email                : dholzh@gmx.de
 ***************************************************************************/



#include "packet.h"
#include "IPDatagram.h"
#include "simulatedINTERNAL.h"

#include <string.h>

/*
		constructor
		
*/

Packet::Packet( cMessage *msg ) : myMsg ( msg ) {

#ifdef IP_SIM

	IPDatagram *ipk;
	TransportPacket *tpk;

	TCP = IP_PROT_TCP;
	UDP = IP_PROT_UDP;

	ipk = ( IPDatagram * ) msg;
	
	prot = ipk->transportProtocol ();
	
	if ( prot == IP_PROT_UDP || prot == IP_PROT_TCP ) {
	
		tpk = ( TransportPacket * ) ipk->encapsulatedMsg ();
			
		ipSrcPort  = tpk->sourcePort ();
		ipDestPort = tpk->destinationPort ();			
		
		//ipSrcAddr  = ipk->srcAddress ();				
		//ipDestAddr = ipk->destAddress ();

		// ACHTUNG: hier muss noch IPAddress zu char* umgewandelt werden

		strcpy ( ipSrcAddr, ipk->srcAddress () );
		strcpy ( ipDestAddr, ipk->destAddress () );

		
	} else {
	
		tpk = NULL;
		
		ipSrcPort = ipDestPort = -1;
		strcpy(ipSrcAddr,"");
		strcpy(ipDestAddr, "");

	}
		
	inAdapter   = ipk->inputPort ();			
	
#else

	TCP = 0;				// ACHTUNG: hier müssen noch die richtigen Werte eingetragen werden!!!
	UDP = 1; 				//
	
	prot 			 = ( int ) msg->par ( PAR_IP_PROTOCOL );
	codepoint  = msg->par ( PAR_IP_CODEPOINT );

	strncpy ( ipSrcAddr, msg->par ( PAR_IP_SRC_ADDR ), 29 );
	strncpy ( ipDestAddr, msg->par ( PAR_IP_DEST_ADDR ), 29 );

	ipDestAddr [29] = ipSrcAddr [29]  = '\0';

//	ipSrcAddr  = msg->par ( PAR_IP_SRC_ADDR );
//	ipDestAddr = msg->par ( PAR_IP_DEST_ADDR );

	ipSrcPort  = msg->par ( PAR_IP_SRC_PORT );
	ipDestPort = msg->par ( PAR_IP_DEST_PORT );
	
	inAdapter  = msg->par ( PAR_IP_IN_ADAPTER );
	
#endif

}



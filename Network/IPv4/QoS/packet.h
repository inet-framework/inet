/***************************************************************************
                          packet.h  -  description
                             -------------------
    begin                : Fri Aug 18 2000
    copyright            : (C) 2000 by Dirk Holzhausen
    email                : dholzh@gmx.de
 ***************************************************************************/



#ifndef PACKET_H
#define PACKET_H

#include "TransportPacket.h"
#include "IPInterfacePacket.h"

//  #define IP_SIM

/**
  *@author Dirk Holzhausen
  */


#ifdef IP_SIM

#include "TransportPacket.h"
#include "IPDatagram.h"

#endif

#include "omnetpp.h"


class Packet {

	private:
		cMessage *myMsg;
		
		int prot;
		int codepoint;
		char ipSrcAddr  [30];
		char ipDestAddr [30];
		int ipSrcPort;
		int ipDestPort;
		int inAdapter;
		
#ifdef IP_SIM
		IPDatagram 			*ipk;
		TransportPacket *tpk;
#endif

	public:
		
		// constants
		
		int TCP;
		int UDP;
		
	
		// constructors
	
		Packet( cMessage *msg );
		
		
		// methods
		
		int getCodepoint () { return codepoint; }
		const char *getSrcAddr () { return ipSrcAddr; }
		const char *getDestAddr () { return ipDestAddr; }
		int getSrcPort () { return ipSrcPort; }
		int getDestPort () { return ipDestPort; }
		
		int getInAdapter () { return inAdapter; }
		
		bool isTCPPacket () { return prot == TCP ? true : false; }
		bool isUDPPacket () { return prot == UDP ? true : false; }
		
		int getProtocol () { return prot; }
		
		cMessage *getMessage () { return myMsg; }
};

#endif

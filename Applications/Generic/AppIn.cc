// $Header$
//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

/*
	file: Applications.cc
        Purpose: receives packets of Traffic generators
                 and prints out their information
	author: me
*/


#include <omnetpp.h>
#include "AppIn.h"
#include "IPInterfacePacket.h"
#include "IPDatagram.h"
#include "ICMP.h"


Define_Module( AppIn );

void AppIn::initialize()
{
	strcpy(nodename, par("nodename"));
}
	
void AppIn::activity()
{
	cMessage *msg;

	while(true)
	{
		msg = receive();
		processMessage(msg);
		breakpoint("AppIn receive");
	}

}

// private function
void AppIn::processMessage(cMessage *msg)
{
	IPInterfacePacket *ip = (IPInterfacePacket *)msg;
	cPacket *p = (cPacket *)msg->decapsulate();
	simtime_t arrivalTime = ip->arrivalTime();
	IPProtocolFieldId protocol = (IPProtocolFieldId)ip->protocol();
	int content = p->hasPar("content") 
				? (int)p->par("content") : (int)-1;
	bool isRequest = p->hasPar("request") 
				? p->par("request").boolValue() : false;
	int length = p->length();
	char src[20], dest[20];

	strcpy( src,  ip->srcAddr());
	strcpy( dest, ip->destAddr());

	// print out Packet info
	ev  << "\n+++" << nodename << " AppIn: Packet received:"
		<< "\nProt: " << (protocol == IP_PROT_TCP ? "TCP" : "UDP")
		<< " Cont: " << content 
		<< " Bitlen: " << length
		// << (isRequest ? " Request" : " Reply")
		<< "   Arrival Time: " << arrivalTime
		<< " Simtime: " << simTime()
		<< "\nSrc: " << src 
		<< " Dest: " << dest << "\n\n";
		
	delete (ip);
	delete (p);

	if (!isRequest)
		return;

}


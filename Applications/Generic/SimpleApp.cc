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
	file: SimpleApp.cc
        Purpose: Traffic Generator that can send simple
        TCP or UDP packets at random stastical intervals
	author: Jochen Reber
*/


#include <omnetpp.h>
#include "SimpleApp.h"
#include "IPInterfacePacket.h"
#include "IPDatagram.h"
#include "ICMP.h"

const int GEN_NO = 1;

Define_Module_Like ( SimpleApp, GeneratorAppOut );

void SimpleApp::initialize()
{
	prot = ( ( par("tcpProtocol").boolValue() == true )
			? IP_PROT_TCP 
			: IP_PROT_UDP );
	generationTime = par("generationTime");
	strcpy(nodename, par("nodename"));
	nodenr = par("nodenr");
	packetSize = par("generationSize");
}

void SimpleApp::activity()
{
	int contCtr = nodenr*10000;
	char dest[20];
	cPacket *transportPacket;
	IPInterfacePacket *iPacket;

	while(true)
	{
		wait(truncnormal(generationTime, generationTime * 0.1));

		transportPacket = new cPacket;
		transportPacket->setLength(1 + intrand(packetSize));
		transportPacket->addPar("content") = contCtr++;
		transportPacket->addPar("request") = true;

		iPacket = new IPInterfacePacket;
		iPacket->encapsulate(transportPacket);
		iPacket->setDestAddr(chooseDestAddr(dest));
		iPacket->setProtocol(prot);
		if (intrand(4) == 0)
		{
			iPacket->setDontFragment(true);
		}	
		send(iPacket, "out");

		ev << "\n*** " << nodename 
		<< " Simple Application: Packet sent:"
		<< "\nProt: " << (prot == IP_PROT_TCP ? "TCP" : "UDP")
		<< " Content: " << int(transportPacket->par("content"))
		<< " Bitlength: " << int(transportPacket->length())
		<< (iPacket->dontFragment() ? " DF" : "")
		<< "   Time: " << simTime() 
		<< "\nDest: " << iPacket->destAddr() 
		<< "\n";
		breakpoint("AppOut send");

	} // end while
}

/* random destination addresses
	based on test2.irt */
char *SimpleApp::chooseDestAddr(char *dest)
{
	switch(intrand(2))
	{
		// computer no 1
		case 0: strcpy(dest, "10.10.0.1");
				break;
		// computer no 2
		case 1: strcpy(dest, "10.10.0.2");
				break;
		// DL sink
		case 2: strcpy(dest, "10.20.0.3");
				break;
		// local loopback
		case 3: strcpy(dest, "127.0.0.1");
				break;
		// IP address doesn't exist
		case 4: strcpy(dest, "10.30.0.5"); 
				break;
	}

	return dest;
}


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
    file: IPTunneling.cc
    Purpose: Implementation for Tunneling of IP Packets
    ------
    Responsibilities:
    receive message from IPMulticast
    set Tunnel destination address
    set Protocol field
    send new IPInterfacePacket to IPSend to be newly encapsulated

    author: Jochen Reber

    comment: interface from IPMulticast still needs doc
*/

#include <omnetpp.h>
#include <stdlib.h>

#include "IPTunneling.h"
#include "IPInterfacePacket.h"
#include "ProcessorManager.h"

Define_Module( IPTunneling );

/*  ----------------------------------------------------------
        Public Functions
    ----------------------------------------------------------  */


void IPTunneling::initialize()
{
    ProcessorAccess::initialize();

    delay = par("procdelay");
}

void IPTunneling::activity()
{
	cMessage *msg;
	IPInterfacePacket *packet = new IPInterfacePacket;
	IPDatagram *datagram;
	IPAddrChar dest;

	while(true)
	{
		msg = receive();

		datagram = new IPDatagram(
			*(IPDatagram *)(msg->parList().get("datagram")));
		strcpy(dest, msg->par("destination_address"));

		delete( msg );

		wait(delay);

		packet->encapsulate(datagram);
		packet->setProtocol(IP_PROT_IP);
		packet->setDestAddr(dest);
		send(packet, "sendOut");
		releaseKernel();
	}
}


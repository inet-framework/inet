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

/* 	-------------------------------------------------
	file: IPOutputCore.cc
	Purpose: Implementation for IPOutput core module
	------
	Responsibilities: 
	receive complete datagram from IPFragmentation
	hop counter check 
	-> throw away and notify ICMP if ttl==0
	otherwise  send it on to output queue
	author: Jochen Reber
	------------------------------------------------- 	*/

#include <omnetpp.h>

#include "hook_types.h"
#include "IPOutputCore.h"
#include "ICMP.h"

Define_Module ( IPOutputCore );

/*  ----------------------------------------------------------
        Public Functions
    ----------------------------------------------------------  */

void IPOutputCore::initialize()
{
	ProcessorAccess::initialize();
	delay = par("procdelay");
    hasHook = (findGate("netfilterOut") != -1);

}

void IPOutputCore::activity()
{
    cMessage *dfmsg;
	IPDatagram *datagram;

	while(true)
	{
		datagram = (IPDatagram *)receive();

        // pass Datagram through netfilter if it exists
        if (hasHook)
        {
            send(datagram, "netfilterOut");
            dfmsg = receiveNewOn("netfilterIn");
            if (dfmsg->kind() == DISCARD_PACKET)
            {
                delete dfmsg;
                releaseKernel();
                continue;
            }
            datagram = (IPDatagram *)dfmsg;
        }

		wait(delay);

        // hop counter check
        if (datagram->timeToLive() <= 0)
        {
            sendErrorMessage(datagram, ICMP_TIME_EXCEEDED, 0);
            // drop datagram, destruction responsability of ICMP
            continue;
        }

		send(datagram, "queueOut");

		releaseKernel();

	}
}

   
//  private function: send error message to ICMP Module
void IPOutputCore::sendErrorMessage(IPDatagram *datagram,
        ICMPType type, ICMPCode code)
{

    //  format of this message defined in ICMP.h
    cMessage *icmpNotification = new cMessage();

    datagram->setName("datagram");
    icmpNotification->addPar("ICMPType") = (long)type;
    icmpNotification->addPar("ICMPCode") = code;
    icmpNotification->parList().add(datagram);

    send(icmpNotification, "errorOut");
}



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
	file: PreRoutingCore.cc
	Purpose: Implementation of PreRouting
	Responsibilities:
		receive IP datagram
		check for header error ->
		throw away and notify ICMP Module on error
		hop counter decrement
		send correct datagram to Routing Module
	structure: activity()-loop with receive at the start;
                claimKernel-Msg to ProcessorManager
	author: Jochen Reber
	date: 13.5.00, 15.5.00, 16.6.00
*/

#include <omnetpp.h>

#include "hook_types.h"
#include "PreRoutingCore.h"
//#include "ProcessorManager.h"

Define_Module( PreRoutingCore );


/*  ----------------------------------------------------------
        Public Functions
    ----------------------------------------------------------  */


void PreRoutingCore::initialize()
{
    // ProcessorAccess::initialize();
    delay = par("procdelay");
    hasHook = (findGate("netfilterOut") != -1);
}


void PreRoutingCore::activity()
{

	cMessage *dfmsg;
	IPDatagram *datagram;
	float relativeHeaderLength;

	while(true)
	{

		datagram = (IPDatagram *)receive();

		// notify ProcessorManager
		// claimKernel();

		// pass Datagram through netfilter if it exists
		if (hasHook)
		{
			send(datagram, "netfilterOut");
			dfmsg = receive();
			ASSERT(dfmsg->arrivedOn("netfilterIn"));  // FIXME revise this
			if (dfmsg->kind() == DISCARD_PACKET)
			{
				delete dfmsg;
				// releaseKernel();
				continue;
			}
			datagram = (IPDatagram *)dfmsg;
		}

		// wait for duration of processing delay
        wait(delay);

		// check for header biterror
		if (datagram->hasBitError())
		{
			/* 	probability of bit error in header =
				size of header / size of total message */
			relativeHeaderLength =
				datagram->headerLength() / datagram->totalLength();
			if (dblrand() <= relativeHeaderLength)
			{
				sendErrorMessage(datagram, ICMP_PARAMETER_PROBLEM, 0);
				continue;
			}
		}

		// hop counter decrement
		datagram->setTimeToLive (datagram->timeToLive()-1);

		send(datagram, "routingOut");
	}

}

// 	private function: send error message to ICMP Module
void PreRoutingCore::sendErrorMessage(IPDatagram *datagram,
		ICMPType type, ICMPCode code)
{

	// 	format of this message defined in ICMP.h
	cMessage *icmpNotification = new cMessage();

	datagram->setName("datagram");
	icmpNotification->addPar("ICMPType") = (long)type;
	icmpNotification->addPar("ICMPCode") = code;
	icmpNotification->parList().add(datagram);

	send(icmpNotification, "errorOut");
}


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
    file: IPSendCore.cc
    Purpose: Implementation for IPSendCore
    ------
	Responsibilities:
	receive IPInterfacePacket from Transport layer or ICMP
	or Tunneling (IP tunneled datagram)
	encapsulate in IP datagram
	set version
	set ds.codepoint
	set TTL
	choose and set fragmentation identifier
	set fragment offset = 0, more fragments = 0
	(fragmentation occurs in IPFragmentation)
	set 'don't Fragment'-bit to received value (default 0)
	set Protocol to received value
	set destination address to received value
	send datagram to Routing

    if IPInterfacePacket is invalid (e.g. invalid source address),
     it is thrown away without feedback

	author: Jochen Reber
	------------------------------------------------- */

#include <omnetpp.h>
#include <stdlib.h>
#include <string.h>

#include "hook_types.h"
#include "IPSendCore.h"
#include "IPDatagram.h"

Define_Module( IPSendCore );

/*  ----------------------------------------------------------
        Public Functions
    ----------------------------------------------------------  */

void IPSendCore::initialize()
{
	RoutingTableAccess::initialize();

	delay = par("procdelay");
	defaultTimeToLive = par("timeToLive");
	defaultMCTimeToLive = par("multicastTimeToLive");
	curFragmentId = 0;
    hasHook = (findGate("netfilterOut") != -1);
}

void IPSendCore::activity()
{
    IPInterfacePacket *interfaceMsg;

    while(true)
    {
        interfaceMsg = (IPInterfacePacket *)receive();

		// claimKernel();
		sendDatagram(interfaceMsg);
	}

}

/*  ----------------------------------------------------------
        Private Functions
    ----------------------------------------------------------  */

void IPSendCore::sendDatagram(IPInterfacePacket *interfaceMsg)
{

	cPacket *transportPacket = NULL;
    IPDatagram *datagram = new IPDatagram();
	cMessage *dfmsg = NULL;
	IPAddrChar dest;
	IPAddrChar src;

	transportPacket = (cPacket *)interfaceMsg->decapsulate();
	datagram->encapsulate(transportPacket);


	// set source and destination address
	strcpy( dest, interfaceMsg->destAddr());
	datagram->setDestAddress(dest);

	// if no interface exists, do not send datagram
	if (rt->interfaceNo() == 0)
	{
		return;
	}

	strcpy( src, interfaceMsg->srcAddr());
	// when source address given in Interface Message, use it
	if (strcmp(src, ""))
	{
		/* if interface parameter does not match existing
			interface, do not send datagram */
		if (rt->interfaceAddressToNo(src) == -1)
		{
			ev << "***IPSend Error of "
//				<< rt->getInterfaceByIndex(0).inetAddrStr
				<< rt->getInterfaceByIndex(0)->inetAddr->getString()
				<< ": Wrong sender address: "
				<< src << "\n";
			breakpoint("IPSend error!");
			return;
		}
		datagram->setSrcAddress(src);
	} else { // otherwise, just use the first
//		datagram->setSrcAddress(rt->getInterfaceByIndex(0).inetAddrStr);
		datagram->setSrcAddress(rt->getInterfaceByIndex(0)->inetAddr->getString());
	}

	// set other fields
	datagram->setCodepoint (interfaceMsg->codepoint());

	datagram->setFragmentId(curFragmentId++);
	datagram->setMoreFragments(false);
	datagram->setDontFragment (interfaceMsg->dontFragment());
	datagram->setFragmentOffset (0);

	datagram->setTimeToLive
		(interfaceMsg->timeToLive() > 0 ?
			interfaceMsg->timeToLive() :
				(rt->isMulticastAddr(datagram->destAddress())
						? defaultMCTimeToLive : defaultTimeToLive));

	datagram->setTransportProtocol ((IPProtocolFieldId)interfaceMsg->protocol());
	datagram->setInputPort(-1);

	// setting an option is currently not possible


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
			return;
		}
		datagram = (IPDatagram *)dfmsg;
	}

	// send new datagram
	wait(delay);
	send(datagram, "routingOut");
        delete interfaceMsg;
}


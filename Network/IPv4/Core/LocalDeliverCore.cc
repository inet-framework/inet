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

/* -------------------------------------------------
   file: LocalDeliverCore.cc
   Implementation for the Simple Module LocalDeliverCore
   ------
   Responsibilities: 
		Receive IP datagram for local delivery
		strip off IP header
		buffer fragments for ip_fragmenttime
		wait until all fragments of one fragment number are received
		discard without notification if not all fragments arrive in 
		ip_fragmenttime
		Defragment once all fragments have arrived
		send Transport packet up to the transport layer
		send ICMP packet to ICMP module
		send IGMP group management packet to Multicast module
		send tunneled IP datagram to PreRouting
   Notation: 
		TCP-Packets --> transportOut[0]
		UDP-Packets --> transportOut[1]
   Author:      Jochen Reber
   -------------------------------------------------
*/

#include <omnetpp.h>

#include "hook_types.h"
#include "LocalDeliverCore.h"
#include "ProcessorManager.h"

Define_Module( LocalDeliverCore );

/*  ----------------------------------------------------------
        Public Functions
    ----------------------------------------------------------  */

// initialisation: set fragmentTimeout
// tbd: include fragmentTimeout in .ned files
void LocalDeliverCore::initialize()
{
	ProcessorAccess::initialize();
	fragmentTimeoutTime = strToSimtime(par("fragmentTimeout"));
	delay = par("procdelay");
    hasHook = (findGate("netfilterOut") != -1);

	int i;
	for (i=0; i < FRAGMENT_BUFFER_MAXIMUM; i++)
	{
		fragmentBuf[i].isFree = true;
		fragmentBuf[i].fragmentId = -1;
	}

	fragmentBufSize = 0;
}

// main activity loop to receive messages
void LocalDeliverCore::activity()
{
	simtime_t lastCheckTime;
    cMessage *dfmsg;
	IPDatagram *datagram;
	IPInterfacePacket *interfacePacket;

	lastCheckTime = simTime();

	while(true)
  	{
		
		// erase timed out fragments in fragmentation buffer	
		// check every 1 second max
		if (simTime() >= lastCheckTime + 1)
		{
			lastCheckTime = simTime();
			eraseTimeoutFragmentsFromBuf();	
		}

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

		// Defragmentation
		// skip Degragmentation if single Fragment Datagram
		if (datagram->fragmentOffset() != 0 
			|| datagram->moreFragments())
		{
			insertInFragmentBuf( datagram );
			if (!datagramComplete(datagram->fragmentId()))
			{
				delete(datagram);
				releaseKernel();
				continue;
			} 

			datagram->setLength( datagram->headerLength()*8 + 
					datagram->encapsulatedMsg()->length() );
					//getPayloadSizeFromBuf( datagram->fragmentId() ) );

			/*
			ev << "\ndefragment\n";
			ev << "\nheader length: " << datagram->headerLength()*8
				<< "  encap length: " << datagram->encapsulatedMsg()->length()
				<< "  new length: " << datagram->length() << "\n"; 
			*/

			removeFragmentFromBuf(datagram->fragmentId());
		}
	
		interfacePacket = setInterfacePacket(datagram);
		delete(datagram);

		wait(delay);
		switch(interfacePacket->protocol())
		{
			case IP_PROT_ICMP:
				send(interfacePacket, "ICMPOut");
				break;
			case IP_PROT_IGMP:
				send(interfacePacket, "multicastOut");
				break;
			case IP_PROT_IP:
				send(interfacePacket, "preRoutingOut");
				releaseKernel();
				break;
			case IP_PROT_TCP:
				send(interfacePacket, "transportOut",0);
				releaseKernel();
				break;
			case IP_PROT_UDP: 	
				send(interfacePacket, "transportOut",1);
				releaseKernel();
				break;
// BCH Andras: from UTS MPLS model
			case IP_PROT_RSVP:
				ev << "IP send packet to RSVPInterface\n";
				send(interfacePacket, "transportOut",3);
				releaseKernel();
				break;
// ECH
			default:
				ev << "LocalDeliver Error: "
					<< "Transport protocol invalid: "
					<< (int)(interfacePacket->protocol())
					<< "\n";
				delete(interfacePacket);
				releaseKernel();
				break;
		} // end switch
	} // end while
}

/*  ----------------------------------------------------------
		Private functions
    ----------------------------------------------------------  */

IPInterfacePacket *LocalDeliverCore::setInterfacePacket
			(IPDatagram *datagram)
{
	cPacket *packet;
	IPInterfacePacket *interfacePacket = new IPInterfacePacket;

	packet = datagram->decapsulate();

	interfacePacket->encapsulate(packet);
	interfacePacket->setProtocol(datagram->transportProtocol());
	interfacePacket->setSrcAddr(datagram->srcAddress());
	interfacePacket->setDestAddr(datagram->destAddress());
	interfacePacket->setCodepoint(datagram->codepoint());

	return interfacePacket;
}

/*  ----------------------------------------------------------
		Private functions: Fragmentation Buffer management
    ----------------------------------------------------------  */

// erase those fragments from the buffer that have timed out
void LocalDeliverCore::eraseTimeoutFragmentsFromBuf()
{
	int i;
	simtime_t curTime = simTime();

	for (i=0; i < fragmentBufSize; i++)
	{
		if (!fragmentBuf[i].isFree &&
			curTime > fragmentBuf[i].timeout)
		{
			/* debugging output
			ev << "++++ fragment kicked out: "
				<< i << " :: "
				<< fragmentBuf[i].fragmentId << " / "
				<< fragmentBuf[i].fragmentOffset << " : "
				<< fragmentBuf[i].timeout << "\n";
			*/
			fragmentBuf[i].isFree = true;
		} // end if
	} // end for
}

void LocalDeliverCore::insertInFragmentBuf(IPDatagram *d)
{
	int i;
	FragmentationBufferEntry *e;

	for (i=0; i < fragmentBufSize; i++)
	{
		if (fragmentBuf[i].isFree == true)
		{
			break;
		} 
	} // end for

	// if no free place found, increase Buffersize to append entry
	if (i == fragmentBufSize)
		fragmentBufSize++;

	e = &fragmentBuf[i];
	e->isFree = false;
	e->fragmentId = d->fragmentId();
	e->fragmentOffset = d->fragmentOffset();
	e->moreFragments = d->moreFragments();
	e->length = d->totalLength() - d->headerLength();
	e->timeout= simTime() + fragmentTimeoutTime;

}

bool LocalDeliverCore::datagramComplete(int fragmentId)
{
	bool isComplete = false;
	int nextFragmentOffset = 0; // unit: 8 bytes
	bool newFragmentFound = true;
	int i;

	while(newFragmentFound)
	{
		newFragmentFound = false;
		for (i=0; i < fragmentBufSize; i++)
		{
			if (!fragmentBuf[i].isFree &&
					fragmentId == fragmentBuf[i].fragmentId &&
					nextFragmentOffset == fragmentBuf[i].fragmentOffset)
			{
				newFragmentFound = true;	
				nextFragmentOffset += fragmentBuf[i].length;
				// Datagram complete if last Fragment found
				if (!fragmentBuf[i].moreFragments)
				{
					return true;
				}
				// reset i to beginning of buffer
			} // end if
		} // end for
	} // end while

	// when no new Fragment found, Datagram is not complete
	return false;
}

int LocalDeliverCore::getPayloadSizeFromBuf(int fragmentId)
{
	int i;
	int payload = 0;

	for (i=0; i < fragmentBufSize; i++)
	{
			if (!fragmentBuf[i].isFree &&
			fragmentBuf[i].fragmentId == fragmentId)
		{
			payload += fragmentBuf[i].length;
		} // end if
	} // end for
	return payload;
}

void LocalDeliverCore::removeFragmentFromBuf(int fragmentId)
{
	int i;

	for (i=0; i < fragmentBufSize; i++)
	{
		if (!fragmentBuf[i].isFree &&
			fragmentBuf[i].fragmentId == fragmentId)
		{
			fragmentBuf[i].fragmentId = -1;
			fragmentBuf[i].isFree = true;
		} // end if
	} // end for
}


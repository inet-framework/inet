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
    file: IPMulticast.cc
    Purpose: Implementation for IP Multicast

    Responsibilities:
    receive datagram with Multicast address from Routing
    duplicate datagram if it is sent to more than one output port
		and IPForward is true
    map multicast address on output port, use multicast routing table
    Outsend copy to  local deliver, if
    NetworkCardAddr.[] is part of multicast address
    if entry in multicast routing table requires tunneling,
    send to Tunneling module
    otherwise send to Fragmentation module

    receive IGMP message from LocalDeliver
    update multicast routing table

    comment:
    Tunneling and IGMP not implemented
    author: Jochen Reber
*/

#include <stdlib.h>
#include <omnetpp.h>
#include <string.h>

#include "IPMulticast.h"

Define_Module( IPMulticast );

/*  ----------------------------------------------------------
        Public Functions
    ----------------------------------------------------------  */

void IPMulticast::initialize()
{
	RoutingTableAccess::initialize();
	IPForward = par("IPForward").boolValue();
	delay = par("procdelay");
}

void IPMulticast::activity()
{
	cMessage *msg;
	IPDatagram *datagram = NULL;
	// copy of original datagram for multiple destinations
	IPDatagram *datagramCopy = NULL;
	IPAddrChar destAddress;
	int i;
	int destNo; // number of destinations for datagram

	while(true)
	{
        msg = receive();

		wait(delay);

		// check if IGMP message from localIn
		if (!strcmp(msg->arrivalGate()->name(), "localIn"))
		{
			// IGMP not implemented!
			delete msg;
			// releaseKernel();

		} else { // otherwise forward datagram with Multicast address

			datagram = (IPDatagram *)msg;
			strcpy(destAddress, datagram->destAddress());

			/* debugging output
			ev << "### incoming mc paket at "
				<< rt->getInterfaceByIndex(0).inetAddrStr << ": "
				<< destAddress << "\n";
			*/

            /* DVMRP: process datagram only if sent
				locally or arrived on the shortest route
				otherwise, discard and continue. */
            if ( datagram->inputPort() != -1 &&
				 datagram->inputPort() !=
				 rt->outputPortNo(datagram->srcAddress()))
            {

				/* debugging output
				ev  << "* outputPort (shortest path): "
				<< rt->outputPortNo(datagram->srcAddress())
				<< "   inputPort: " << datagram->inputPort() << "\n";
				*/

                delete(datagram);
				// releaseKernel();
                continue;
            } // *endif input port check*


			// check for local delivery
			if (rt->multicastLocalDeliver(destAddress))
			{
				datagramCopy = (IPDatagram *) datagram->dup();

// BCH Andras -- code from UTS MPLS model
                // find "local_addr" module parameter among our parents, and assign it to packet
                cModule *curmod = this;
                for (curmod = parentModule(); curmod != NULL;
                            curmod = curmod->parentModule())
                {
                    if(curmod->hasPar("local_addr"))
                    {
                    //IPAddress *myAddr =new IPAddress((curmod->par("local_addr").stringValue()));
                    datagramCopy->setDestAddress(curmod->par("local_addr"));
                    break;
                    }
                }
// ECH
				// claimKernelAgain();
				send(datagramCopy, "localOut");
			} // *endif local deliver*


            /* forward datagram only if IPForward is true
				or sent locally */
            if ( datagram->inputPort() != -1 && IPForward == false )
            {
                delete(datagram);
				// releaseKernel();
                continue;
            } // *endif IPForward*


			destNo = rt->multicastDestinations(destAddress);
			if (destNo > 0)
			{
				int outputPort = -1,
					inputPort = -1;

				for (i=0; i < destNo; i++)
				{
					outputPort = rt->mcOutputPortNo(destAddress, i);
					inputPort = datagram->inputPort();

					// don't forward to input port
					if (outputPort >= 0 && outputPort != inputPort)
					{
						datagramCopy = (IPDatagram *) datagram->dup();
						datagramCopy->setOutputPort(outputPort);
						// claimKernelAgain();
						send(datagramCopy, "fragmentationOut");
					} // *endif*
				} // *endfor*
				// only copies sent, original datagram deleted
				delete(datagram);
				// each datagram sent out claimed the kernel again once,
				// releaseKernel();

			} else // no destination: delete datagram
			{
				delete(datagram);
				// releaseKernel();
				continue;
			} // *endif multicast destinations*

		} // *endelse localIn*

	} // *endwhile*
}


//
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


#include <stdlib.h>
#include <omnetpp.h>
#include <string.h>

#include "IPMulticast.h"

Define_Module(IPMulticast);



void IPMulticast::initialize()
{
    QueueBase::initialize();
    IPForward = par("IPForward").boolValue();
}

void IPMulticast::endService(cMessage *msg)
{
    RoutingTable *rt = routingTableAccess.get();

    // FIXME We should probably handle if IGMP message comes from localIn.
    // IGMP is not implemented.
    IPDatagram *datagram = check_and_cast<IPDatagram *>(msg);

    IPAddress destAddress = datagram->destAddress();

    // DVMRP: process datagram only if sent locally or arrived on
    // the shortest route; otherwise discard and continue.
    if (datagram->inputPort()!=-1 && datagram->inputPort()!=rt->outputPortNo(datagram->srcAddress()))
    {
        /* debugging output
        ev  << "* outputPort (shortest path): "
        << rt->outputPortNo(datagram->srcAddress())
        << "   inputPort: " << datagram->inputPort() << "\n";
        */

        delete datagram;
        return;
    }

    // check for local delivery
    if (rt->multicastLocalDeliver(destAddress))
    {
        IPDatagram *datagramCopy = (IPDatagram *) datagram->dup();

// BCH Andras -- code from UTS MPLS model  FIXME!!!!!!!!!!!!!!!!!!!!!!!!
        // find "local_addr" module parameter among our parents, and assign it to packet
        cModule *curmod = this;
        for (curmod = parentModule(); curmod != NULL;
                    curmod = curmod->parentModule())
        {
            if (curmod->hasPar("local_addr"))
            {
                datagramCopy->setDestAddress(IPAddress(curmod->par("local_addr").stringValue()));
                break;
            }
        }
// ECH
        send(datagramCopy, "localOut");
    }

    // forward datagram only if IPForward is true or sent locally
    if (datagram->inputPort() != -1 && !IPForward)
    {
        delete datagram;
        return;
    }

    int destNo; // number of destinations for datagram
    destNo = rt->numMulticastDestinations(destAddress);

    if (destNo == 0)
    {
        // no destination: delete datagram
        delete datagram;
    }
    else
    {
        // copy original datagram for multiple destinations
        for (int i=0; i < destNo; i++)
        {
            int outputPort = rt->multicastOutputPortNo(destAddress, i);
            int inputPort = datagram->inputPort();

            // don't forward to input port
            if (outputPort >= 0 && outputPort != inputPort)
            {
                IPDatagram *datagramCopy = (IPDatagram *) datagram->dup();
                datagramCopy->setOutputPort(outputPort);
                send(datagramCopy, "fragmentationOut");
            }
        }

        // only copies sent, delete original datagram
        delete datagram;
    }
}


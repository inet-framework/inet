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
//


#include <stdlib.h>
#include <omnetpp.h>
#include "Routing.h"


Define_Module(Routing);


void Routing::initialize()
{
    QueueWithQoS::initialize();
    IPForward = par("IPForward").boolValue();
}

void Routing::endService(cMessage *msg)
{
    // FIXME may we get ICMP here? what to do with it then?
    IPDatagram *datagram = check_and_cast<IPDatagram *>(msg);

    // FIXME add option handling code here!

    IPAddress destAddress = datagram->destAddress();

    ev << "Packet destination address is: " << destAddress << "\n";

    //  multicast check
    RoutingTable *rt = routingTableAccess.get();
    if (destAddress.isMulticast())
    {
        send(datagram, "multicastOut");
        return;
    }

    // check for local delivery
    if (rt->localDeliver(destAddress))
    {
        send(datagram, "localOut");
        return;
    }

    // if datagram arrived from input gate and IP_FORWARD is off, delete datagram
    if (datagram->inputPort()!=-1 && !IPForward)
    {
        delete datagram;
        return;
    }

    // error handling: destination address does not exist in routing table:
    // notify ICMP, throw packet away and continue
    int outputPort = rt->outputPortNo(destAddress);
    if (outputPort==-1)
    {
        icmpAccess.get()->sendErrorMessage(datagram, ICMP_DESTINATION_UNREACHABLE, 0);
        return;
    }

    // default: send datagram to fragmentation
    datagram->setOutputPort(outputPort);
    send(datagram, "fragmentationOut");
}




//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
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


//  Cleanup and rewrite: Andras Varga, 2004

#include <stdlib.h>
#include <omnetpp.h>
#include "IPControlInfo_m.h"
#include "IPRouting.h"
#include "InterfaceTableAccess.h"
#include "IPv4InterfaceData.h"
#include "RoutingTableAccess.h"


Define_Module(IPRouting);


void IPRouting::initialize()
{
    QueueWithQoS::initialize();

    ift = InterfaceTableAccess().get();
    rt = RoutingTableAccess().get();

    numMulticast = numLocalDeliver = numDropped = numUnroutable = numForwarded = 0;

    WATCH(numMulticast);
    WATCH(numLocalDeliver);
    WATCH(numDropped);
    WATCH(numUnroutable);
    WATCH(numForwarded);
}

void IPRouting::endService(cMessage *msg)
{
    // FIXME may we get ICMP here? what to do with it then?
    IPDatagram *datagram = check_and_cast<IPDatagram *>(msg);
    IPRoutingDecision *routingDecision = check_and_cast<IPRoutingDecision *>(datagram->controlInfo());

    // TBD add option handling code here!

    IPAddress destAddress = datagram->destAddress();

    ev << "Routing datagram `" << datagram->name() << "' with dest=" << destAddress << ": ";

    //  multicast check
    if (destAddress.isMulticast())
    {
        ev << "sending to multicast\n";
        numMulticast++;
        send(datagram, "multicastOut");
        return;
    }

    // check for local delivery
    if (rt->localDeliver(destAddress))
    {
        ev << "local delivery\n";
        numLocalDeliver++;
        send(datagram, "localOut");
        return;
    }

    // if datagram arrived from input gate and IP forwarding is off, delete datagram
    if (routingDecision->inputPort()!=-1 && !rt->ipForward())
    {
        ev << "forwarding off, dropping packet\n";
        numDropped++;
        delete datagram;
        return;
    }

    // error handling: destination address does not exist in routing table:
    // notify ICMP, throw packet away and continue
    int outputPort = rt->outputPortNo(destAddress);
    if (outputPort==-1)
    {
        ev << "unroutable, sending ICMP_DESTINATION_UNREACHABLE\n";
        numUnroutable++;
        icmpAccess.get()->sendErrorMessage(datagram, ICMP_DESTINATION_UNREACHABLE, 0);
        return;
    }

    // get next-hop address
    IPAddress nextHopAddr = rt->nextGatewayAddress(destAddress);

    // set datagram source address if not yet set
    if (datagram->srcAddress().isUnspecified())
        datagram->setSrcAddress(ift->interfaceByPortNo(outputPort)->ipv4()->inetAddress());

    // default: send datagram to fragmentation
    routingDecision->setOutputPort(outputPort);
    routingDecision->setNextHopAddr(nextHopAddr);

    ev << "output interface is " << outputPort << "\n";
    numForwarded++;
    send(datagram, "fragmentationOut");
}




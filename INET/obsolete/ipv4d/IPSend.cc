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

#include <omnetpp.h>
#include <stdlib.h>
#include <string.h>

#include "IPSend.h"
#include "IPDatagram.h"
#include "IPControlInfo_m.h"
#include "InterfaceTableAccess.h"
#include "IPv4InterfaceData.h"
#include "RoutingTableAccess.h"


Define_Module(IPSend);


void IPSend::initialize()
{
    ift = InterfaceTableAccess().get();
    rt = RoutingTableAccess().get();

    defaultTimeToLive = par("timeToLive");
    defaultMCTimeToLive = par("multicastTimeToLive");
    curFragmentId = 0;
}

void IPSend::handleMessage(cMessage *msg)
{
    IPDatagram *dgram = encapsulate(msg);
    send(dgram, "routingOut");
}

IPDatagram *IPSend::encapsulate(cMessage *transportPacket)
{
    // if no interface exists, do not send datagram
    if (ift->numInterfaces() == 0)
    {
        ev << "No interfaces exist, dropping packet\n";
        delete transportPacket;
        return NULL;
    }

    IPControlInfo *controlInfo = check_and_cast<IPControlInfo*>(transportPacket->removeControlInfo());

    IPDatagram *datagram = new IPDatagram(transportPacket->name());
    datagram->setLength(8*IP_HEADER_BYTES);
    datagram->encapsulate(transportPacket);

    // set source and destination address
    IPAddress dest = controlInfo->destAddr();
    datagram->setDestAddress(dest);

    IPAddress src = controlInfo->srcAddr();

    // when source address given in Interface Message, use it
    // (otherwise it'll get the address of the outgoing interface after routing)
    if (!src.isUnspecified())
    {
        // if interface parameter does not match existing interface, do not send datagram
        if (rt->interfaceByAddress(src)==NULL)
            opp_error("Wrong source address %s in (%s)%s: no interface with such address",
                      src.str().c_str(), transportPacket->className(), transportPacket->fullName());
        datagram->setSrcAddress(src);
    }

    // set other fields
    datagram->setDiffServCodePoint(controlInfo->diffServCodePoint());

    datagram->setIdentification(curFragmentId++);
    datagram->setMoreFragments(false);
    datagram->setDontFragment (controlInfo->dontFragment());
    datagram->setFragmentOffset(0);

    datagram->setTimeToLive(
           controlInfo->timeToLive() > 0 ?
           controlInfo->timeToLive() :
           (datagram->destAddress().isMulticast() ? defaultMCTimeToLive : defaultTimeToLive)
    );

    datagram->setTransportProtocol(controlInfo->protocol());
    delete controlInfo;

    // add blank RoutingDecision info
    IPRoutingDecision *routingDecision = new IPRoutingDecision();
    datagram->setControlInfo(routingDecision);

    // setting IP options is currently not supported

    return datagram;
}


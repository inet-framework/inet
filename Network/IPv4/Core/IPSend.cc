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



#include <omnetpp.h>
#include <stdlib.h>
#include <string.h>

#include "IPSend.h"
#include "IPDatagram.h"

Define_Module(IPSend);


void IPSend::initialize()
{
    QueueWithQoS::initialize();

    defaultTimeToLive = par("timeToLive");
    defaultMCTimeToLive = par("multicastTimeToLive");
    curFragmentId = 0;
}

void IPSend::arrival(cMessage *msg)
{
    IPDatagram *dgram = encapsulate(msg);
    if (!dgram) return;
    qosHook->enqueue(dgram, queue);
}

cMessage *IPSend::arrivalWhenIdle(cMessage *msg)
{
    IPDatagram *dgram = encapsulate(msg);
    if (!dgram) return NULL;
    return qosHook->dropIfNotNeeded(dgram);
}

void IPSend::endService(cMessage *msg)
{
    send(msg, "routingOut");
}

IPDatagram *IPSend::encapsulate(cMessage *msg)
{
    // if no interface exists, do not send datagram
    RoutingTable *rt = routingTableAccess.get();
    if (rt->numInterfaces() == 0)
    {
        delete msg;
        return NULL;
    }

    IPInterfacePacket *interfaceMsg = check_and_cast<IPInterfacePacket *>(msg);

    cPacket *transportPacket = (cPacket *) interfaceMsg->decapsulate();

    IPDatagram *datagram = new IPDatagram();
    datagram->encapsulate(transportPacket);

    // set source and destination address
    const IPAddress& dest = interfaceMsg->destAddr();
    datagram->setDestAddress(dest);

    const IPAddress& src = interfaceMsg->srcAddr();

    // when source address given in Interface Message, use it
    if (!src.isNull())
    {
        // if interface parameter does not match existing interface, do not send datagram
        if (rt->findInterfaceByAddress(src) == -1)
            opp_error("Wrong source address %s in (%s)%s: no interface with such address",
                      src.str().c_str(), interfaceMsg->className(), interfaceMsg->fullName());
        datagram->setSrcAddress(src);
    }
    else
    {
        // otherwise, just use the first
        datagram->setSrcAddress(rt->getInterfaceByIndex(0)->inetAddr);
    }

    // set other fields
    datagram->setDiffServCodePoint(interfaceMsg->diffServCodePoint());

    datagram->setFragmentId(curFragmentId++);
    datagram->setMoreFragments(false);
    datagram->setDontFragment (interfaceMsg->dontFragment());
    datagram->setFragmentOffset(0);

    datagram->setTimeToLive(
           interfaceMsg->timeToLive() > 0 ?
           interfaceMsg->timeToLive() :
           (datagram->destAddress().isMulticast() ? defaultMCTimeToLive : defaultTimeToLive)
    );

    datagram->setTransportProtocol ((IPProtocolFieldId)interfaceMsg->protocol());
    datagram->setInputPort(-1);

    // setting an option is currently not possible
    delete interfaceMsg;

    return datagram;
}


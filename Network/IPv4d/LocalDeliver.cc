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


#include <omnetpp.h>
#include "LocalDeliver.h"
#include "IPControlInfo_m.h"


//  Cleanup and rewrite: Andras Varga, 2004

Define_Module(LocalDeliver);


void LocalDeliver::initialize()
{
    fragmentTimeoutTime = par("fragmentTimeout");
    lastCheckTime = 0;
}


void LocalDeliver::handleMessage(cMessage *msg)
{
    IPDatagram *datagram = check_and_cast<IPDatagram *>(msg);

    // erase timed out fragments in fragmentation buffer; check every 10 seconds max
    if (simTime() >= lastCheckTime + 10)
    {
        lastCheckTime = simTime();
        fragbuf.purgeStaleFragments(simTime()-fragmentTimeoutTime);
    }

    // Defragmentation. skip defragmentation if datagram is not fragmented
    if (datagram->fragmentOffset()!=0 || datagram->moreFragments())
    {
        datagram = fragbuf.addFragment(datagram, simTime());
        if (!datagram)
            return;
    }

    int protocol = datagram->transportProtocol();
    cMessage *packet = decapsulateIP(datagram);

    switch (protocol)
    {
        case IP_PROT_ICMP:
            send(packet, "ICMPOut");
            break;
        case IP_PROT_IGMP:
            send(packet, "multicastOut");
            break;
        case IP_PROT_IP:
            send(packet, "preRoutingOut");
            break;
        case IP_PROT_TCP:
            send(packet, "transportOut",0);
            break;
        case IP_PROT_UDP:
            send(packet, "transportOut",1);
            break;
        case IP_PROT_RSVP:
            ev << "IP send packet to RSVPInterface\n";
            send(packet, "transportOut",3);
            break;
        default:
            error("Unknown transport protocol number %d", protocol);
    }
}


cMessage *LocalDeliver::decapsulateIP(IPDatagram *datagram)
{
    cMessage *packet = datagram->decapsulate();

    IPControlInfo *controlInfo = new IPControlInfo();
    controlInfo->setProtocol(datagram->transportProtocol());
    controlInfo->setSrcAddr(datagram->srcAddress());
    controlInfo->setDestAddr(datagram->destAddress());
    controlInfo->setDiffServCodePoint(datagram->diffServCodePoint());
    packet->setControlInfo(controlInfo);
    delete datagram;

    return packet;
}


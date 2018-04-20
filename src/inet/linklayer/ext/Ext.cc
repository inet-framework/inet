//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

// This file is based on the Ppp.cc of INET written by Andras Varga.

#define WANT_WINSOCK2

#include <stdio.h>
#include <string.h>

#include <platdep/sockets.h>

#include "inet/common/INETDefs.h"
#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/BytesChunk.h"

#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ext/Ext.h"

#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"

#include <arpa/inet.h>

namespace inet {

Define_Module(Ext);

void Ext::initialize(int stage)
{
    MacBase::initialize(stage);

    // subscribe at scheduler for external messages
    if (stage == INITSTAGE_LOCAL) {
        if (auto scheduler = dynamic_cast<cSocketRTScheduler *>(getSimulation()->getScheduler())) {
            rtScheduler = scheduler;
            device = par("device");
            const char *filter = par("filterString");
            rtScheduler->setInterfaceModule(this, device, filter);
            connected = true;
        }
        else {
            // this simulation run works without external interface
            connected = false;
        }
        numSent = numRcvd = numDropped = 0;
        WATCH(numSent);
        WATCH(numRcvd);
        WATCH(numDropped);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        registerInterface();
    }
}

InterfaceEntry *Ext::createInterfaceEntry()
{
    InterfaceEntry *e = getContainingNicModule(this);

    e->setMtu(par("mtu"));
    e->setMulticast(true);
    e->setPointToPoint(true);

    return e;
}

void Ext::handleMessage(cMessage *msg)
{
    using namespace serializer;
    if (!isOperational) {
        handleMessageWhenDown(msg);
        return;
    }

    Packet *packet = check_and_cast<Packet *>(msg);

    if (msg->isSelfMessage()) {
        // incoming real packet from wire (captured by pcap)
        const auto& nwHeader = packet->peekAtFront<Ipv4Header>();
        EV << "Delivering a packet from "
           << nwHeader->getSourceAddress()
           << " to "
           << nwHeader->getDestinationAddress()
           << " and length of"
           << packet->getByteLength()
           << " bytes to networklayer.\n";
        packet->addTagIfAbsent<InterfaceInd>()->setInterfaceId(interfaceEntry->getInterfaceId());
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ipv4);
        send(packet, "upperLayerOut");
        numRcvd++;
    }
    else {
        auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
        if (protocol != &Protocol::ipv4)
            throw cRuntimeError("ExtInterface accepts ipv4 packets only");

        const auto& ipv4Header = packet->peekAtFront<Ipv4Header>();

        if (connected) {
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
#if !defined(linux) && !defined(__linux) && !defined(_WIN32)
            addr.sin_len = sizeof(struct sockaddr_in);
#endif // if !defined(linux) && !defined(__linux) && !defined(_WIN32)
            addr.sin_port = htons(0);
            addr.sin_addr.s_addr = htonl(ipv4Header->getDestAddress().getInt());
            auto bytesChunk = packet->peekAllAsBytes();
            size_t packetLength = bytesChunk->copyToBuffer(buffer, sizeof(buffer));
            ASSERT(packetLength == packet->getByteLength());

            EV << "Delivering an IPv4 packet from "
               << ipv4Header->getSrcAddress()
               << " to "
               << ipv4Header->getDestAddress()
               << " and length of "
               << packet->getByteLength()
               << " bytes to link layer.\n";
            rtScheduler->sendBytes(this, buffer, packetLength, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
            numSent++;
            delete packet;
        }
        else {
            EV << "Interface is not connected, dropping packet " << msg << endl;
            numDropped++;
        }
    }
}

void Ext::displayBusy()
{
    getDisplayString().setTagArg("i", 1, "yellow");
    gate("physOut")->getDisplayString().setTagArg("ls", 0, "yellow");
    gate("physOut")->getDisplayString().setTagArg("ls", 1, "3");
}

void Ext::displayIdle()
{
    getDisplayString().setTagArg("i", 1, "");
    gate("physOut")->getDisplayString().setTagArg("ls", 0, "black");
    gate("physOut")->getDisplayString().setTagArg("ls", 1, "1");
}

void Ext::refreshDisplay() const
{
    if (connected) {
        char buf[80];
        sprintf(buf, "pcap device: %s\nrcv:%d snt:%d", device, numRcvd, numSent);
        getDisplayString().setTagArg("t", 0, buf);
    }
    else {
        getDisplayString().setTagArg("i", 1, "#707070");
        getDisplayString().setTagArg("i", 2, "100");
        getDisplayString().setTagArg("t", 0, "not connected");
    }
}

void Ext::finish()
{
    std::cout << getFullPath() << ": " << numSent << " packets sent, "
              << numRcvd << " packets received, " << numDropped << " packets dropped.\n";
}

void Ext::flushQueue()
{
    // does not have a queue, do nothing
}

void Ext::clearQueue()
{
    // does not have a queue, do nothing
}

} // namespace inet


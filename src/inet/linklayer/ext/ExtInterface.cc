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

// This file is based on the PPP.cc of INET written by Andras Varga.

#define WANT_WINSOCK2

#include <stdio.h>
#include <string.h>

#include <platdep/sockets.h>

#include "inet/common/INETDefs.h"

#include "inet/linklayer/ext/ExtInterface.h"

#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/common/serializer/ipv4/IPv4Serializer.h"
#include "inet/common/INETUtils.h"
#include "inet/networklayer/common/IPProtocolId_m.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"

namespace inet {

Define_Module(ExtInterface);

void ExtInterface::initialize(int stage)
{
    MACBase::initialize(stage);

    // subscribe at scheduler for external messages
    if (stage == INITSTAGE_LOCAL) {
        if (dynamic_cast<cSocketRTScheduler *>(getSimulation()->getScheduler()) != nullptr) {
            rtScheduler = check_and_cast<cSocketRTScheduler *>(getSimulation()->getScheduler());
            //device = getEnvir()->config()->getAsString("Capture", "device", "lo0");
            device = par("device");
            //const char *filter = getEnvir()->config()->getAsString("Capture", "filter-string", "ip");
            const char *filter = par("filterString");
            rtScheduler->setInterfaceModule(this, device, filter);
            connected = true;
        }
        else {
            // this simulation run works without external interface..
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
    else if (stage == INITSTAGE_LAST) {
        // if not connected, make it gray
        if (hasGUI() && !connected) {
            getDisplayString().setTagArg("i", 1, "#707070");
            getDisplayString().setTagArg("i", 2, "100");
        }

        // update display string when addresses have been autoconfigured etc.
        if (hasGUI())
            updateDisplayString();
    }
}

InterfaceEntry *ExtInterface::createInterfaceEntry()
{
    InterfaceEntry *e = new InterfaceEntry(this);

    e->setMtu(par("mtu"));
    e->setMulticast(true);
    e->setPointToPoint(true);

    return e;
}

void ExtInterface::handleMessage(cMessage *msg)
{
    using namespace serializer;

    if (!isOperational) {
        handleMessageWhenDown(msg);
        return;
    }

    if (dynamic_cast<ExtFrame *>(msg) != nullptr) {
        // incoming real packet from wire (captured by pcap)
        uint32 packetLength;
        ExtFrame *rawPacket = check_and_cast<ExtFrame *>(msg);

        packetLength = rawPacket->getDataArraySize();
        for (uint32 i = 0; i < packetLength; i++)
            buffer[i] = rawPacket->getData(i);

        Buffer b(const_cast<unsigned char *>(buffer), packetLength);
        Context c;
        IPv4Datagram *ipPacket = check_and_cast<IPv4Datagram *>(IPv4Serializer().deserializePacket(b, c));
        EV << "Delivering an IPv4 packet from "
           << ipPacket->getSrcAddress()
           << " to "
           << ipPacket->getDestAddress()
           << " and length of"
           << ipPacket->getByteLength()
           << " bytes to IPv4 layer.\n";
        send(ipPacket, "upperLayerOut");
        numRcvd++;
    }
    else {
        memset(buffer, 0, sizeof(buffer));
        IPv4Datagram *ipPacket = check_and_cast<IPv4Datagram *>(msg);

        if (connected) {
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
#if !defined(linux) && !defined(__linux) && !defined(_WIN32)
            addr.sin_len = sizeof(struct sockaddr_in);
#endif // if !defined(linux) && !defined(__linux) && !defined(_WIN32)
            addr.sin_port = 0;
            addr.sin_addr.s_addr = htonl(ipPacket->getDestAddress().getInt());
            Buffer b(const_cast<unsigned char *>(buffer), sizeof(buffer));
            Context c;
            c.throwOnSerializerNotFound = false;
            IPv4Serializer().serializePacket(ipPacket, b, c);
            if (b.hasError() || c.errorOccured) {
                EV_ERROR << "Cannot serialize and send packet << '" << ipPacket->getName() << "' with protocol " << ipPacket->getTransportProtocol() << ".\n";
                numDropped++;
                delete (msg);
                return;
            }
            int32 packetLength = b.getPos();
            EV << "Delivering an IPv4 packet from "
               << ipPacket->getSrcAddress()
               << " to "
               << ipPacket->getDestAddress()
               << " and length of "
               << ipPacket->getByteLength()
               << " bytes to link layer.\n";
            rtScheduler->sendBytes(buffer, packetLength, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
            numSent++;
        }
        else {
            EV << "Interface is not connected, dropping packet " << msg << endl;
            numDropped++;
        }
    }
    delete (msg);
    if (hasGUI())
        updateDisplayString();
}

void ExtInterface::displayBusy()
{
    getDisplayString().setTagArg("i", 1, "yellow");
    gate("physOut")->getDisplayString().setTagArg("ls", 0, "yellow");
    gate("physOut")->getDisplayString().setTagArg("ls", 1, "3");
}

void ExtInterface::displayIdle()
{
    getDisplayString().setTagArg("i", 1, "");
    gate("physOut")->getDisplayString().setTagArg("ls", 0, "black");
    gate("physOut")->getDisplayString().setTagArg("ls", 1, "1");
}

void ExtInterface::updateDisplayString()
{
    if (!hasGUI())
        return;

    const char *str;
    char buf[80];

    if (connected) {
        sprintf(buf, "pcap device: %s\nrcv:%d snt:%d", device, numRcvd, numSent);
        str = buf;
    }
    else
        str = "not connected";
    getDisplayString().setTagArg("t", 0, str);
}

void ExtInterface::finish()
{
    std::cout << getFullPath() << ": " << numSent << " packets sent, "
              << numRcvd << " packets received, " << numDropped << " packets dropped.\n";
}

void ExtInterface::flushQueue()
{
    // does not have a queue, do nothing
}

void ExtInterface::clearQueue()
{
    // does not have a queue, do nothing
}

} // namespace inet


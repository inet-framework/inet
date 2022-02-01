//
// Copyright (C) 2014 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/tunapp/TunLoopbackApp.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/NetworkHeaderBase_m.h"
#include "inet/transportlayer/common/L4Tools.h"
#include "inet/transportlayer/contract/TransportHeaderBase_m.h"

namespace inet {

Define_Module(TunLoopbackApp);

void TunLoopbackApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        tunInterface = par("tunInterface");
        packetsSent = 0;
        packetsReceived = 0;
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        IInterfaceTable *interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        NetworkInterface *networkInterface = interfaceTable->findInterfaceByName(tunInterface);
        if (networkInterface == nullptr)
            throw cRuntimeError("TUN interface not found: %s", tunInterface);
        tunSocket.setOutputGate(gate("socketOut"));
        tunSocket.open(networkInterface->getInterfaceId());
    }
}

void TunLoopbackApp::handleMessage(cMessage *message)
{
    if (message->arrivedOn("socketIn")) {
        EV_INFO << "Message " << message->getName() << " arrived from tun. " << packetsReceived + 1 << " packets received so far\n";
        packetsReceived++;

        Packet *packet = check_and_cast<Packet *>(message);
        auto packetProtocol = getPacketProtocol(packet);
        auto networkProtocol = getNetworkProtocol(packet);
        if (packetProtocol != networkProtocol)
            throw cRuntimeError("Cannot handle packet");

        const auto& networkHeader = removeNetworkProtocolHeader(packet, networkProtocol);
        const auto& transportProtocol = *networkHeader->getProtocol();
        const auto& transportHeader = removeTransportProtocolHeader(packet, transportProtocol);

        unsigned int destPort = transportHeader->getDestinationPort();
        transportHeader->setDestinationPort(transportHeader->getSourcePort());
        transportHeader->setSourcePort(destPort);
        L3Address destAddr = networkHeader->getDestinationAddress();
        networkHeader->setDestinationAddress(networkHeader->getSourceAddress());
        networkHeader->setSourceAddress(destAddr);

        insertTransportProtocolHeader(packet, transportProtocol, transportHeader);
        insertNetworkProtocolHeader(packet, networkProtocol, networkHeader);

        delete message->removeControlInfo();
        packet->clearTags();
        tunSocket.send(packet);
        packetsSent++;
    }
    else
        throw cRuntimeError("Unknown message: %s", message->getName());
}

void TunLoopbackApp::finish()
{
    EV_INFO << "packets sent: " << packetsSent << endl;
    EV_INFO << "packets received: " << packetsReceived << endl;
}

} // namespace inet


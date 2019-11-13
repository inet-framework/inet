//
// Copyright (C) 2014 Irene Ruengeler
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/applications/tunapp/TunLoopbackApp.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/networklayer/common/L3Tools.h"
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
        InterfaceEntry *interfaceEntry = interfaceTable->findInterfaceByName(tunInterface);
        if (interfaceEntry == nullptr)
            throw cRuntimeError("TUN interface not found: %s", tunInterface);
        tunSocket.setOutputGate(gate("socketOut"));
        tunSocket.open(interfaceEntry->getInterfaceId());
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

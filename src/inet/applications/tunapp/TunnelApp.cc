//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/tun/TunControlInfo_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo.h"

#include "inet/applications/tunapp/TunnelApp.h"

namespace inet {

Define_Module(TunnelApp);

TunnelApp::TunnelApp()
{
}

TunnelApp::~TunnelApp()
{
}

void TunnelApp::initialize(int stage)
{
    ApplicationBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        interface = par("interface");
        const char *protocolName = par("protocol");
        protocol = Protocol::getProtocol(protocolName);
        destinationAddress = par("destinationAddress");
        if (protocol == &Protocol::udp) {
            destinationPort = par("destinationPort");
            localPort = par("localPort");
        }
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        if (protocol == &Protocol::ipv4) {
            l3Socket.setOutputGate(gate("socketOut"));
            l3Socket.setL3Protocol(&Protocol::ipv4);
            l3Socket.bind(&Protocol::ipv4);
        }
        if (protocol == &Protocol::udp) {
            serverSocket.setOutputGate(gate("socketOut"));
            if (localPort != -1)
                serverSocket.bind(localPort);
            clientSocket.setOutputGate(gate("socketOut"));
            if (destinationPort != -1)
                clientSocket.connect(L3AddressResolver().resolve(destinationAddress), destinationPort);
        }
        IInterfaceTable *interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        InterfaceEntry *interfaceEntry = interfaceTable->getInterfaceByName(interface);
        if (interfaceEntry == nullptr)
            throw cRuntimeError("TUN interface not found: %s", interface);
        tunSocket.setOutputGate(gate("socketOut"));
        tunSocket.open(interfaceEntry->getInterfaceId());
    }
}

void TunnelApp::handleMessageWhenUp(cMessage *message)
{
    if (message->arrivedOn("socketIn")) {
        ASSERT(message->getControlInfo() == nullptr);

        auto packet = check_and_cast<Packet *>(message);
        auto sockInd = packet->findTag<SocketInd>();
        int sockId = (sockInd != nullptr) ? sockInd->getSocketId() : -1;
        if (sockInd != nullptr && sockId == tunSocket.getSocketId()) {
            // InterfaceInd says packet is from tunnel interface and socket id is present and equals to tunSocket
            if (protocol == &Protocol::ipv4) {
                packet->clearTags();
                packet->addTag<L3AddressReq>()->setDestAddress(L3AddressResolver().resolve(destinationAddress));
                packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ipv4);
                l3Socket.send(packet);
            }
            else if (protocol == &Protocol::udp) {
                packet->clearTags();
                clientSocket.send(packet);
            }
            else
                throw cRuntimeError("Unknown protocol: %s", protocol->getName());;
        }
        else {
            auto transportProtocolTag = packet->findTag<TransportProtocolInd>();
            auto packetProtocol = transportProtocolTag ? transportProtocolTag->getProtocol() : packet->getTag<NetworkProtocolInd>()->getProtocol();
            if (protocol == packetProtocol) {
                delete message->removeControlInfo();
                packet->clearTags();
                tunSocket.send(packet);
            }
            else
                throw cRuntimeError("Unknown protocol: %s", packetProtocol->getName());;
        }
    }
    else
        throw cRuntimeError("Message arrived on unknown gate %s", message->getArrivalGate()->getFullName());
}

} // namespace inet


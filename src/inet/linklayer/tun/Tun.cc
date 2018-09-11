//
// Copyright (C) 2015 Irene Ruengeler
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

#include <algorithm>
#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/stlutils.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/tun/TunControlInfo_m.h"
#include "inet/linklayer/tun/Tun.h"
#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {

Define_Module(Tun);

void Tun::initialize(int stage)
{
    MacBase::initialize(stage);
    if (stage == INITSTAGE_LINK_LAYER)
        registerInterface();
}

void Tun::configureInterfaceEntry()
{
    InterfaceEntry *e = getContainingNicModule(this);
    e->setMtu(par("mtu"));
}

void Tun::handleMessage(cMessage *message)
{
    if (message->arrivedOn("upperLayerIn")) {
        if (message->isPacket()) {
            auto packet = check_and_cast<Packet *>(message);
            auto socketReq = packet->findTag<SocketReq>();
            // check if packet is from app by finding SocketReq with sockedId that is in socketIds
            auto sId = socketReq != nullptr ? socketReq->getSocketId() : -1;
            ASSERT(packet->getControlInfo() == nullptr);
            if (socketReq != nullptr && contains(socketIds, sId)) {
                // TODO: should we determine the network protocol by looking at the packet?!
                packet->clearTags();
                packet->addTag<InterfaceInd>()->setInterfaceId(interfaceEntry->getInterfaceId());
                packet->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
                packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ipv4);
                emit(packetSentToUpperSignal, packet);
                send(packet, "upperLayerOut");
            }
            else {
                emit(packetReceivedFromUpperSignal, packet);
                for (int socketId : socketIds) {
                    Packet *copy = packet->dup();
                    copy->clearTags();
                    copy->addTagIfAbsent<SocketInd>()->setSocketId(socketId);
                    copy->addTagIfAbsent<InterfaceInd>()->setInterfaceId(interfaceEntry->getInterfaceId());
                    copy->addTagIfAbsent<PacketProtocolTag>()->setProtocol(packet->getTag<PacketProtocolTag>()->getProtocol());
                    auto npTag = packet->getTag<NetworkProtocolInd>();
                    auto newnpTag = copy->addTagIfAbsent<NetworkProtocolInd>();
                    *newnpTag = *npTag;
                    send(copy, "upperLayerOut");
                }
                delete packet;
            }
        }
        else {
            cObject *controlInfo = message->getControlInfo();
            int socketId = check_and_cast<Request *>(message)->getTag<SocketReq>()->getSocketId();
            if (dynamic_cast<TunOpenCommand *>(controlInfo) != nullptr) {
                auto it = std::find(socketIds.begin(), socketIds.end(), socketId);
                if (it != socketIds.end())
                    throw cRuntimeError("Socket is already open: %d", socketId);
                socketIds.push_back(socketId);
                delete message;
            }
            else if (dynamic_cast<TunCloseCommand *>(controlInfo) != nullptr) {
                auto it = std::find(socketIds.begin(), socketIds.end(), socketId);
                if (it == socketIds.end())
                    throw cRuntimeError("Socket is already closed: %d", socketId);
                socketIds.erase(it);
                delete message;
            }
            else
                throw cRuntimeError("Unknown command: %s", message->getName());
        }
    }
    else
        throw cRuntimeError("Unknown message: %s", message->getName());
}

void Tun::flushQueue()
{
    // does not have a queue, do nothing
}

void Tun::clearQueue()
{
    // does not have a queue, do nothing
}

} // namespace inet


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
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/stlutils.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/tun/TunControlInfo_m.h"
#include "inet/linklayer/tun/TunInterface.h"
#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {

Define_Module(TunInterface);

void TunInterface::initialize(int stage)
{
    MacBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        registerInterface();
}

InterfaceEntry *TunInterface::createInterfaceEntry()
{
    InterfaceEntry *e = getContainingNicModule(this);
    e->setMtu(par("mtu"));
    return e;
}

void TunInterface::handleMessage(cMessage *message)
{
    if (message->arrivedOn("upperLayerIn")) {
        if (message->isPacket()) {
            auto socketReq = message->getTag<SocketReq>();
            // check if packet is from app by finding SocketReq with sockedId that is in socketIds
            auto sId = socketReq != nullptr ? socketReq->getSocketId() : -1;
            if (socketReq != nullptr && contains(socketIds, sId)) {
                ASSERT(message->getControlInfo() == nullptr);
                // TODO: should we determine the network protocol by looking at the packet?!
                message->clearTags();
                message->ensureTag<InterfaceInd>()->setInterfaceId(interfaceEntry->getInterfaceId());
                message->ensureTag<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
                message->ensureTag<PacketProtocolTag>()->setProtocol(&Protocol::ipv4);
                emit(packetSentToUpperSignal, message);
                send(message, "upperLayerOut");
            }
            else {
                emit(packetReceivedFromUpperSignal, message);
                ASSERT(message->getControlInfo() == nullptr);
                for (int socketId : socketIds) {
                    cMessage *copy = message->dup();
                    copy->clearTags();
                    copy->ensureTag<SocketInd>()->setSocketId(socketId);
                    copy->ensureTag<InterfaceInd>()->setInterfaceId(interfaceEntry->getInterfaceId());
                    copy->ensureTag<PacketProtocolTag>()->setProtocol(message->getMandatoryTag<PacketProtocolTag>()->getProtocol());
                    auto npTag = message->getMandatoryTag<NetworkProtocolInd>();
                    auto newnpTag = copy->ensureTag<NetworkProtocolInd>();
                    *newnpTag = *npTag;
                    send(copy, "upperLayerOut");
                }
                delete message;
            }
        }
        else {
            cObject *controlInfo = message->getControlInfo();
            int socketId = message->getMandatoryTag<SocketReq>()->getSocketId();
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

void TunInterface::flushQueue()
{
    // does not have a queue, do nothing
}

void TunInterface::clearQueue()
{
    // does not have a queue, do nothing
}

} // namespace inet


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
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/tun/TunControlInfo_m.h"
#include "inet/linklayer/tun/TunInterface.h"
#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {

Define_Module(TunInterface);

simsignal_t TunInterface::packetSentToLowerSignal = registerSignal("packetSentToLower");
simsignal_t TunInterface::packetReceivedFromLowerSignal = registerSignal("packetReceivedFromLower");
simsignal_t TunInterface::packetSentToUpperSignal = registerSignal("packetSentToUpper");
simsignal_t TunInterface::packetReceivedFromUpperSignal = registerSignal("packetReceivedFromUpper");

void TunInterface::initialize(int stage)
{
    MACBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        registerInterface();
}

InterfaceEntry *TunInterface::createInterfaceEntry()
{
    return new InterfaceEntry(this);
}

void TunInterface::handleMessage(cMessage *message)
{
    if (message->arrivedOn("upperLayerIn")) {
        if (message->isPacket()) {
            cObject *controlInfo = message->getControlInfo();
            if (dynamic_cast<TunSendCommand *>(controlInfo)) {
                // TODO: should we determine the network protocol by looking at the packet?!
                message->ensureTag<InterfaceInd>()->setInterfaceId(interfaceEntry->getInterfaceId());
                delete message->removeControlInfo();
                message->setControlInfo(controlInfo);
                emit(packetSentToUpperSignal, message);
                send(message, "upperLayerOut");
            }
            else {
                emit(packetReceivedFromUpperSignal, message);
                delete message->removeControlInfo();
                for (int socketId : socketIds) {
                    cMessage *copy = message->dup();
                    copy->ensureTag<SocketInd>()->setSocketId(socketId);
                    copy->ensureTag<InterfaceInd>()->setInterfaceId(interfaceEntry->getInterfaceId());
                    TunControlInfo *controlInfo = new TunControlInfo();
                    copy->setControlInfo(controlInfo);
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


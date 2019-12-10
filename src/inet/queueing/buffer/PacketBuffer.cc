//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include <algorithm>
#include "inet/common/Simsignals.h"
#include "inet/common/StringFormat.h"
#include "inet/queueing/buffer/PacketBuffer.h"
#include "inet/queueing/compat/cpacketqueue.h"

namespace inet {
namespace queueing {

Define_Module(PacketBuffer);

void PacketBuffer::initialize(int stage)
{
    PacketBufferBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        displayStringTextFormat = par("displayStringTextFormat");
        packetCapacity = par("packetCapacity");
        dataCapacity = b(par("dataCapacity"));
        packetDropperFunction = createDropperFunction(par("dropperClass"));
    }
    else if (stage == INITSTAGE_LAST)
        updateDisplayString();
}

IPacketDropperFunction *PacketBuffer::createDropperFunction(const char *dropperClass) const
{
    if (strlen(dropperClass) == 0)
        return nullptr;
    else
        return check_and_cast<IPacketDropperFunction *>(createOne(dropperClass));
}

bool PacketBuffer::isOverloaded() const
{
    return (packetCapacity != -1 && getNumPackets() > packetCapacity) ||
           (dataCapacity != b(-1) && getTotalLength() > dataCapacity);
}

b PacketBuffer::getTotalLength() const
{
    b totalLength = b(0);
    for (auto packet : packets)
        totalLength += packet->getTotalLength();
    return totalLength;
}

void PacketBuffer::addPacket(Packet *packet)
{
    Enter_Method("addPacket");
    EV_INFO << "Adding packet " << packet->getName() << " to the buffer.\n";
    emit(packetAddedSignal, packet);
    packets.push_back(packet);
    if (isOverloaded()) {
        if (packetDropperFunction != nullptr) {
            while (!isEmpty() && isOverloaded()) {
                auto packet = packetDropperFunction->selectPacket(this);
                EV_INFO << "Dropping packet " << packet->getName() << " from the buffer.\n";
                packets.erase(find(packets.begin(), packets.end(), packet));
                auto queue = dynamic_cast<cPacketQueue *>(packet->getOwner());
                if (queue != nullptr) {
                    ICallback *callback = dynamic_cast<ICallback *>(queue->getOwner());
                    if (callback != nullptr)
                        callback->handlePacketRemoved(packet);
                }
                dropPacket(packet, QUEUE_OVERFLOW);
            }
        }
        else
            throw cRuntimeError("Buffer is overloaded but packet dropper function is not specified");
    }
    updateDisplayString();
}

void PacketBuffer::removePacket(Packet *packet)
{
    Enter_Method("removePacket");
    EV_INFO << "Removing packet " << packet->getName() << " from the buffer.\n";
    emit(packetRemovedSignal, packet);
    packets.erase(find(packets.begin(), packets.end(), packet));
    updateDisplayString();
    auto queue = dynamic_cast<cPacketQueue *>(packet->getOwner());
    if (queue != nullptr) {
        ICallback *callback = dynamic_cast<ICallback *>(queue->getOwner());
        if (callback != nullptr)
            callback->handlePacketRemoved(packet);
    }
}

Packet *PacketBuffer::getPacket(int index) const
{
    if (index < 0 || (size_t)index >= packets.size())
        throw cRuntimeError("index %i out of range", index);
    return packets[index];
}

} // namespace queueing
} // namespace inet


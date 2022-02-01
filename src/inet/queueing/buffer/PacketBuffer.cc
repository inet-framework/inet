//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/buffer/PacketBuffer.h"

#include <algorithm>

#include "inet/common/Simsignals.h"
#include "inet/common/StringFormat.h"
#include "inet/common/stlutils.h"

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
    EV_INFO << "Adding packet" << EV_FIELD(packet) << EV_ENDL;
    emit(packetAddedSignal, packet);
    packets.push_back(packet);
    if (isOverloaded()) {
        if (packetDropperFunction != nullptr) {
            while (!isEmpty() && isOverloaded()) {
                auto packet = packetDropperFunction->selectPacket(this);
                EV_INFO << "Dropping packet" << EV_FIELD(packet) << EV_ENDL;
                packets.erase(find(packets, packet));
                auto queue = dynamic_cast<cPacketQueue *>(packet->getOwner());
                if (queue != nullptr) {
                    ICallback *callback = dynamic_cast<ICallback *>(queue->getOwner());
                    if (callback != nullptr)
                        callback->handlePacketRemoved(packet);
                }
                // TODO maybe the buffer should take ownership and queues should be aware of it
                take(packet);
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
    EV_INFO << "Removing packet" << EV_FIELD(packet) << EV_ENDL;
    emit(packetRemovedSignal, packet);
    packets.erase(find(packets, packet));
    updateDisplayString();
    auto queue = dynamic_cast<cPacketQueue *>(packet->getOwner());
    if (queue != nullptr) {
        ICallback *callback = dynamic_cast<ICallback *>(queue->getOwner());
        if (callback != nullptr)
            callback->handlePacketRemoved(packet);
    }
}

void PacketBuffer::removeAllPackets()
{
    Enter_Method("removeAllPacket");
    EV_INFO << "Removing all packets" << EV_ENDL;
    while (!isEmpty()) {
        auto packet = getPacket(0);
        emit(packetRemovedSignal, packet);
        packets.erase(packets.begin());
        auto queue = dynamic_cast<cPacketQueue *>(packet->getOwner());
        if (queue != nullptr) {
            ICallback *callback = dynamic_cast<ICallback *>(queue->getOwner());
            if (callback != nullptr)
                callback->handlePacketRemoved(packet);
        }
    }
    updateDisplayString();
}

Packet *PacketBuffer::getPacket(int index) const
{
    if (index < 0 || (size_t)index >= packets.size())
        throw cRuntimeError("index %i out of range", index);
    return packets[index];
}

} // namespace queueing
} // namespace inet


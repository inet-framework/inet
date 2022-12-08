//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/dissector/PacketDissector.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/chunk/SequenceChunk.h"

namespace inet {

// PacketDissector::ProtocolDissectorCallback

PacketDissector::ProtocolDissectorCallback::ProtocolDissectorCallback(const PacketDissector& packetDissector) :
    packetDissector(packetDissector)
{
}

void PacketDissector::ProtocolDissectorCallback::startProtocolDataUnit(const Protocol *protocol)
{
    packetDissector.callback.startProtocolDataUnit(protocol);
}

void PacketDissector::ProtocolDissectorCallback::endProtocolDataUnit(const Protocol *protocol)
{
    packetDissector.callback.endProtocolDataUnit(protocol);
}

void PacketDissector::ProtocolDissectorCallback::markIncorrect()
{
    packetDissector.callback.markIncorrect();
}

void PacketDissector::ProtocolDissectorCallback::visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol)
{
    packetDissector.callback.visitChunk(chunk, protocol);
}

void PacketDissector::ProtocolDissectorCallback::dissectPacket(Packet *packet, const Protocol *protocol)
{
    packetDissector.doDissectPacket(packet, protocol);
}

// ProtocolDataUnit

PacketDissector::ProtocolDataUnit::ProtocolDataUnit(int level, const Protocol *protocol) :
    level(level),
    protocol(protocol)
{
}

b PacketDissector::ProtocolDataUnit::getChunkLength() const
{
    b length = b(0);
    for (const auto& chunk : chunks)
        length += chunk->getChunkLength();
    return length;
}

// ChunkBuilder

void PacketDissector::ChunkBuilder::visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol)
{
    if (content == nullptr)
        content = chunk;
    else {
        if (content->canInsertAtBack(chunk)) {
            const auto& newContent = makeExclusivelyOwnedMutableChunk(content);
            newContent->insertAtBack(chunk);
            newContent->markImmutable();
            content = newContent->simplify();
        }
        else {
            auto sequenceChunk = makeShared<SequenceChunk>();
            sequenceChunk->insertAtBack(content);
            sequenceChunk->insertAtBack(chunk);
            sequenceChunk->markImmutable();
            content = sequenceChunk;
        }
    }
}

// PduTreeBuilder

void PacketDissector::PduTreeBuilder::startProtocolDataUnit(const Protocol *protocol)
{
    if (isEndProtocolDataUnitCalled)
        isSimplyEncapsulatedPacket_ = false;
    auto level = makeShared<ProtocolDataUnit>(pduLevels.size(), protocol);
    if (pduLevels.size() == 0) {
        if (topLevelPdu == nullptr)
            topLevelPdu = level;
        else if (remainingJunk == nullptr)
            remainingJunk = level;
        else
            throw cRuntimeError("Invalid state");
    }
    else
        pduLevels.top()->insert(level);
    pduLevels.push(level.get());
}

void PacketDissector::PduTreeBuilder::endProtocolDataUnit(const Protocol *protocol)
{
    isEndProtocolDataUnitCalled = true;
    pduLevels.pop();
}

void PacketDissector::PduTreeBuilder::markIncorrect()
{
    pduLevels.top()->markIncorrect();
}

void PacketDissector::PduTreeBuilder::visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol)
{
    pduLevels.top()->insert(chunk);
}

// PacketDissector

PacketDissector::PacketDissector(const ProtocolDissectorRegistry& protocolDissectorRegistry, ICallback& callback) :
    protocolDissectorRegistry(protocolDissectorRegistry),
    callback(callback)
{
}

void PacketDissector::doDissectPacket(Packet *packet, const Protocol *protocol) const
{
    auto protocolDissector = callback.shouldDissectProtocolDataUnit(protocol) ? protocolDissectorRegistry.findProtocolDissector(protocol) : nullptr;
    if (protocolDissector == nullptr)
        protocolDissector = protocolDissectorRegistry.getProtocolDissector(nullptr);
    ProtocolDissectorCallback callback(*this);
    protocolDissector->dissect(packet, protocol, callback);
}

void PacketDissector::dissectPacket(Packet *packet) const
{
    const auto& packetProtocolTag = packet->findTag<PacketProtocolTag>();
    auto protocol = packetProtocolTag != nullptr ? packetProtocolTag->getProtocol() : nullptr;
    if (packetProtocolTag != nullptr)
        dissectPacket(packet, protocol, packetProtocolTag->getFrontOffset(), packetProtocolTag->getBackOffset());
    else
        dissectPacket(packet, protocol);
}

void PacketDissector::dissectPacket(Packet *packet, const Protocol *protocol, b extraFrontOffset, b extraBackOffset) const
{
    auto frontOffset = packet->getFrontOffset();
    auto backOffset = packet->getBackOffset();
    try {
        packet->setFrontOffset(frontOffset + extraFrontOffset);
        packet->setBackOffset(backOffset + extraBackOffset);
        // dissect packet data part according to protocol
        doDissectPacket(packet, protocol);
        // dissect remaining junk in packet without protocol (e.g. ethernet padding at IP level)
        if (packet->getDataLength() != b(0))
            doDissectPacket(packet, nullptr);
        ASSERT(packet->getDataLength() == b(0));
    }
    catch (...) {
        packet->setFrontOffset(frontOffset);
        packet->setBackOffset(backOffset);
        throw;
    }
    packet->setFrontOffset(frontOffset);
    packet->setBackOffset(backOffset);
}

} // namespace


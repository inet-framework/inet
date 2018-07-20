//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/common/packet/dissector/PacketDissector.h"

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

PacketDissector::ProtocolDataUnit::ProtocolDataUnit(int level, const Protocol* protocol) :
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
    auto protocolDissector = protocolDissectorRegistry.findProtocolDissector(protocol);
    if (protocolDissector == nullptr)
        protocolDissector = protocolDissectorRegistry.getProtocolDissector(nullptr);
    ProtocolDissectorCallback callback(*this);
    protocolDissector->dissect(packet, callback);
}

void PacketDissector::dissectPacket(Packet *packet, const Protocol *protocol) const
{
    auto headerPopOffset = packet->getFrontOffset();
    auto trailerPopOffset = packet->getBackOffset();
    // dissect packet data part according to protocol
    doDissectPacket(packet, protocol);
    // dissect remaining junk in packet without protocol (e.g. ethernet padding at IP level)
    if (packet->getDataLength() != b(0))
        doDissectPacket(packet, nullptr);
    ASSERT(packet->getDataLength() == b(0));
    packet->setFrontOffset(headerPopOffset);
    packet->setBackOffset(trailerPopOffset);
}

} // namespace


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

void PacketDissector::ProtocolDissectorCallback::startProtocol(const Protocol *protocol)
{
    packetDissector.callback.startProtocol(protocol);
}

void PacketDissector::ProtocolDissectorCallback::endProtocol(const Protocol *protocol)
{
    packetDissector.callback.endProtocol(protocol);
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

// PduTreeBuilder

void PacketDissector::PduTreeBuilder::startProtocol(const Protocol *protocol)
{
    if (isEndProtocolCalled)
        isSimplePacket_ = false;
    auto level = makeShared<ProtocolDataUnit>(levels.size(), protocol);
    if (topLevel == nullptr)
        topLevel = level;
    else
        levels.top()->insert(level);
    levels.push(level.get());
}

void PacketDissector::PduTreeBuilder::endProtocol(const Protocol *protocol)
{
    isEndProtocolCalled = true;
    levels.pop();
}

void PacketDissector::PduTreeBuilder::visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol)
{
    levels.top()->insert(chunk);
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
    auto headerPopOffset = packet->getHeaderPopOffset();
    auto trailerPopOffset = packet->getTrailerPopOffset();
    doDissectPacket(packet, protocol);
    ASSERT(packet->getDataLength() == B(0));
    packet->setHeaderPopOffset(headerPopOffset);
    packet->setTrailerPopOffset(trailerPopOffset);
}

} // namespace


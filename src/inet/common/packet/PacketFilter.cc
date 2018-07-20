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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/packet/dissector/PacketDissector.h"
#include "inet/common/packet/PacketFilter.h"
#include "inet/common/ProtocolTag_m.h"

namespace inet {

void PacketFilter::setPattern(const char* packetPattern, const char* chunkPattern)
{
    packetMatchExpression.setPattern(packetPattern, false, true, true);
    chunkMatchExpression.setPattern(chunkPattern, false, true, true);
}

bool PacketFilter::matches(const cPacket *cpacket) const
{
    MatchableObject matchableObject(MatchableObject::ATTRIBUTE_FULLNAME, cpacket);
    // TODO: eliminate const_cast when cMatchExpression::matches becomes const
    if (!const_cast<PacketFilter *>(this)->packetMatchExpression.matches(&matchableObject))
        return false;
    else if (auto packet = dynamic_cast<const Packet *>(cpacket)) {
        PacketDissectorCallback callback(*this);
        return callback.matches(packet);
    }
    else
        return true;
}

PacketFilter::PacketDissectorCallback::PacketDissectorCallback(const PacketFilter& packetFilter) :
    packetFilter(packetFilter)
{
}

bool PacketFilter::PacketDissectorCallback::matches(const Packet *packet)
{
    auto packetProtocolTag = packet->findTag<PacketProtocolTag>();
    auto protocol = packetProtocolTag != nullptr ? packetProtocolTag->getProtocol() : nullptr;
    PacketDissector packetDissector(ProtocolDissectorRegistry::globalRegistry, *this);
    auto copy = packet->dup();
    packetDissector.dissectPacket(copy, protocol);
    delete copy;
    return matches_;
}

void PacketFilter::PacketDissectorCallback::startProtocolDataUnit(const Protocol *protocol)
{
}

void PacketFilter::PacketDissectorCallback::endProtocolDataUnit(const Protocol *protocol)
{
}

void PacketFilter::PacketDissectorCallback::markIncorrect()
{
}

void PacketFilter::PacketDissectorCallback::visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol)
{
    MatchableObject matchableObject(MatchableObject::ATTRIBUTE_CLASSNAME, chunk.get());
    if (!matches_)
        // TODO: eliminate const_cast when cMatchExpression::matches becomes const
        matches_ = const_cast<PacketFilter *>(&this->packetFilter)->chunkMatchExpression.matches(&matchableObject);
}

} // namespace inet

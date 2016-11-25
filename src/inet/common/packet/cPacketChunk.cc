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

#include "inet/common/packet/cPacketChunk.h"

namespace inet {

cPacketChunk::cPacketChunk(cPacket *packet) :
    Chunk(),
    packet(packet)
{
    take(packet);
}

cPacketChunk::cPacketChunk(const cPacketChunk& other) :
    Chunk(other),
    packet(other.packet->dup())
{
    take(packet);
}

cPacketChunk::~cPacketChunk()
{
    dropAndDelete(packet);
}

std::string cPacketChunk::str() const {
    std::ostringstream os;
    os << "cPacketChunk, packet = {" << ( packet != nullptr ? packet->str() : std::string("<null>")) << "}";
    return os.str();
}

} // namespace

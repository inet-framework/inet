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


import inet.common.packet.chunk.Chunk;

namespace inet::quic;

enum PacketHeaderType {
    PACKET_HEADER_TYPE_INITIAL = 0xc; // 1100
    PACKET_HEADER_TYPE_0RTT = 0xd; // 1101
    PACKET_HEADER_TYPE_HANDSHAKE = 0xe; // 1110
    PACKET_HEADER_TYPE_RETRY = 0xf; // 1111
    PACKET_HEADER_TYPE_SHORT = 0x4; // 0100
}

class PacketHeader extends FieldsChunk
{
    uint8_t packetType;
}

class InitialPacketHeader extends PacketHeader
{
    
}

class ShortPacketHeader extends PacketHeader
{
    packetType = PACKET_HEADER_TYPE_SHORT;
    uint64_t connectionID;
    uint32_t packetNumber;
    bool iBit = false;
}
cplusplus(ShortPacketHeader) {{
  public:
    static const size_t SIZE = 13;
    std::string str() const override {
        std::ostringstream os;
        os << "Short[connectionID=" << connectionID
            << ", packetNumber=" << packetNumber
            << "]";
        return os.str();
    }
}}
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

cplusplus {{
    typedef omnetpp::cPacket *cPacketPtr;
}}

struct cPacketPtr;

class Fragment
{
    cPacketPtr data = nullptr;
    int byteOffset = -1;
    int byteLength = -1;
}

cplusplus {{
    typedef Fragment *FragmentPtr;
}}

struct FragmentPtr;

enum Protocol
{
    Tcp = 0;
    Ip = 1;
    Ethernet = 2;
}

packet ByteArrayData
{
    uint8_t bytes[];
}

packet ByteLengthData
{
}

packet ApplicationData
{
    byteLength = 10;
    uint16_t someData = -1;
}

packet TcpSegment
{
    @customize(true);
    uint16_t srcPort = -1;
    uint16_t destPort = -1;
    FragmentPtr payload[];
}

packet IpDatagram
{
    int protocol @enum(Protocol) = -1;
}

packet EthernetFrame
{
    int protocol @enum(Protocol) = -1;
}

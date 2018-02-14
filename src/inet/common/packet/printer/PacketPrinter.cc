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

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/ProtocolTag_m.h"

#ifdef WITH_IPv4
#include "inet/networklayer/arp/ipv4/ArpPacket_m.h"
#include "inet/networklayer/ipv4/IcmpHeader.h"
#include "inet/networklayer/ipv4/Ipv4Header.h"
#endif // ifdef WITH_IPv4

namespace inet {

Register_MessagePrinter(PacketPrinter);

int PacketPrinter::getScoreFor(cMessage *msg) const
{
    return msg->isPacket() ? 100 : 0;
}

void PacketPrinter::printMessage(std::ostream& stream, cMessage *message) const
{
    auto separator = "";
    for (auto cpacket = dynamic_cast<cPacket *>(message); cpacket != nullptr; cpacket = cpacket->getEncapsulatedPacket()) {
        stream << separator;
        if (auto packet = dynamic_cast<Packet *>(cpacket))
            printPacket(stream, packet);
        else
            stream << separator << cpacket->getClassName() << ":" << cpacket->getByteLength() << " bytes";
        separator = "  \t";
    }
}

void PacketPrinter::printPacket(std::ostream& stream, Packet *packet) const
{
    ChunkVisitor chunkVisitor(stream, *this);
    auto packetProtocolTag = packet->findTag<PacketProtocolTag>();
    auto protocol = packetProtocolTag != nullptr ? packetProtocolTag->getProtocol() : nullptr;
    PacketDissector packetDissector(ProtocolDissectorRegistry::globalRegistry, chunkVisitor);
    auto copy = packet->dup();
    packetDissector.dissectPacket(copy, protocol);
    delete copy;
    stream << std::endl;
}

void PacketPrinter::printIeee80211Chunk(std::ostream& stream, const Ptr<const Chunk>& chunk) const
{
    stream << "(IEEE 802.11) " << chunk;
}

void PacketPrinter::printIeee8022Chunk(std::ostream& stream, const Ptr<const Chunk>& chunk) const
{
    stream << "(IEEE 802.2) " << chunk;
}

void PacketPrinter::printArpChunk(std::ostream& stream, const Ptr<const Chunk>& chunk) const
{
    if (auto packet = dynamicPtrCast<const ArpPacket>(chunk)) {
        switch (packet->getOpcode()) {
            case ARP_REQUEST:
                stream << "ARP req: " << packet->getDestIPAddress()
                       << "=? (s=" << packet->getSrcIPAddress() << "(" << packet->getSrcMACAddress() << "))";
                break;

            case ARP_REPLY:
                stream << "ARP reply: "
                       << packet->getSrcIPAddress() << "=" << packet->getSrcMACAddress()
                       << " (d=" << packet->getDestIPAddress() << "(" << packet->getDestMACAddress() << "))";
                break;

            case ARP_RARP_REQUEST:
                stream << "RARP req: " << packet->getDestMACAddress()
                       << "=? (s=" << packet->getSrcIPAddress() << "(" << packet->getSrcMACAddress() << "))";
                break;

            case ARP_RARP_REPLY:
                stream << "RARP reply: "
                       << packet->getSrcMACAddress() << "=" << packet->getSrcIPAddress()
                       << " (d=" << packet->getDestIPAddress() << "(" << packet->getDestMACAddress() << "))";
                break;

            default:
                stream << "ARP op=" << packet->getOpcode() << ": d=" << packet->getDestIPAddress()
                       << "(" << packet->getDestMACAddress()
                       << ") s=" << packet->getSrcIPAddress()
                       << "(" << packet->getSrcMACAddress() << ")";
                break;
        }
    }
    else
        stream << "(ARP) " << chunk;
}

void PacketPrinter::printIpv4Chunk(std::ostream& stream, const Ptr<const Chunk>& chunk) const
{
    stream << "(IPv4) " << chunk;
}

void PacketPrinter::printIcmpChunk(std::ostream& stream, const Ptr<const Chunk>& chunk) const
{
    stream << "(ICMP) " << chunk;
}

void PacketPrinter::printUdpChunk(std::ostream& stream, const Ptr<const Chunk>& chunk) const
{
    stream << "(UDP) " << chunk;
}

void PacketPrinter::printUnimplementedProtocolChunk(std::ostream& stream, const Ptr<const Chunk>& chunk, const Protocol* protocol) const
{
    stream << "(UNIMPLEMENTED " << protocol->getName() << ") " << chunk;
}

void PacketPrinter::printUnknownProtocolChunk(std::ostream& stream, const Ptr<const Chunk>& chunk) const
{
    stream << "(UNKNOWN) " << chunk;
}

PacketPrinter::ChunkVisitor::ChunkVisitor(std::ostream& stream, const PacketPrinter& packetPrinter) :
    stream(stream),
    packetPrinter(packetPrinter)
{
}

void PacketPrinter::ChunkVisitor::startProtocol(const Protocol *protocol) const
{
}

void PacketPrinter::ChunkVisitor::endProtocol(const Protocol *protocol) const
{
}

void PacketPrinter::ChunkVisitor::visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol) const
{
    if (protocol == &Protocol::ieee80211)
        packetPrinter.printIeee80211Chunk(stream, chunk);
    else if (protocol == &Protocol::ieee8022)
        packetPrinter.printIeee8022Chunk(stream, chunk);
    else if (protocol == &Protocol::arp)
        packetPrinter.printArpChunk(stream, chunk);
    else if (protocol == &Protocol::ipv4)
        packetPrinter.printIpv4Chunk(stream, chunk);
    else if (protocol == &Protocol::icmpv4)
        packetPrinter.printIcmpChunk(stream, chunk);
    else if (protocol == &Protocol::udp)
        packetPrinter.printUdpChunk(stream, chunk);
    else if (protocol != nullptr)
        packetPrinter.printUnimplementedProtocolChunk(stream, chunk, protocol);
    else
        packetPrinter.printUnknownProtocolChunk(stream, chunk);
    stream << " ";
}

} // namespace


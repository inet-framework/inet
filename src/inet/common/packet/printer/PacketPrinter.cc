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

#ifdef WITH_ETHERNET
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#endif // ifdef WITH_ETHERNET

#ifdef WITH_IEEE80211
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#endif // ifdef WITH_IEEE80211

#ifdef WITH_IPv4
#include "inet/networklayer/arp/ipv4/ArpPacket_m.h"
#include "inet/networklayer/ipv4/IcmpHeader.h"
#include "inet/networklayer/ipv4/Ipv4Header.h"
#endif // ifdef WITH_IPv4

#ifdef WITH_TCP_COMMON
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#endif // ifdef WITH_TCP_COMMON

#ifdef WITH_UDP
#include "inet/transportlayer/udp/UdpHeader_m.h"
#endif // ifdef WITH_UDP

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
    PacketDissector::ProtocolTreeBuilder protocolTreeBuilder;
    auto packetProtocolTag = packet->findTag<PacketProtocolTag>();
    auto protocol = packetProtocolTag != nullptr ? packetProtocolTag->getProtocol() : nullptr;
    PacketDissector packetDissector(ProtocolDissectorRegistry::globalRegistry, protocolTreeBuilder);
    auto copy = packet->dup();
    packetDissector.dissectPacket(copy, protocol);
    delete copy;
    Context context;
    if (protocolTreeBuilder.isSimplePacket())
        const_cast<PacketPrinter *>(this)->printPacketInsideOut(protocolTreeBuilder.getTopLevel(), context);
    else {
        context.sourceColumn << "mixed";
        context.destinationColumn << "mixed";
        context.protocolColumn = "mixed";
        const_cast<PacketPrinter *>(this)->printPacketLeftToRight(protocolTreeBuilder.getTopLevel(), context);
    }
    stream << context.sourceColumn.str() << "\t" << context.destinationColumn.str() << "\t" << context.protocolColumn << "\t" << context.infoColumn.str() << std::endl;
}

void PacketPrinter::printPacketInsideOut(const Ptr<const PacketDissector::ProtocolLevel>& protocolLevel, Context& context)
{
    auto protocol = protocolLevel->getProtocol();
    for (const auto& chunk : protocolLevel->getChunks()) {
        if (auto childLevel = dynamicPtrCast<const PacketDissector::ProtocolLevel>(chunk))
            printPacketInsideOut(childLevel, context);
        else if (protocol == &Protocol::ethernet) {
            auto header = dynamicPtrCast<const EthernetMacHeader>(chunk);
            if (header != nullptr) {
                context.sourceColumn << header->getSrc();
                context.destinationColumn << header->getDest();
            }
            if (protocolLevel->getLevel() > context.infoLevel) {
                context.infoLevel = protocolLevel->getLevel();
                context.protocolColumn = protocol->getName();
                context.infoColumn.str("");
                // TODO: printEthernetChunk(context.infoColumn, chunk);
            }
        }
        else if (protocol == &Protocol::ieee80211) {
            auto header = dynamicPtrCast<const ieee80211::Ieee80211TwoAddressHeader>(chunk);
            if (header != nullptr) {
                context.sourceColumn << header->getTransmitterAddress();
                context.destinationColumn << header->getReceiverAddress();
            }
            if (protocolLevel->getLevel() > context.infoLevel) {
                context.infoLevel = protocolLevel->getLevel();
                context.protocolColumn = protocol->getName();
                context.infoColumn.str("");
                printIeee80211Chunk(context.infoColumn, chunk);
            }
        }
        else if (protocol == &Protocol::ieee8022) {
            if (protocolLevel->getLevel() > context.infoLevel) {
                context.infoLevel = protocolLevel->getLevel();
                context.infoColumn.str("");
                printIeee8022Chunk(context.infoColumn, chunk);
            }
        }
        else if (protocol == &Protocol::arp) {
            if (protocolLevel->getLevel() > context.infoLevel) {
                context.infoLevel = protocolLevel->getLevel();
                context.protocolColumn = protocol->getName();
                context.infoColumn.str("");
                printArpChunk(context.infoColumn, chunk);
            }
        }
        else if (protocol == &Protocol::ipv4) {
            auto header = dynamicPtrCast<const Ipv4Header>(chunk);
            if (header != nullptr) {
                context.sourceColumn.str("");
                context.sourceColumn << header->getSrcAddress();
                context.destinationColumn.str("");
                context.destinationColumn << header->getDestAddress();
            }
            if (protocolLevel->getLevel() > context.infoLevel) {
                context.infoLevel = protocolLevel->getLevel();
                context.protocolColumn = protocol->getName();
                context.infoColumn.str("");
                printIpv4Chunk(context.infoColumn, chunk);
            }
        }
        else if (protocol == &Protocol::icmpv4) {
            if (protocolLevel->getLevel() > context.infoLevel) {
                context.infoLevel = protocolLevel->getLevel();
                context.protocolColumn = protocol->getName();
                context.infoColumn.str("");
                printIcmpChunk(context.infoColumn, chunk);
            }
        }
        else if (protocol == &Protocol::udp) {
            auto header = dynamicPtrCast<const UdpHeader>(chunk);
            if (header != nullptr) {
                context.sourceColumn << ":" << header->getSrcPort();
                context.destinationColumn << ":" << header->getDestPort();
            }
            if (protocolLevel->getLevel() > context.infoLevel) {
                context.infoLevel = protocolLevel->getLevel();
                context.protocolColumn = protocol->getName();
                context.infoColumn.str("");
                printUdpChunk(context.infoColumn, chunk);
            }
        }
        else if (protocol == &Protocol::tcp) {
            auto header = dynamicPtrCast<const tcp::TcpHeader>(chunk);
            if (header != nullptr) {
                context.sourceColumn << ":" << header->getSrcPort();
                context.destinationColumn << ":" << header->getDestPort();
            }
            if (protocolLevel->getLevel() > context.infoLevel) {
                context.infoLevel = protocolLevel->getLevel();
                context.protocolColumn = protocol->getName();
                context.infoColumn.str("");
                // TODO: printTcpChunk(context.infoColumn, chunk);
            }
        }
        else if (protocol != nullptr) {
            if (protocolLevel->getLevel() > context.infoLevel) {
                context.infoLevel = protocolLevel->getLevel();
                context.infoColumn.str("");
                printUnimplementedProtocolChunk(context.infoColumn, chunk, protocol);
            }
        }
        else {
            if (protocolLevel->getLevel() > context.infoLevel) {
                context.infoLevel = protocolLevel->getLevel();
                context.infoColumn.str("");
                printUnknownProtocolChunk(context.infoColumn, chunk);
            }
        }
    }
}

void PacketPrinter::printPacketLeftToRight(const Ptr<const PacketDissector::ProtocolLevel>& protocolLevel, Context& context)
{
    auto protocol = protocolLevel->getProtocol();
    for (const auto& chunk : protocolLevel->getChunks()) {
        if (auto childLevel = dynamicPtrCast<const PacketDissector::ProtocolLevel>(chunk))
            printPacketLeftToRight(childLevel, context);
        else if (protocol == &Protocol::ieee80211)
            printIeee80211Chunk(context.infoColumn, chunk);
        else if (protocol == &Protocol::ieee8022)
            printIeee8022Chunk(context.infoColumn, chunk);
        else if (protocol == &Protocol::arp)
            printArpChunk(context.infoColumn, chunk);
        else if (protocol == &Protocol::ipv4)
            printIpv4Chunk(context.infoColumn, chunk);
        else if (protocol == &Protocol::icmpv4)
            printIcmpChunk(context.infoColumn, chunk);
        else if (protocol == &Protocol::udp)
            printUdpChunk(context.infoColumn, chunk);
        else if (protocol != nullptr)
            printUnimplementedProtocolChunk(context.infoColumn, chunk, protocol);
        else
            printUnknownProtocolChunk(context.infoColumn, chunk);
    }
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

} // namespace


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

#include <algorithm>
#include "inet/common/packet/chunk/BitCountChunk.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
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
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
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

bool PacketPrinter::isEnabledOption(const Options *options, const char *name) const
{
    // TODO: reenable when new omnet is released
    // return std::find(options->enabledTags.begin(), options->enabledTags.end(), name) != options->enabledTags.end();
    return true;
}

std::vector<std::string> PacketPrinter::getSupportedTags() const
{
    return {"source_column", "destination_column", "protocol_column", "length_column", "info_column",
            "inside_out", "left_to_right"};
}

std::vector<std::string> PacketPrinter::getDefaultEnabledTags() const
{
    return {"source_column", "destination_column", "protocol_column", "length_column", "info_column", "inside_out"};
}

std::vector<std::string> PacketPrinter::getColumnNames(const Options *options) const
{
    std::vector<std::string> columnNames;
    if (isEnabledOption(options, "source_column"))
        columnNames.push_back("Source");
    if (isEnabledOption(options, "destination_column"))
        columnNames.push_back("Destination");
    if (isEnabledOption(options, "protocol_column"))
        columnNames.push_back("Protocol");
    if (isEnabledOption(options, "length_column"))
        columnNames.push_back("Length");
    if (isEnabledOption(options, "info_column"))
        columnNames.push_back("Info");
    return columnNames;
}

void PacketPrinter::printContext(std::ostream& stream, const Options *options, Context& context) const
{
    if (!context.isCorrect)
        stream << "\x1b[103m";
    stream << "\x1b[30m";
    if (isEnabledOption(options, "source_column"))
       stream << context.sourceColumn.str() << "\t";
    if (isEnabledOption(options, "destination_column"))
       stream << context.destinationColumn.str() << "\t";
    if (isEnabledOption(options, "protocol_column"))
       stream << "\x1b[34m" << context.protocolColumn << "\x1b[30m\t";
    if (isEnabledOption(options, "length_column"))
       stream << context.lengthColumn.str() << "\t";
    if (isEnabledOption(options, "info_column"))
       stream << context.infoColumn.str();
    stream << std::endl;
}

void PacketPrinter::printMessage(std::ostream& stream, cMessage *message) const
{
    // TODO: enable when migrating to new printer API
//    Options options;
//    options.enabledTags = getDefaultEnabledTags();
//    printMessage(stream, message, &options);
    printMessage(stream, message, nullptr);
}

void PacketPrinter::printMessage(std::ostream& stream, cMessage *message, const Options *options) const
{
    Context context;
    for (auto cpacket = dynamic_cast<cPacket *>(message); cpacket != nullptr; cpacket = cpacket->getEncapsulatedPacket()) {
        if (auto signal = dynamic_cast<inet::physicallayer::Signal *>(cpacket))
            printSignal(signal, options, context);
        else if (auto packet = dynamic_cast<Packet *>(cpacket))
            printPacket(packet, options, context);
        else
            context.infoColumn << cpacket->str();
    }
    printContext(stream, options, context);
}

void PacketPrinter::printSignal(std::ostream& stream, inet::physicallayer::Signal *signal) const
{
    // TODO: enable when migrating to new printer API
//    Options options;
//    options.enabledTags = getDefaultEnabledTags();
//    printMessage(stream, message, &options);
    printSignal(stream, signal, nullptr);
}

void PacketPrinter::printSignal(std::ostream& stream, inet::physicallayer::Signal *signal, const Options *options) const
{
    Context context;
    printSignal(signal, options, context);
    printContext(stream, options, context);
}

void PacketPrinter::printSignal(inet::physicallayer::Signal *signal, const Options *options, Context& context) const
{
    context.infoColumn << signal->str();
}

void PacketPrinter::printPacket(std::ostream& stream, Packet *packet) const
{
    // TODO: enable when migrating to new printer API
//    Options options;
//    options.enabledTags = getDefaultEnabledTags();
//    printMessage(stream, message, &options);
    printPacket(stream, packet, nullptr);
}

void PacketPrinter::printPacket(std::ostream& stream, Packet *packet, const Options *options) const
{
    Context context;
    printPacket(packet, options, context);
    printContext(stream, options, context);
}

void PacketPrinter::printPacket(Packet *packet, const Options *options, Context& context) const
{
    PacketDissector::PduTreeBuilder pduTreeBuilder;
    auto packetProtocolTag = packet->findTag<PacketProtocolTag>();
    auto protocol = packetProtocolTag != nullptr ? packetProtocolTag->getProtocol() : nullptr;
    PacketDissector packetDissector(ProtocolDissectorRegistry::globalRegistry, pduTreeBuilder);
    packetDissector.dissectPacket(packet, protocol);
    auto& protocolDataUnit = pduTreeBuilder.getTopLevelPdu();
    context.lengthColumn << protocolDataUnit->getChunkLength();
    if (pduTreeBuilder.isSimplyEncapsulatedPacket())
        const_cast<PacketPrinter *>(this)->printPacketInsideOut(protocolDataUnit, context);
    else
        const_cast<PacketPrinter *>(this)->printPacketLeftToRight(protocolDataUnit, context);
}

void PacketPrinter::printPacketInsideOut(const Ptr<const PacketDissector::ProtocolDataUnit>& protocolDataUnit, Context& context) const
{
    auto protocol = protocolDataUnit->getProtocol();
    context.isCorrect &= protocolDataUnit->isCorrect();
    for (const auto& chunk : protocolDataUnit->getChunks()) {
        if (auto childLevel = dynamicPtrCast<const PacketDissector::ProtocolDataUnit>(chunk))
            printPacketInsideOut(childLevel, context);
        else if (protocol == &Protocol::ethernetMac) {
            auto header = dynamicPtrCast<const EthernetMacHeader>(chunk);
            if (header != nullptr) {
                context.sourceColumn << header->getSrc();
                context.destinationColumn << header->getDest();
            }
            if (protocolDataUnit->getLevel() > context.infoLevel) {
                context.infoLevel = protocolDataUnit->getLevel();
                context.protocolColumn = protocol->getDescriptiveName();
                context.infoColumn.str("");
                // TODO: printEthernetChunk(context.infoColumn, chunk);
            }
        }
        else if (protocol == &Protocol::ieee80211Phy) {
            if (protocolDataUnit->getLevel() > context.infoLevel) {
                context.infoLevel = protocolDataUnit->getLevel();
                context.protocolColumn = protocol->getDescriptiveName();
                context.infoColumn.str("");
                printIeee80211PhyChunk(context.infoColumn, chunk);
            }
        }
        else if (protocol == &Protocol::ieee80211Mac) {
            auto oneAddressHeader = dynamicPtrCast<const ieee80211::Ieee80211OneAddressHeader>(chunk);
            if (oneAddressHeader != nullptr)
                context.destinationColumn << oneAddressHeader->getReceiverAddress();
            auto twoAddressHeader = dynamicPtrCast<const ieee80211::Ieee80211TwoAddressHeader>(chunk);
            if (twoAddressHeader != nullptr)
                context.sourceColumn << twoAddressHeader->getTransmitterAddress();
            if (protocolDataUnit->getLevel() > context.infoLevel) {
                context.infoLevel = protocolDataUnit->getLevel();
                context.protocolColumn = protocol->getDescriptiveName();
                context.infoColumn.str("");
                printIeee80211MacChunk(context.infoColumn, chunk);
            }
        }
        else if (protocol == &Protocol::ieee80211Mgmt) {
            if (protocolDataUnit->getLevel() > context.infoLevel) {
                context.infoLevel = protocolDataUnit->getLevel();
                context.protocolColumn = protocol->getDescriptiveName();
                context.infoColumn.str("");
                printIeee80211MgmtChunk(context.infoColumn, chunk);
            }
        }
        else if (protocol == &Protocol::ieee8022) {
            if (protocolDataUnit->getLevel() > context.infoLevel) {
                context.infoLevel = protocolDataUnit->getLevel();
                context.infoColumn.str("");
                printIeee8022Chunk(context.infoColumn, chunk);
            }
        }
        else if (protocol == &Protocol::arp) {
            if (protocolDataUnit->getLevel() > context.infoLevel) {
                context.infoLevel = protocolDataUnit->getLevel();
                context.protocolColumn = protocol->getDescriptiveName();
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
            if (protocolDataUnit->getLevel() > context.infoLevel) {
                context.infoLevel = protocolDataUnit->getLevel();
                context.protocolColumn = protocol->getDescriptiveName();
                context.infoColumn.str("");
                printIpv4Chunk(context.infoColumn, chunk);
            }
        }
        else if (protocol == &Protocol::icmpv4) {
            if (protocolDataUnit->getLevel() > context.infoLevel) {
                context.infoLevel = protocolDataUnit->getLevel();
                context.protocolColumn = protocol->getDescriptiveName();
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
            if (protocolDataUnit->getLevel() > context.infoLevel) {
                context.infoLevel = protocolDataUnit->getLevel();
                context.protocolColumn = protocol->getDescriptiveName();
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
            if (protocolDataUnit->getLevel() > context.infoLevel) {
                context.infoLevel = protocolDataUnit->getLevel();
                context.protocolColumn = protocol->getDescriptiveName();
                context.infoColumn.str("");
                // TODO: printTcpChunk(context.infoColumn, chunk);
            }
        }
        else if (protocol != nullptr) {
            if (protocolDataUnit->getLevel() > context.infoLevel) {
                context.infoLevel = protocolDataUnit->getLevel();
                context.infoColumn.str("");
                printUnimplementedProtocolChunk(context.infoColumn, chunk, protocol);
            }
        }
        else {
            if (protocolDataUnit->getLevel() > context.infoLevel) {
                context.infoLevel = protocolDataUnit->getLevel();
                context.infoColumn.str("");
                printUnknownProtocolChunk(context.infoColumn, chunk);
            }
        }
    }
}

void PacketPrinter::printPacketLeftToRight(const Ptr<const PacketDissector::ProtocolDataUnit>& protocolDataUnit, Context& context) const
{
    auto protocol = protocolDataUnit->getProtocol();
    context.isCorrect &= protocolDataUnit->isCorrect();
    for (const auto& chunk : protocolDataUnit->getChunks()) {
        if (auto childLevel = dynamicPtrCast<const PacketDissector::ProtocolDataUnit>(chunk))
            printPacketLeftToRight(childLevel, context);
        else if (protocol == &Protocol::ieee80211Phy)
            printIeee80211PhyChunk(context.infoColumn, chunk);
        else if (protocol == &Protocol::ieee80211Mac)
            printIeee80211MacChunk(context.infoColumn, chunk);
        else if (protocol == &Protocol::ieee80211Mgmt)
            printIeee80211MgmtChunk(context.infoColumn, chunk);
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

void PacketPrinter::printIeee80211MacChunk(std::ostream& stream, const Ptr<const Chunk>& chunk) const
{
    using namespace ieee80211;
    if (auto macHeader = dynamicPtrCast<const inet::ieee80211::Ieee80211MacHeader>(chunk)) {
        stream << "WLAN ";
        switch (macHeader->getType()) {
            case ST_RTS: {
                stream << "RTS";
                break;
            }
            case ST_CTS:
                stream << "CTS";
                break;

            case ST_ACK:
                stream << "ACK";
                break;

            case ST_BLOCKACK_REQ:
                stream << "BlockAckReq";
                break;

            case ST_BLOCKACK:
                stream << "BlockAck";
                break;

            case ST_DATA:
            case ST_DATA_WITH_QOS:
                stream << "DATA";
                break;

            default:
                stream << "???";
                break;
        }
    }
}

void PacketPrinter::printIeee80211MgmtChunk(std::ostream& stream, const Ptr<const Chunk>& chunk) const
{
    stream << "(IEEE 802.11 Mgmt) " << chunk;
}

void PacketPrinter::printIeee80211PhyChunk(std::ostream& stream, const Ptr<const Chunk>& chunk) const
{
    stream << "(IEEE 802.11 Phy) " << chunk;
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
                stream << "ARP req: " << packet->getDestIpAddress()
                       << "=? (s=" << packet->getSrcIpAddress() << "(" << packet->getSrcMacAddress() << "))";
                break;

            case ARP_REPLY:
                stream << "ARP reply: "
                       << packet->getSrcIpAddress() << "=" << packet->getSrcMacAddress()
                       << " (d=" << packet->getDestIpAddress() << "(" << packet->getDestMacAddress() << "))";
                break;

            case ARP_RARP_REQUEST:
                stream << "RARP req: " << packet->getDestMacAddress()
                       << "=? (s=" << packet->getSrcIpAddress() << "(" << packet->getSrcMacAddress() << "))";
                break;

            case ARP_RARP_REPLY:
                stream << "RARP reply: "
                       << packet->getSrcMacAddress() << "=" << packet->getSrcIpAddress()
                       << " (d=" << packet->getDestIpAddress() << "(" << packet->getDestMacAddress() << "))";
                break;

            default:
                stream << "ARP op=" << packet->getOpcode() << ": d=" << packet->getDestIpAddress()
                       << "(" << packet->getDestMacAddress()
                       << ") s=" << packet->getSrcIpAddress()
                       << "(" << packet->getSrcMacAddress() << ")";
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
    stream << "(UNIMPLEMENTED " << protocol->getDescriptiveName() << ") " << chunk;
}

void PacketPrinter::printUnknownProtocolChunk(std::ostream& stream, const Ptr<const Chunk>& chunk) const
{
    if (auto byteCountChunk = dynamicPtrCast<const ByteCountChunk>(chunk))
        stream << byteCountChunk->getChunkLength();
    else if (auto bitCountChunk = dynamicPtrCast<const BitCountChunk>(chunk))
        stream << bitCountChunk->getChunkLength();
    else
        stream << "(UNKNOWN) " << chunk;
}

} // namespace


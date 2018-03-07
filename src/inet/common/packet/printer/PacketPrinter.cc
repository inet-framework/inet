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
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
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

const ProtocolPrinter& PacketPrinter::getProtocolPrinter(const Protocol *protocol) const
{
    auto protocolPrinter = ProtocolPrinterRegistry::globalRegistry.findProtocolPrinter(protocol);
    if (protocolPrinter == nullptr)
        protocolPrinter = ProtocolPrinterRegistry::globalRegistry.findProtocolPrinter(nullptr);
    return *protocolPrinter;
}

std::set<std::string> PacketPrinter::getSupportedTags() const
{
    return {"source_column", "destination_column", "protocol_column", "length_column", "info_column",
            "inside_out", "left_to_right"};
}

std::set<std::string> PacketPrinter::getDefaultEnabledTags() const
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

void PacketPrinter::printContext(std::ostream& stream, const Options *options, PacketPrinterContext& context) const
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
    PacketPrinterContext context;
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
    PacketPrinterContext context;
    printSignal(signal, options, context);
    printContext(stream, options, context);
}

void PacketPrinter::printSignal(inet::physicallayer::Signal *signal, const Options *options, PacketPrinterContext& context) const
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
    PacketPrinterContext context;
    printPacket(packet, options, context);
    printContext(stream, options, context);
}

void PacketPrinter::printPacket(Packet *packet, const Options *options, PacketPrinterContext& context) const
{
    PacketDissector::PduTreeBuilder pduTreeBuilder;
    auto packetProtocolTag = packet->findTag<PacketProtocolTag>();
    auto protocol = packetProtocolTag != nullptr ? packetProtocolTag->getProtocol() : nullptr;
    PacketDissector packetDissector(ProtocolDissectorRegistry::globalRegistry, pduTreeBuilder);
    packetDissector.dissectPacket(packet, protocol);
    auto& protocolDataUnit = pduTreeBuilder.getTopLevelPdu();
    context.lengthColumn << protocolDataUnit->getChunkLength();
    if (pduTreeBuilder.isSimplyEncapsulatedPacket())
        const_cast<PacketPrinter *>(this)->printPacketInsideOut(protocolDataUnit, options, context);
    else
        const_cast<PacketPrinter *>(this)->printPacketLeftToRight(protocolDataUnit, options, context);
}

void PacketPrinter::printPacketInsideOut(const Ptr<const PacketDissector::ProtocolDataUnit>& protocolDataUnit, const Options *options, PacketPrinterContext& context) const
{
    auto protocol = protocolDataUnit->getProtocol();
    context.isCorrect &= protocolDataUnit->isCorrect();
    for (const auto& chunk : protocolDataUnit->getChunks()) {
        if (auto childLevel = dynamicPtrCast<const PacketDissector::ProtocolDataUnit>(chunk))
            printPacketInsideOut(childLevel, options, context);
        else {
            auto& protocolPrinter = getProtocolPrinter(protocol);
            if (protocolDataUnit->getLevel() > context.infoLevel && protocol != nullptr)
                context.infoColumn.str("");
            else if (protocolDataUnit->getLevel() > 0)
                context.infoColumn << " | ";
            if (protocol == &Protocol::ethernetMac) {
                auto header = dynamicPtrCast<const EthernetMacHeader>(chunk);
                if (header != nullptr) {
                    context.sourceColumn << header->getSrc();
                    context.destinationColumn << header->getDest();
                }
                if (protocolDataUnit->getLevel() > context.infoLevel) {
                    // TODO: printEthernetChunk(context.infoColumn, chunk);
                }
            }
            else if (protocol == &Protocol::ieee80211Phy) {
                if (protocolDataUnit->getLevel() > context.infoLevel) {
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
                    printIeee80211MacChunk(context.infoColumn, chunk);
                }
            }
            else if (protocol == &Protocol::ieee80211Mgmt) {
                if (protocolDataUnit->getLevel() > context.infoLevel) {
                    printIeee80211MgmtChunk(context.infoColumn, chunk);
                }
            }
            else if (protocol == &Protocol::ieee8022) {
                if (protocolDataUnit->getLevel() > context.infoLevel) {
                    printIeee8022Chunk(context.infoColumn, chunk);
                }
            }
            else {
                if (protocolDataUnit->getLevel() > context.infoLevel)
                    protocolPrinter.print(chunk, protocol, options, context);
            }
            if (protocolDataUnit->getLevel() > context.infoLevel) {
                context.infoLevel = protocolDataUnit->getLevel();
                if (protocol != nullptr)
                    context.protocolColumn = protocol->getDescriptiveName();
            }
        }
    }
}

void PacketPrinter::printPacketLeftToRight(const Ptr<const PacketDissector::ProtocolDataUnit>& protocolDataUnit, const Options *options, PacketPrinterContext& context) const
{
    auto protocol = protocolDataUnit->getProtocol();
    context.isCorrect &= protocolDataUnit->isCorrect();
    for (const auto& chunk : protocolDataUnit->getChunks()) {
        if (auto childLevel = dynamicPtrCast<const PacketDissector::ProtocolDataUnit>(chunk))
            printPacketLeftToRight(childLevel, options, context);
        else
            getProtocolPrinter(protocol).print(chunk, protocol, options, context);
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

} // namespace


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
    return {"Show Source column", "Show Destination column", "Show Protocol column", "Show Length column", "Show Info column",
            "Print inside out", "Print left to right"};
}

std::set<std::string> PacketPrinter::getDefaultEnabledTags() const
{
    return {"Show Source column", "Show Destination column", "Show Protocol column", "Show Length column", "Show Info column", "Print inside out"};
}

std::vector<std::string> PacketPrinter::getColumnNames(const Options *options) const
{
    std::vector<std::string> columnNames;
    if (isEnabledOption(options, "Show Source column"))
        columnNames.push_back("Source");
    if (isEnabledOption(options, "Show Destination column"))
        columnNames.push_back("Destination");
    if (isEnabledOption(options, "Show Protocol column"))
        columnNames.push_back("Protocol");
    if (isEnabledOption(options, "Show Length column"))
        columnNames.push_back("Length");
    if (isEnabledOption(options, "Show Info column"))
        columnNames.push_back("Info");
    return columnNames;
}

void PacketPrinter::printContext(std::ostream& stream, const Options *options, PacketPrinterContext& context) const
{
    if (!context.isCorrect)
        stream << "\x1b[103m";
    stream << "\x1b[30m";
    if (isEnabledOption(options, "Show Source column"))
       stream << context.sourceColumn.str() << "\t";
    if (isEnabledOption(options, "Show Destination column"))
       stream << context.destinationColumn.str() << "\t";
    if (isEnabledOption(options, "Show Protocol column"))
       stream << "\x1b[34m" << context.protocolColumn << "\x1b[30m\t";
    if (isEnabledOption(options, "Show Length column"))
       stream << context.lengthColumn.str() << "\t";
    if (isEnabledOption(options, "Show Info column"))
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
            if (protocolDataUnit->getLevel() > context.infoLevel)
                protocolPrinter.print(chunk, protocol, options, context);
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

} // namespace


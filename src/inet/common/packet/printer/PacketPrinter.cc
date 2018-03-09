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
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/common/ProtocolTag_m.h"

namespace inet {

Register_MessagePrinter(PacketPrinter);

int PacketPrinter::getScoreFor(cMessage *msg) const
{
    return msg->isPacket() ? 100 : 0;
}

bool PacketPrinter::isEnabledOption(const Options *options, const char *name) const
{
    return options->enabledTags.find(name) != options->enabledTags.end();
}

static bool isPhysicalLayerProtocol(const Protocol *protocol)
{
    return protocol && protocol->getLayer() == Protocol::PhysicalLayer;
}

static bool isLinkLayerProtocol(const Protocol *protocol)
{
    return protocol && protocol->getLayer() == Protocol::LinkLayer;
}

static bool isNetworkLayerProtocol(const Protocol *protocol)
{
    return protocol && protocol->getLayer() == Protocol::NetworkLayer;
}

static bool isTransportLayerProtocol(const Protocol *protocol)
{
    return protocol && protocol->getLayer() == Protocol::TransportLayer;
}

bool PacketPrinter::isEnabledInfo(const Options *options, const Protocol *protocol) const
{
    return isEnabledOption(options, "Show all info") ||
          (isEnabledOption(options, "Show physical layer info") && isPhysicalLayerProtocol(protocol)) ||
          (isEnabledOption(options, "Show link layer info") && isLinkLayerProtocol(protocol)) ||
          (isEnabledOption(options, "Show network layer info") && isNetworkLayerProtocol(protocol)) ||
          (isEnabledOption(options, "Show transport layer info") && isTransportLayerProtocol(protocol));
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
    return {"Print inside out", "Print left to right",
            "Show 'Source' column", "Show 'Destination' column", "Show 'Protocol' column", "Show 'Length' column", "Show 'Info' column",
            "Show all PDU source fields", "Show all PDU destination fields", "Show all PDU protocols", "Show all PDU lengths",
            "Show physical layer info", "Show link layer info", "Show network layer info", "Show transport layer info", "Show all info", "Show innermost info",
            "Show auto source fields", "Show auto destination fields", "Show auto info"};
}

std::set<std::string> PacketPrinter::getDefaultEnabledTags() const
{
    return {"Print inside out",
            "Show 'Source' column", "Show 'Destination' column", "Show 'Protocol' column", "Show 'Length' column", "Show 'Info' column",
            "Show auto source fields", "Show auto destination fields", "Show auto info"};
}

std::vector<std::string> PacketPrinter::getColumnNames(const Options *options) const
{
    std::vector<std::string> columnNames;
    if (isEnabledOption(options, "Show 'Source' column"))
        columnNames.push_back("Source");
    if (isEnabledOption(options, "Show 'Destination' column"))
        columnNames.push_back("Destination");
    if (isEnabledOption(options, "Show 'Protocol' column"))
        columnNames.push_back("Protocol");
    if (isEnabledOption(options, "Show 'Length' column"))
        columnNames.push_back("Length");
    if (isEnabledOption(options, "Show 'Info' column"))
        columnNames.push_back("Info");
    return columnNames;
}

void PacketPrinter::printContext(std::ostream& stream, const Options *options, Context& context) const
{
    if (!context.isCorrect)
        stream << "\x1b[103m";
    stream << "\x1b[30m";
    if (isEnabledOption(options, "Show 'Source' column"))
       stream << context.sourceColumn.str() << "\t";
    if (isEnabledOption(options, "Show 'Destination' column"))
       stream << context.destinationColumn.str() << "\t";
    if (isEnabledOption(options, "Show 'Protocol' column"))
       stream << "\x1b[34m" << context.protocolColumn.str() << "\x1b[30m\t";
    if (isEnabledOption(options, "Show 'Length' column"))
       stream << context.lengthColumn.str() << "\t";
    if (isEnabledOption(options, "Show 'Info' column"))
       stream << context.infoColumn.str();
    stream << std::endl;
}

void PacketPrinter::printMessage(std::ostream& stream, cMessage *message) const
{
    Options options;
    options.enabledTags = getDefaultEnabledTags();
    printMessage(stream, message, &options);
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
    Options options;
    options.enabledTags = getDefaultEnabledTags();
    printSignal(stream, signal, &options);
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
    Options options;
    options.enabledTags = getDefaultEnabledTags();
    printPacket(stream, packet, &options);
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
    if (pduTreeBuilder.isSimplyEncapsulatedPacket() && isEnabledOption(options, "Print inside out"))
        const_cast<PacketPrinter *>(this)->printPacketInsideOut(protocolDataUnit, options, context);
    else
        const_cast<PacketPrinter *>(this)->printPacketLeftToRight(protocolDataUnit, options, context);
}

void PacketPrinter::printPacketInsideOut(const Ptr<const PacketDissector::ProtocolDataUnit>& protocolDataUnit, const Options *options, Context& context) const
{
    auto protocol = protocolDataUnit->getProtocol();
    context.isCorrect &= protocolDataUnit->isCorrect();
    printLengthColumn(protocolDataUnit, options, context);
    for (const auto& chunk : protocolDataUnit->getChunks()) {
        if (auto childLevel = dynamicPtrCast<const PacketDissector::ProtocolDataUnit>(chunk))
            printPacketInsideOut(childLevel, options, context);
        else {
            auto& protocolPrinter = getProtocolPrinter(protocol);
            ProtocolPrinter::Context protocolContext;
            protocolPrinter.print(chunk, protocol, options, protocolContext);
            if (protocolDataUnit->getLevel() > context.infoLevel) {
                context.infoLevel = protocolDataUnit->getLevel();
                printSourceColumn(protocolContext.sourceColumn.str(), protocol, options, context);
                printDestinationColumn(protocolContext.destinationColumn.str(), protocol, options, context);
                printProtocolColumn(protocol, options, context);
                // prepend info column
                bool showAutoInfo = isEnabledOption(options, "Show auto info");
                bool showInnermostInfo = isEnabledOption(options, "Show innermost info");
                if (showAutoInfo || showInnermostInfo || isEnabledInfo(options, protocol)) {
                    if (showInnermostInfo && !(showAutoInfo && protocol == nullptr))
                        context.infoColumn.str("");
                    if (context.infoColumn.str().length() != 0)
                        protocolContext.infoColumn << " | ";
                    context.infoColumn.str(protocolContext.infoColumn.str() + context.infoColumn.str());
                }
            }
        }
    }
}

void PacketPrinter::printPacketLeftToRight(const Ptr<const PacketDissector::ProtocolDataUnit>& protocolDataUnit, const Options *options, Context& context) const
{
    auto protocol = protocolDataUnit->getProtocol();
    context.isCorrect &= protocolDataUnit->isCorrect();
    printLengthColumn(protocolDataUnit, options, context);
    for (const auto& chunk : protocolDataUnit->getChunks()) {
        if (auto childLevel = dynamicPtrCast<const PacketDissector::ProtocolDataUnit>(chunk))
            printPacketLeftToRight(childLevel, options, context);
        else {
            auto& protocolPrinter = getProtocolPrinter(protocol);
            ProtocolPrinter::Context protocolContext;
            protocolPrinter.print(chunk, protocol, options, protocolContext);
            if (protocolDataUnit->getLevel() > context.infoLevel) {
                context.infoLevel = protocolDataUnit->getLevel();
                printSourceColumn(protocolContext.sourceColumn.str(), protocol, options, context);
                printDestinationColumn(protocolContext.destinationColumn.str(), protocol, options, context);
                printProtocolColumn(protocol, options, context);
            }
            // append info column
            if (isEnabledInfo(options, protocol)) {
                if (context.infoColumn.str().length() != 0)
                    context.infoColumn << " | ";
                context.infoColumn << protocolContext.infoColumn.str();
            }
        }
    }
}

void PacketPrinter::printSourceColumn(const std::string source, const Protocol *protocol, const Options *options, Context& context) const
{
    if (source.length() != 0) {
        bool concatenate = isEnabledOption(options, "Show all PDU source fields") ||
                          (isEnabledOption(options, "Show auto source fields") && !isNetworkLayerProtocol(protocol));
        if (!concatenate)
            context.sourceColumn.str("");
        else if (context.sourceColumn.str().length() != 0)
            context.sourceColumn << ":";
        context.sourceColumn << source;
    }
}

void PacketPrinter::printDestinationColumn(const std::string destination, const Protocol *protocol, const Options *options, Context& context) const
{
    if (destination.length() != 0) {
        bool concatenate = isEnabledOption(options, "Show all PDU destination fields") ||
                          (isEnabledOption(options, "Show auto destination fields") && !isNetworkLayerProtocol(protocol));
        if (!concatenate)
            context.destinationColumn.str("");
        else if (context.destinationColumn.str().length() != 0)
            context.destinationColumn << ":";
        context.destinationColumn << destination;
    }
}

void PacketPrinter::printProtocolColumn(const Protocol *protocol, const Options *options, Context& context) const
{
    if (protocol != nullptr) {
        if (!isEnabledOption(options, "Show all PDU protocols"))
            context.protocolColumn.str("");
        else if (context.protocolColumn.str().length() != 0)
            context.protocolColumn << ", ";
        context.protocolColumn << protocol->getDescriptiveName();
    }
}

void PacketPrinter::printLengthColumn(const Ptr<const PacketDissector::ProtocolDataUnit>& protocolDataUnit, const Options *options, Context& context) const
{
    auto lengthColumnLength = context.lengthColumn.str().length();
    if (lengthColumnLength == 0 || isEnabledOption(options, "Show all PDU lengths")) {
        if (lengthColumnLength != 0)
            context.lengthColumn << ", ";
        context.lengthColumn << protocolDataUnit->getChunkLength();
    }
}

} // namespace


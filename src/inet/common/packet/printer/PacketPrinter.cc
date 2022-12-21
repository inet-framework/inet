//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/printer/PacketPrinter.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/common/stlutils.h"

namespace inet {

Register_MessagePrinter(PacketPrinter);

std::string PacketPrinter::DirectiveResolver::resolveDirective(char directive) const
{
    switch (directive) {
        case 's':
            return context.sourceColumn.str();
        case 'd':
            return context.destinationColumn.str();
        case 'p':
            return context.protocolColumn.str();
        case 'l':
            return context.lengthColumn.str();
        case 't':
            return context.typeColumn.str();
        case 'i':
            return context.infoColumn.str();
        case 'n':
            return std::to_string(numPacket);
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
}

int PacketPrinter::getScoreFor(cMessage *msg) const
{
    return msg->isPacket() ? 100 : 0;
}

bool PacketPrinter::isEnabledOption(const Options *options, const char *name) const
{
    return contains(options->enabledTags, name);
}

bool PacketPrinter::isEnabledInfo(const Options *options, const Protocol *protocol) const
{
    if (isEnabledOption(options, "Show all info"))
        return true;
    if (protocol) {
        switch (protocol->getLayer()) {
            case Protocol::PhysicalLayer: return isEnabledOption(options, "Show physical layer info");
            case Protocol::LinkLayer: return isEnabledOption(options, "Show link layer info");
            case Protocol::NetworkLayer: return isEnabledOption(options, "Show network layer info");
            case Protocol::TransportLayer: return isEnabledOption(options, "Show transport layer info");
            default: break;
        }
    }
    return false;
}

const ProtocolPrinter& PacketPrinter::getProtocolPrinter(const Protocol *protocol) const
{
    auto protocolPrinter = ProtocolPrinterRegistry::getInstance().findProtocolPrinter(protocol);
    if (protocolPrinter == nullptr)
        protocolPrinter = ProtocolPrinterRegistry::getInstance().findProtocolPrinter(nullptr);
    return *protocolPrinter;
}

std::set<std::string> PacketPrinter::getSupportedTags() const
{
    return { "Print inside out", "Print left to right",
             "Show 'Source' column", "Show 'Destination' column", "Show 'Protocol' column", "Show 'Type' column", "Show 'Length' column", "Show 'Info' column",
             "Show all PDU source fields", "Show all PDU destination fields", "Show all PDU protocols", "Show all PDU lengths",
             "Show physical layer info", "Show link layer info", "Show network layer info", "Show transport layer info", "Show all info", "Show innermost info",
             "Show auto source fields", "Show auto destination fields", "Show auto info" };
}

std::set<std::string> PacketPrinter::getDefaultEnabledTags() const
{
    return { "Print inside out",
             "Show 'Source' column", "Show 'Destination' column", "Show 'Protocol' column", "Show 'Type' column", "Show 'Length' column", "Show 'Info' column",
             "Show auto source fields", "Show auto destination fields", "Show auto info" };
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
    if (isEnabledOption(options, "Show 'Type' column"))
        columnNames.push_back("Type");
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
    if (isEnabledOption(options, "Show 'Type' column"))
        stream << "\x1b[34m" << context.typeColumn.str() << "\x1b[30m\t";
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
        if (false) ;
#ifdef INET_WITH_PHYSICALLAYERWIRELESSCOMMON
        else if (auto signal = dynamic_cast<physicallayer::Signal *>(cpacket))
            printSignal(signal, options, context);
#endif // INET_WITH_PHYSICALLAYERWIRELESSCOMMON
        else if (auto packet = dynamic_cast<Packet *>(cpacket))
            printPacket(packet, options, context);
        else
            context.infoColumn << cpacket->str();
    }
    printContext(stream, options, context);
}

#ifdef INET_WITH_PHYSICALLAYERWIRELESSCOMMON
void PacketPrinter::printSignal(std::ostream& stream, physicallayer::Signal *signal) const
{
    Options options;
    options.enabledTags = getDefaultEnabledTags();
    printSignal(stream, signal, &options);
}

void PacketPrinter::printSignal(std::ostream& stream, physicallayer::Signal *signal, const Options *options) const
{
    Context context;
    printSignal(signal, options, context);
    printContext(stream, options, context);
}

void PacketPrinter::printSignal(physicallayer::Signal *signal, const Options *options, Context& context) const
{
    context.infoColumn << signal->str();
}
#endif // INET_WITH_PHYSICALLAYERWIRELESSCOMMON

void PacketPrinter::printPacket(std::ostream& stream, Packet *packet, const char *format) const
{
    Options options;
    options.enabledTags = getDefaultEnabledTags();
    printPacket(stream, packet, &options, format);
}

void PacketPrinter::printPacket(std::ostream& stream, Packet *packet, const Options *options, const char *format) const
{
    Context context;
    printPacket(packet, options, context);
    if (format == nullptr)
        printContext(stream, options, context);
    else {
        DirectiveResolver directiveResolver(context, numPacket++);
        StringFormat stringFormat;
        stringFormat.parseFormat(format);
        stream << stringFormat.formatString(&directiveResolver);
    }
}

void PacketPrinter::printPacket(Packet *packet, const Options *options, Context& context) const
{
    PacketDissector::PduTreeBuilder pduTreeBuilder;
    try {
        PacketDissector packetDissector(ProtocolDissectorRegistry::getInstance(), pduTreeBuilder);
        packetDissector.dissectPacket(packet);
    }
    catch (cRuntimeError& e) {
        // NOTE: don't propagate errors from printPacket, becaue it can break Qtenv for example
        context.infoColumn << e.what() << ": ";
    }
    const auto& protocolDataUnit = pduTreeBuilder.getTopLevelPdu();
    if (protocolDataUnit != nullptr) {
        if (pduTreeBuilder.isSimplyEncapsulatedPacket() && isEnabledOption(options, "Print inside out"))
            const_cast<PacketPrinter *>(this)->printPacketInsideOut(protocolDataUnit, options, context);
        else
            const_cast<PacketPrinter *>(this)->printPacketLeftToRight(protocolDataUnit, options, context);
    }
}

std::string PacketPrinter::printPacketToString(Packet *packet, const char *format) const
{
    std::stringstream stream;
    printPacket(stream, packet, format);
    return stream.str();
}

std::string PacketPrinter::printPacketToString(Packet *packet, const Options *options, const char *format) const
{
    std::stringstream stream;
    printPacket(stream, packet, options, format);
    return stream.str();
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
                if (protocol != &Protocol::unknown)
                    printProtocolColumn(protocol, options, context);
                // prepend info column
                bool showAutoInfo = isEnabledOption(options, "Show auto info");
                bool showInnermostInfo = isEnabledOption(options, "Show innermost info");
                if (showAutoInfo || showInnermostInfo || isEnabledInfo(options, protocol)) {
                    if (showInnermostInfo && !(showAutoInfo && protocol == nullptr)) {
                        context.typeColumn.str("");
                        context.infoColumn.str("");
                    }
                    if (protocolContext.typeColumn.str().length() != 0) {
                        if (context.typeColumn.str().length() != 0)
                            protocolContext.typeColumn << " | ";
                        context.typeColumn.str(protocolContext.typeColumn.str() + context.typeColumn.str());
                    }
                    if (protocolContext.infoColumn.str().length() != 0) {
                        if (context.infoColumn.str().length() != 0)
                            protocolContext.infoColumn << " | ";
                        context.infoColumn.str(protocolContext.infoColumn.str() + context.infoColumn.str());
                    }
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
                if (protocol != &Protocol::unknown)
                    printProtocolColumn(protocol, options, context);
            }
            // append info column
            if (isEnabledInfo(options, protocol)) {
                if (context.typeColumn.str().length() != 0)
                    context.typeColumn << " | ";
                context.typeColumn << protocolContext.typeColumn.str();
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
                          (isEnabledOption(options, "Show auto source fields") && !(protocol && protocol->getLayer() == Protocol::NetworkLayer));
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
                          (isEnabledOption(options, "Show auto destination fields") && !(protocol && protocol->getLayer() == Protocol::NetworkLayer));
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


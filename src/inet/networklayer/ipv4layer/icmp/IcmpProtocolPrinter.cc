//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/ipv4/IcmpProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/networklayer/ipv4/IcmpHeader_m.h"

namespace inet {

Register_Protocol_Printer(&Protocol::icmpv4, IcmpProtocolPrinter);

void IcmpProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto header = dynamicPtrCast<const IcmpHeader>(chunk)) {
        switch (header->getType()) {
            case ICMP_DESTINATION_UNREACHABLE:
                context.typeColumn << "DEST-UN";
                // TODO packet contains a complete Ipv4Header and the first 8 bytes of transport header (or icmp). (protocol specified in Ipv4Header.)
                context.infoColumn << "code=" << header->getCode();
                break;
            case ICMP_TIME_EXCEEDED:
                context.typeColumn << "TIME-EX";
                // TODO packet contains a complete Ipv4Header and the first 8 bytes of transport header (or icmp). (protocol specified in Ipv4Header.)
                context.infoColumn << "code=" << header->getCode();
                break;
            case ICMP_PARAMETER_PROBLEM:
                context.typeColumn << "PAR-PROB";
                // TODO packet contains a complete Ipv4Header and the first 8 bytes of transport header (or icmp). (protocol specified in Ipv4Header.)
                context.infoColumn << "code=" << header->getCode();
                break;
            case ICMP_ECHO_REQUEST:
                context.typeColumn << "ECHO-REQ";
                context.infoColumn << "code=" << header->getCode();
                if (auto echoHeader = dynamicPtrCast<const IcmpEchoRequest>(header))
                    context.infoColumn << " id=" << echoHeader->getIdentifier() << " seq=" << echoHeader->getSeqNumber();
                break;
            case ICMP_ECHO_REPLY:
                context.typeColumn << "ECHO-REPLY";
                context.infoColumn << "code=" << header->getCode();
                if (auto echoHeader = dynamicPtrCast<const IcmpEchoReply>(header))
                    context.infoColumn << " id=" << echoHeader->getIdentifier() << " seq=" << echoHeader->getSeqNumber();
                break;
            default:
                context.infoColumn << "ICMP-" << header->getType() << "-" << header->getCode();
                break;
        }
    }
    else
        context.infoColumn << "(ICMP) " << chunk;
}

} // namespace inet


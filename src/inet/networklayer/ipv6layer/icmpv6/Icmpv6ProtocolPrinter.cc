//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/icmpv6/Icmpv6ProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/networklayer/icmpv6/Icmpv6Header_m.h"

namespace inet {

Register_Protocol_Printer(&Protocol::icmpv6, Icmpv6ProtocolPrinter);

void Icmpv6ProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto header = dynamicPtrCast<const Icmpv6Header>(chunk)) {
        switch (header->getType()) {
            case ICMPv6_UNSPECIFIED:
                context.infoColumn << "ICMPv6-UNSPEC";
                break;
            case ICMPv6_DESTINATION_UNREACHABLE: {
                auto header2 = CHK(dynamicPtrCast<const Icmpv6DestUnreachableMsg>(header));
                // TODO packet contains a complete Ipv4Header and the first 8 bytes of transport header (or icmp). (protocol specified in Ipv4Header.)
                context.infoColumn << "ICMPv6-DEST-UN code=" << header2->getCode();
                break;
            }
            case ICMPv6_PACKET_TOO_BIG:
                context.infoColumn << "ICMPv6-PK-TOO-BIG";
                break;
            case ICMPv6_TIME_EXCEEDED:
                context.infoColumn << "ICMPv6-TIME-EXCEEDED";
                break;
            case ICMPv6_PARAMETER_PROBLEM:
                context.infoColumn << "ICMPv6-PARAM-PROBLEM";
                break;
            case ICMPv6_ECHO_REQUEST: {
                auto echoHeader = CHK(dynamicPtrCast<const Icmpv6EchoRequestMsg>(header));
                context.infoColumn << "ICMPv6-ECHO-REQ code=" << echoHeader->getCode()
                                   << " id=" << echoHeader->getIdentifier() << " seq=" << echoHeader->getSeqNumber();
                break;
            }
            case ICMPv6_ECHO_REPLY: {
                auto echoHeader = CHK(dynamicPtrCast<const Icmpv6EchoReplyMsg>(header));
                context.infoColumn << "ICMPv6-ECHO-REPLY code=" << echoHeader->getCode()
                                   << " id=" << echoHeader->getIdentifier() << " seq=" << echoHeader->getSeqNumber();
                break;
            }
            case ICMPv6_MLD_QUERY:
                context.infoColumn << "ICMPv6-MLD-QRY";
                // TODO
                break;
            case ICMPv6_MLD_REPORT:
                context.infoColumn << "ICMPv6-MLD-REPORT";
                // TODO
                break;
            case ICMPv6_MLD_DONE:
                context.infoColumn << "ICMPv6-MLD-DONE";
                // TODO
                break;
            case ICMPv6_ROUTER_SOL:
                context.infoColumn << "ICMPv6-ROUTER-SOL";
                // TODO
                break;
            case ICMPv6_ROUTER_AD:
                context.infoColumn << "ICMPv6-ROUTER-AD";
                // TODO
                break;
            case ICMPv6_NEIGHBOUR_SOL:
                context.infoColumn << "ICMPv6-NEIGHBOUR-SOL";
                // TODO
                break;
            case ICMPv6_NEIGHBOUR_AD:
                context.infoColumn << "ICMPv6-NEIGHBOUR-AD";
                // TODO
                break;
            case ICMPv6_REDIRECT:
                context.infoColumn << "ICMPv6-NEIGHBOUR-REDIR";
                break;
            case ICMPv6_MLDv2_REPORT:
                context.infoColumn << "ICMPv6-MLDv2-REPORT";
                // TODO
                break;
            case ICMPv6_EXPERIMENTAL_MOBILITY:
                context.infoColumn << "ICMPv6-EXPERIMENTAL-MOBILITY";
                // TODO
                break;
        }
    }
    else
        context.infoColumn << "(ICMPv6) " << chunk;
}

} // namespace inet


//
// Copyright (C) 2018 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// @author: Zoltan Bojthe
//

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/networklayer/ipv4/IcmpHeader_m.h"
#include "inet/networklayer/ipv4/IcmpProtocolPrinter.h"

namespace inet {

Register_Protocol_Printer(&Protocol::icmpv4, IcmpProtocolPrinter);

void IcmpProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto header = dynamicPtrCast<const IcmpHeader>(chunk)) {
        switch (header->getType()) {
            case ICMP_DESTINATION_UNREACHABLE:
                context.typeColumn << "DEST-UN";
                //TODO packet contains a complete Ipv4Header and the first 8 bytes of transport header (or icmp). (protocol specified in Ipv4Header.)
                context.infoColumn << "code=" << header->getCode();
                break;
            case ICMP_TIME_EXCEEDED:
                context.typeColumn << "TIME-EX";
                //TODO packet contains a complete Ipv4Header and the first 8 bytes of transport header (or icmp). (protocol specified in Ipv4Header.)
                context.infoColumn << "code=" << header->getCode();
                break;
            case ICMP_PARAMETER_PROBLEM:
                context.typeColumn << "PAR-PROB";
                //TODO packet contains a complete Ipv4Header and the first 8 bytes of transport header (or icmp). (protocol specified in Ipv4Header.)
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


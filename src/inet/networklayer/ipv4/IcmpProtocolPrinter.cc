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
                //TODO packet contains a complete Ipv4Header and the first 8 bytes of transport header (or icmp). (protocol specified in Ipv4Header.)
                context.infoColumn << "ICMP-DEST-UN code=" << header->getCode();
                break;
            case ICMP_TIME_EXCEEDED:
                //TODO packet contains a complete Ipv4Header and the first 8 bytes of transport header (or icmp). (protocol specified in Ipv4Header.)
                context.infoColumn << "ICMP-TIME-EX code=" << header->getCode();
                break;
            case ICMP_PARAMETER_PROBLEM:
                //TODO packet contains a complete Ipv4Header and the first 8 bytes of transport header (or icmp). (protocol specified in Ipv4Header.)
                context.infoColumn << "ICMP-PAR-PROB code=" << header->getCode();
                break;
            case ICMP_ECHO_REQUEST:
                context.infoColumn << "ICMP-ECHO-REQ code=" << header->getCode();
                if (auto echoHeader = dynamicPtrCast<const IcmpEchoRequest>(header))
                    context.infoColumn << " id=" << echoHeader->getIdentifier() << " seq=" << echoHeader->getSeqNumber();
                break;
            case ICMP_ECHO_REPLY:
                context.infoColumn << "ICMP-ECHO-REPLY code=" << header->getCode();
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


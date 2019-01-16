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
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4ProtocolPrinter.h"

namespace inet {

Register_Protocol_Printer(&Protocol::ipv4, Ipv4ProtocolPrinter);

void Ipv4ProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto header = dynamicPtrCast<const Ipv4Header>(chunk)) {
        context.sourceColumn << header->getSrcAddress();
        context.destinationColumn << header->getDestAddress();
        B payloadLength = B(header->getTotalLengthField()) - header->getChunkLength();
        context.infoColumn << "IPv4"
                << " id:" << header->getIdentification()
                << " ttl:" << header->getTimeToLive();
        if (header->getDontFragment())
            context.infoColumn << " DontFrag";
        if (header->getMoreFragments() || header->getFragmentOffset() > 0) {
            context.infoColumn << " frag:[" << header->getFragmentOffset() << ".." << header->getFragmentOffset() + payloadLength.get();
            if (!header->getMoreFragments())
                context.infoColumn << " End";
            context.infoColumn << ")";
        }
        auto payloadProtocol = header->getProtocol();
        if (payloadProtocol)
            context.infoColumn << " payload:" << payloadProtocol->getName() << " " << payloadLength;
        else
            context.infoColumn << " payload: protocol(" << header->getProtocolId() << ") " << payloadLength;
        //TODO options
    }
    else
        context.infoColumn << "(IPv4) " << chunk;
}

} // namespace inet


//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/udp/UdpProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

namespace inet {

Register_Protocol_Printer(&Protocol::udp, UdpProtocolPrinter);

void UdpProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto header = dynamicPtrCast<const UdpHeader>(chunk)) {
        context.sourceColumn << header->getSrcPort();
        context.destinationColumn << header->getDestPort();
        context.infoColumn << header->getSrcPort() << "->" << header->getDestPort() << ", payload:" << (B(header->getTotalLengthField()) - header->getChunkLength());
    }
    else
        context.infoColumn << "(UDP) " << chunk;
}

} // namespace inet


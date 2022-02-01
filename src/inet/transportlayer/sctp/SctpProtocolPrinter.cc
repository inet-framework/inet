//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/sctp/SctpProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/transportlayer/sctp/SctpHeader.h"

namespace inet {
namespace sctp {

Register_Protocol_Printer(&Protocol::sctp, SctpProtocolPrinter);

void SctpProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto header = dynamicPtrCast<const SctpHeader>(chunk)) {
        context.sourceColumn << header->getSrcPort();
        context.destinationColumn << header->getDestPort();
        context.infoColumn << header->getSrcPort() << "->" << header->getDestPort();
//                           << ", payload:" << (B(header->getTotalLengthField()) - header->getChunkLength()); // TODO
    }
    else
        context.infoColumn << "(UDP) " << chunk;
}

} // namespace sctp
} // namespace inet


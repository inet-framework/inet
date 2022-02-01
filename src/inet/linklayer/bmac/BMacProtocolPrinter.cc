//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/bmac/BMacProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/linklayer/bmac/BMacHeader_m.h"

namespace inet {

Register_Protocol_Printer(&Protocol::bmac, BMacProtocolPrinter);

void BMacProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto header = dynamicPtrCast<const BMacHeaderBase>(chunk)) {
        context.sourceColumn << header->getSrcAddr();
        context.destinationColumn << header->getDestAddr();
        context.infoColumn << "(Acking MAC) " << chunk; // TODO
    }
    else
        context.infoColumn << "(Acking MAC) " << chunk;
}

} // namespace inet


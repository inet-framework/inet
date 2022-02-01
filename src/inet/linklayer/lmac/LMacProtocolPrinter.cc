//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/lmac/LMacProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/linklayer/lmac/LMacHeader_m.h"

namespace inet {

Register_Protocol_Printer(&Protocol::lmac, LMacProtocolPrinter);

void LMacProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto header = dynamicPtrCast<const LMacHeaderBase>(chunk)) {
        context.sourceColumn << header->getSrcAddr();
        context.destinationColumn << header->getDestAddr();
        context.infoColumn << "L-MAC : " << header; // TODO
    }
    else
        context.infoColumn << "(L-MAC) " << chunk;
}

} // namespace inet


//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/acking/AckingMacProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/linklayer/acking/AckingMacHeader_m.h"

namespace inet {

Register_Protocol_Printer(&Protocol::ackingMac, AckingMacProtocolPrinter);

void AckingMacProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto header = dynamicPtrCast<const AckingMacHeader>(chunk)) {
        context.sourceColumn << header->getSrc();
        context.destinationColumn << header->getDest();
        context.infoColumn << "(Acking MAC) " << chunk; // TODO
    }
    else
        context.infoColumn << "(Acking MAC) " << chunk;
}

} // namespace inet


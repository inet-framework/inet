//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/common/EthernetMacProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"

namespace inet {

Register_Protocol_Printer(&Protocol::ethernetMac, EthernetMacProtocolPrinter);

void EthernetMacProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto header = dynamicPtrCast<const EthernetMacHeader>(chunk)) {
        context.sourceColumn << header->getSrc();
        context.destinationColumn << header->getDest();
        context.infoColumn << "(Eth) " << chunk; // TODO
    }
    else
        context.infoColumn << "(Eth) " << chunk;
}

} // namespace inet


//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/xmac/XMacProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/linklayer/xmac/XMacHeader_m.h"

namespace inet {

Register_Protocol_Printer(&Protocol::xmac, XMacProtocolPrinter);

void XMacProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto header = dynamicPtrCast<const XMacHeaderBase>(chunk)) {
        context.sourceColumn << header->getSrcAddr();
        context.destinationColumn << header->getDestAddr();
        context.infoColumn << "XMAC type:" << header->getType() << " " << header; // TODO
    }
    else
        context.infoColumn << "(XMAC) " << chunk;
}

} // namespace inet


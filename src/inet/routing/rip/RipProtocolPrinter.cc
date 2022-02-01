//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/routing/rip/RipProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/routing/rip/RipPacket_m.h"

namespace inet {

Register_Protocol_Printer(&Protocol::rip, RipProtocolPrinter);

void RipProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto packet = dynamicPtrCast<const RipPacket>(chunk)) {
        context.infoColumn << "RIP: ";
        switch (packet->getCommand()) {
            case RIP_REQUEST:
                context.infoColumn << "req ";
                break;

            case RIP_RESPONSE:
                context.infoColumn << "resp ";
                break;

            default:
                context.infoColumn << "unknown ";
                break;
        }
        unsigned int size = packet->getEntryArraySize();
        for (unsigned int i = 0; i < size; ++i) {
            const RipEntry& entry = packet->getEntry(i);
            if (i > 0)
                context.infoColumn << "; ";
            if (i > 2) {
                context.infoColumn << "...(" << size << " entries)";
                break;
            }
            context.infoColumn << entry.address << "/" << entry.prefixLength;
            if (!entry.nextHop.isUnspecified())
                context.infoColumn << "->" << entry.nextHop;
            if (entry.metric == 16)
                context.infoColumn << " unroutable";
            else
                context.infoColumn << " m=" << entry.metric;
        }
    }
    else
        context.infoColumn << "(RIP) " << chunk;
}

} // namespace inet


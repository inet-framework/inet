//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/common/EchoProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/networklayer/common/EchoPacket_m.h"

namespace inet {

Register_Protocol_Printer(&Protocol::echo, EchoProtocolPrinter);

void EchoProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto header = dynamicPtrCast<const EchoPacket>(chunk)) {
        context.infoColumn << "Echo";
        switch (header->getType()) {
            case ECHO_PROTOCOL_REQUEST:
                context.infoColumn << " request";
                break;
            case ECHO_PROTOCOL_REPLY:
                context.infoColumn << " reply";
                break;
            default:
                context.infoColumn << " type=" << header->getType();
                break;
        }
        context.infoColumn << " id:" << header->getIdentifier() << " seq:" << header->getSeqNumber();
    }
    else
        context.infoColumn << "(Echo) " << chunk;
}

} // namespace inet


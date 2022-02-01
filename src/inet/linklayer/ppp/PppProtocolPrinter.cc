//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ppp/PppProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/linklayer/ppp/PppFrame_m.h"

namespace inet {

Register_Protocol_Printer(&Protocol::ppp, PppProtocolPrinter);

void PppProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto header = dynamicPtrCast<const PppHeader>(chunk)) {
        context.infoColumn << "(PPP) " << chunk; // TODO
    }
    else
        context.infoColumn << "(PPP) " << chunk;
}

} // namespace inet


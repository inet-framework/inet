//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/csmaca/CsmaCaMacProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/linklayer/csmaca/CsmaCaMacHeader_m.h"

namespace inet {

Register_Protocol_Printer(&Protocol::csmaCaMac, CsmaCaMacProtocolPrinter);

void CsmaCaMacProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto header = dynamicPtrCast<const CsmaCaMacHeader>(chunk)) {
        context.sourceColumn << header->getTransmitterAddress();
        context.destinationColumn << header->getReceiverAddress();
        context.infoColumn << "(CSMA/CA MAC) " << chunk; // TODO
    }
    else
        context.infoColumn << "(CSMA/CA MAC) " << chunk;
}

} // namespace inet


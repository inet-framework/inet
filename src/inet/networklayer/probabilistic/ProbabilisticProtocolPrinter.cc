//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/probabilistic/ProbabilisticProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/networklayer/probabilistic/ProbabilisticBroadcastHeader_m.h"

namespace inet {

Register_Protocol_Printer(&Protocol::probabilistic, ProbabilisticProtocolPrinter);

void ProbabilisticProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto header = dynamicPtrCast<const ProbabilisticBroadcastHeader>(chunk)) {
        context.sourceColumn << header->getSourceAddress();
        context.destinationColumn << header->getDestinationAddress();
        B payloadLength = header->getPayloadLengthField();
        context.infoColumn << "Probabilistic";
        auto payloadProtocol = header->getProtocol();
        if (payloadProtocol)
            context.infoColumn << " payload:" << payloadProtocol->getName() << " " << payloadLength;
        else
            context.infoColumn << " payload: protocol(" << header->getProtocolId() << ") " << payloadLength;
    }
    else
        context.infoColumn << "(Probabilistic) " << chunk;
}

} // namespace inet


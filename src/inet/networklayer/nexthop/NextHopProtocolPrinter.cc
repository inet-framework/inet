//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/nexthop/NextHopProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/networklayer/nexthop/NextHopForwardingHeader_m.h"

namespace inet {

Register_Protocol_Printer(&Protocol::nextHopForwarding, NextHopProtocolPrinter);

void NextHopProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto header = dynamicPtrCast<const NextHopForwardingHeader>(chunk)) {
        context.sourceColumn << header->getSourceAddress();
        context.destinationColumn << header->getDestinationAddress();
        B payloadLength = header->getPayloadLengthField();
        context.infoColumn << "Gnp"
                           << " nbHops:" << header->getHopLimit();
        auto payloadProtocol = header->getProtocol();
        if (payloadProtocol)
            context.infoColumn << " payload:" << payloadProtocol->getName() << " " << payloadLength;
        else
            context.infoColumn << " payload: protocol(" << header->getProtocolId() << ") " << payloadLength;
    }
    else
        context.infoColumn << "(Gnp) " << chunk;
}

} // namespace inet


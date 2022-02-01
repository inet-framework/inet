//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/wiseroute/WiseRouteProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/networklayer/wiseroute/WiseRouteHeader_m.h"

namespace inet {

Register_Protocol_Printer(&Protocol::wiseRoute, WiseRouteProtocolPrinter);

void WiseRouteProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto header = dynamicPtrCast<const WiseRouteHeader>(chunk)) {
        context.sourceColumn << header->getSourceAddress();
        context.destinationColumn << header->getDestinationAddress();
        B payloadLength = header->getPayloadLengthField();
        context.infoColumn << "WiseRoute"
                           << " ttl:" << header->getNbHops();
        auto payloadProtocol = header->getProtocol();
        if (payloadProtocol)
            context.infoColumn << " payload:" << payloadProtocol->getName() << " " << payloadLength;
        else
            context.infoColumn << " payload: protocol(" << header->getProtocolId() << ") " << payloadLength;
    }
    else
        context.infoColumn << "(WiseRoute) " << chunk;
}

} // namespace inet


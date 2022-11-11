//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/ipv6/Ipv6ProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/networklayer/ipv6/Ipv6ExtensionHeaders_m.h"
#include "inet/networklayer/ipv6/Ipv6Header_m.h"

namespace inet {

Register_Protocol_Printer(&Protocol::ipv6, Ipv6ProtocolPrinter);

void Ipv6ProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (Ptr<const Ipv6Header> header = dynamicPtrCast<const Ipv6Header>(chunk)) {
        const Ipv6FragmentHeader *fh = dynamic_cast<const Ipv6FragmentHeader *>(header->findExtensionHeaderByType(IP_PROT_IPv6EXT_FRAGMENT));
        B payloadLength = B(header->getPayloadLength());
        context.sourceColumn << header->getSrcAddress();
        context.destinationColumn << header->getDestAddress();
        context.infoColumn << "IPv6"
                           << " ttl:" << header->getHopLimit();
        if (fh && (fh->getMoreFragments() || fh->getFragmentOffset() > 0)) {
            context.infoColumn << " id:" << fh->getIdentification()
                               << " frag:[" << fh->getFragmentOffset() << ".." << fh->getFragmentOffset() + payloadLength.get();
            if (!fh->getMoreFragments())
                context.infoColumn << " End";
            context.infoColumn << ")";
        }
        auto payloadProtocol = header->getProtocol();
        if (payloadProtocol)
            context.infoColumn << " payload:" << payloadProtocol->getName() << " " << payloadLength;
        else
            context.infoColumn << " payload: protocol(" << header->getProtocolId() << ") " << payloadLength;
        // TODO options
    }
    else
        context.infoColumn << "(IPv6) " << chunk;
}

} // namespace inet


// SPDX-License-Identifier: LGPL-3.0-or-later

#include "inet/routing/aodv/AodvSubnetProtocolPrinter.h"

#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/routing/aodv/AodvControlPackets_m.h"
#include "inet/routing/aodv/AodvSubnetProtocol.h"

namespace inet {

Register_Protocol_Printer(&aodv::aodvSubnetProtocol, AodvSubnetProtocolPrinter);

void AodvSubnetProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    auto announcement = dynamicPtrCast<const aodv::AodvSubnetAnnouncement>(chunk);
    if (!announcement) {
        context.infoColumn << "AODV subnet: " << chunk;
        return;
    }
    context.sourceColumn << announcement->getCandidateAddress();
    context.typeColumn << "AODV subnet";
    context.infoColumn << announcement->getPrefix() << "/" << unsigned(announcement->getPrefixLength())
        << (announcement->getEligible() ? " eligible" : " unavailable");
    if (announcement->getSequenceValid())
        context.infoColumn << " seq=" << announcement->getSequenceNumber();
    else
        context.infoColumn << " seq=unknown";
}

} // namespace inet

//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/arp/ipv4/ArpProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/networklayer/arp/ipv4/ArpPacket_m.h"

namespace inet {

Register_Protocol_Printer(&Protocol::arp, ArpProtocolPrinter);

void ArpProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto arpPacket = dynamicPtrCast<const ArpPacket>(chunk)) {
        context.infoColumn << arpPacket->str();
        switch (arpPacket->getOpcode()) {
            case ARP_REQUEST:
            case ARP_RARP_REQUEST:
                context.typeColumn << "REQUEST";
                break;
            case ARP_REPLY:
            case ARP_RARP_REPLY:
                context.typeColumn << "REPLY";
                break;
        }
    }
    else
        context.infoColumn << "(ARP) " << chunk;
}

} // namespace inet


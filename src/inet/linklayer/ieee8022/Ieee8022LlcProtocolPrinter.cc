//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8022/Ieee8022LlcProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcHeader_m.h"
#include "inet/linklayer/ieee8022/Ieee8022SnapHeader_m.h"

namespace inet {

Register_Protocol_Printer(&Protocol::ieee8022llc, Ieee8022LlcProtocolPrinter);

void Ieee8022LlcProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto header = dynamicPtrCast<const Ieee8022LlcHeader>(chunk)) {
        context.infoColumn << "ssap:" << header->getSsap() << " dsap:" << header->getDsap() << " ctrl:" << header->getControl();
        if (auto snapHeader = dynamicPtrCast<const Ieee8022LlcSnapHeader>(chunk))
            context.infoColumn << "oui:" << snapHeader->getOui() << " protocol:" << snapHeader->getProtocolId();
    }
    else
        context.infoColumn << "(IEEE 802.2 Llc) " << chunk;
}

} // namespace inet


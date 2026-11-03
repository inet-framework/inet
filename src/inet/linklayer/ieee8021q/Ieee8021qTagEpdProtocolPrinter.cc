//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021q/Ieee8021qTagEpdProtocolPrinter.h"

#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/linklayer/ieee8021q/Ieee8021qTagHeader_m.h"

namespace inet {

Register_Protocol_Printer(&Protocol::ieee8021qCTag, Ieee8021qTagEpdProtocolPrinter);
Register_Protocol_Printer(&Protocol::ieee8021qSTag, Ieee8021qTagEpdProtocolPrinter);

void Ieee8021qTagEpdProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto header = dynamicPtrCast<const Ieee8021qTagEpdHeader>(chunk))
        context.infoColumn << (protocol == &Protocol::ieee8021qSTag ? "S-tag" : "C-tag") << " pcp = " << (int)header->getPcp() << ", dei = " << (header->getDei() ? "true" : "false") << ", vid = " << header->getVid();
    else
        context.infoColumn << "(" << protocol->getDescriptiveName() << ") " << chunk;
}

} // namespace inet


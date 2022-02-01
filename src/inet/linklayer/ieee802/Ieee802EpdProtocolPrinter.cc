//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/linklayer/ieee802/Ieee802EpdProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/linklayer/ieee802/Ieee802EpdHeader_m.h"

namespace inet {

Register_Protocol_Printer(&Protocol::ieee802epd, Ieee802EpdProtocolPrinter);

void Ieee802EpdProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto header = dynamicPtrCast<const Ieee802EpdHeader>(chunk))
        context.infoColumn << "etherType: " << header->getEtherType();
    else
        context.infoColumn << "(IEEE 802 EPD)" << chunk;
}

} // namespace inet


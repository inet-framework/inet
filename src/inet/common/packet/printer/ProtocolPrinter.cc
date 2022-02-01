//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/printer/ProtocolPrinter.h"

#include "inet/common/packet/chunk/BitCountChunk.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"

namespace inet {

Register_Protocol_Printer(nullptr, DefaultProtocolPrinter);

void DefaultProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (protocol == nullptr)
        context.infoColumn << "(UNKNOWN) ";
    else
        context.infoColumn << "(UNIMPLEMENTED " << protocol->getDescriptiveName() << ") ";
    if (auto byteCountChunk = dynamicPtrCast<const ByteCountChunk>(chunk))
        context.infoColumn << byteCountChunk->getChunkLength();
    else if (auto bitCountChunk = dynamicPtrCast<const BitCountChunk>(chunk))
        context.infoColumn << bitCountChunk->getChunkLength();
    else
        context.infoColumn << chunk;
}

} // namespace


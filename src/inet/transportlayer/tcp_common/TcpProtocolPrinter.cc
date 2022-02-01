//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/tcp_common/TcpProtocolPrinter.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

namespace inet {

Register_Protocol_Printer(&Protocol::tcp, TcpProtocolPrinter);

void TcpProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    using namespace tcp;

    if (auto header = dynamicPtrCast<const TcpHeader>(chunk)) {
        context.sourceColumn << header->getSrcPort();
        context.destinationColumn << header->getDestPort();
        static const char *flagStart = " [";
        static const char *flagSepar = " ";
        static const char *flagEnd = "]";
        context.infoColumn << header->getSrcPort() << "->" << header->getDestPort();
        const char *separ = flagStart;
        if (header->getUrgBit()) {
            context.infoColumn << separ << "Urg=" << header->getUrgentPointer();
            separ = flagSepar;
        }
        if (header->getSynBit()) {
            context.infoColumn << separ << "Syn";
            separ = flagSepar;
        }
        if (header->getAckBit()) {
            context.infoColumn << separ << "Ack=" << header->getAckNo();
            separ = flagSepar;
        }
        if (header->getPshBit()) {
            context.infoColumn << separ << "Psh";
            separ = flagSepar;
        }
        if (header->getRstBit()) {
            context.infoColumn << separ << "Rst";
            separ = flagSepar;
        }
        if (header->getFinBit()) {
            context.infoColumn << separ << "Fin";
            separ = flagSepar;
        }
        if (separ == flagSepar)
            context.infoColumn << flagEnd;

        context.infoColumn << " Seq=" << header->getSequenceNo()
                           << " Win=" << header->getWindow();
        // TODO show options
    }
    else
        context.infoColumn << "(TCP) " << chunk;
}

} // namespace inet


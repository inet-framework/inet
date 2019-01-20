//
// Copyright (C) 2018 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// @author: Zoltan Bojthe
//

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "inet/transportlayer/tcp_common/TcpProtocolPrinter.h"

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
        //TODO show options
    }
    else
        context.infoColumn << "(TCP) " << chunk;
}

} // namespace inet


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
#include "inet/networklayer/common/EchoPacket_m.h"
#include "inet/networklayer/common/EchoProtocolPrinter.h"

namespace inet {

Register_Protocol_Printer(&Protocol::echo, EchoProtocolPrinter);

void EchoProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto header = dynamicPtrCast<const EchoPacket>(chunk)) {
        context.infoColumn << "Echo";
        switch (header->getType()) {
            case ECHO_PROTOCOL_REQUEST:
                context.infoColumn << " request";
                break;
            case ECHO_PROTOCOL_REPLY:
                context.infoColumn << " reply";
                break;
            default:
                context.infoColumn << " type=" << header->getType();
                break;
        }
        context.infoColumn << " id:" << header->getIdentifier() << " seq:" << header->getSeqNumber();
    }
    else
        context.infoColumn << "(Echo) " << chunk;
}

} // namespace inet


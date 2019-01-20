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
#include "inet/linklayer/acking/AckingMacHeader_m.h"
#include "inet/linklayer/acking/AckingMacProtocolPrinter.h"

namespace inet {

Register_Protocol_Printer(&Protocol::ackingMac, AckingMacProtocolPrinter);

void AckingMacProtocolPrinter::print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, Context& context) const
{
    if (auto header = dynamicPtrCast<const AckingMacHeader>(chunk)) {
        context.sourceColumn << header->getSrc();
        context.destinationColumn << header->getDest();
        context.infoColumn << "(Acking MAC) " << chunk;        //TODO
    }
    else
        context.infoColumn << "(Acking MAC) " << chunk;
}

} // namespace inet


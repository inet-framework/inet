//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/common/packet/chunk/BitCountChunk.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/common/packet/printer/ProtocolPrinter.h"
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


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

#ifndef __INET_PROTOCOLPRINTER_H_
#define __INET_PROTOCOLPRINTER_H_

#include "inet/common/packet/chunk/Chunk.h"
#include "inet/common/Protocol.h"

namespace inet {

class PacketPrinterContext;

/**
 * Protocol printer classes print protocol specific chunks into a context.
 */
class INET_API ProtocolPrinter : public cObject
{
  public:
    /**
     * Prints the given chunk of protocol according to options into context.
     */
    virtual void print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, PacketPrinterContext& context) const = 0;
};

class INET_API DefaultProtocolPrinter : public ProtocolPrinter
{
  public:
    virtual void print(const Ptr<const Chunk>& chunk, const Protocol *protocol, const cMessagePrinter::Options *options, PacketPrinterContext& context) const override;
};

} // namespace

#endif // #ifndef __INET_PROTOCOLPRINTER_H_


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

#ifndef __INET_PACKETPRINTER_H_
#define __INET_PACKETPRINTER_H_

#include "inet/common/packet/dissector/PacketDissector.h"

namespace inet {

class INET_API PacketPrinter : public cMessagePrinter
{
  protected:
    class INET_API ChunkVisitor : public PacketDissector::ChunkVisitor
    {
      protected:
        std::ostream& stream;
        const PacketPrinter& packetPrinter;

      public:
        ChunkVisitor(std::ostream& stream, const PacketPrinter& packetPrinter);
        virtual void visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol) const override;
    };

  public:
    virtual int getScoreFor(cMessage *msg) const override;

    virtual void printMessage(std::ostream& stream, cMessage *message) const override;
    virtual void printPacket(std::ostream& stream, Packet *packet) const;
    virtual void printIeee80211Chunk(std::ostream& stream, const Ptr<const Chunk>& chunk) const;
    virtual void printIeee8022Chunk(std::ostream& stream, const Ptr<const Chunk>& chunk) const;
    virtual void printArpChunk(std::ostream& stream, const Ptr<const Chunk>& chunk) const;
    virtual void printIpv4Chunk(std::ostream& stream, const Ptr<const Chunk>& chunk) const;
    virtual void printIcmpChunk(std::ostream& stream, const Ptr<const Chunk>& chunk) const;
    virtual void printUdpChunk(std::ostream& stream, const Ptr<const Chunk>& chunk) const;
    virtual void printUnimplementedProtocolChunk(std::ostream& stream, const Ptr<const Chunk>& chunk, const Protocol* protocol) const;
    virtual void printUnknownProtocolChunk(std::ostream& stream, const Ptr<const Chunk>& chunk) const;
};

} // namespace

#endif // #ifndef __INET_PACKETPRINTER_H_


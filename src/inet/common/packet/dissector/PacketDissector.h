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

#ifndef __INET_PACKETDISSECTOR_H_
#define __INET_PACKETDISSECTOR_H_

#include <functional>
#include "inet/common/packet/dissector/ProtocolDissector.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/Protocol.h"

namespace inet {

/**
 * This class provides efficient general purpose packet dissection. Packet
 * dissection may be useful for printing packet details, filter packets,
 * finding information in a packet deep down.
 *
 * The supported protocols are provided by the ProtocolDissectorRegistry. The
 * packet dissection algorithm calls the visitChunk() method of the provided
 * PacketDissector::ChunkVisitor for each protocol specific chunk found in the
 * packet. The chunks are passed to the method in the order they appear in the
 * packet from left to right using their most specific protocol dependent form.
 */
class INET_API PacketDissector
{
  public:
    class INET_API ChunkVisitor
    {
      public:
        virtual void startProtocol(const Protocol *protocol) = 0;
        virtual void endProtocol(const Protocol *protocol) = 0;
        virtual void visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol) = 0;
    };

  protected:
    const ProtocolDissectorRegistry& protocolDissectorRegistry;
    ChunkVisitor& chunkVisitor;

  public:
    PacketDissector(const ProtocolDissectorRegistry& protocolDissectorRegistry, ChunkVisitor& chunkVisitor);

    /**
     * Dissects the given packet assuming its a packet of the provided protocol.
     */
    void dissectPacket(Packet *packet, const Protocol *protocol) const;

    /**
     * TODO
     */
    void startProtocol(const Protocol *protocol) const;

    /**
     * TODO
     */
    void endProtocol(const Protocol *protocol) const;

    /**
     * This is a callback method for individual ProtocolDissectors to notify the
     * PacketDissector about a chunk that is found.
     */
    void visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol) const;
};

} // namespace

#endif // #ifndef __INET_PACKETDISSECTOR_H_


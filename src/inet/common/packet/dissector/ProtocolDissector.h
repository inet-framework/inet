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

#ifndef __INET_PROTOCOLDISSECTOR_H_
#define __INET_PROTOCOLDISSECTOR_H_

#include "inet/common/Protocol.h"
#include "inet/common/packet/Packet.h"

namespace inet {

/**
 * Protocol dissector classes dissect packets into protocol specific meaningful
 * parts. The algorithm calls the visitor method exactly one time for each part
 * in order from left to right. For an aggregate packet all aggregated parts are
 * visited in the order they appear in the packet. For a fragmented packet the
 * fragment part is visited as a whole. If dissecting that part is also needed
 * then another dissector must be used for that part.
 *
 * Dissectors can handle both protocol specific and raw representations (raw
 * bytes or bits). In general, dissectors call the chunk visitor with the most
 * specific representation available for a particular protocol.
 */
class INET_API ProtocolDissector : public cObject
{
  public:
    class INET_API ICallback
    {
      public:
        /**
         * Notifies about the start of a new protocol data unit (PDU).
         */
        virtual void startProtocolDataUnit(const Protocol *protocol) = 0;

        /**
         * Notifies about the end of the current protocol data unit (PDU).
         */
        virtual void endProtocolDataUnit(const Protocol *protocol) = 0;

        /**
         * Marks the current protocol data unit as incorrect (e.g. bad CRC/FCS, incorrect length field, bit error).
         */
        virtual void markIncorrect() = 0;

        /**
         * Notifies about a new chunk in the current protocol data unit (PDU).
         */
        virtual void visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol) = 0;

        /**
         * Requests the dissection of the data part of packet according to the given protocol.
         */
        virtual void dissectPacket(Packet *packet, const Protocol *protocol) = 0;
    };

  public:
    /**
     * Dissects the packet according to the protocol implemented by this ProtocolDissector.
     */
    virtual void dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const = 0;
};

class INET_API DefaultProtocolDissector : public ProtocolDissector
{
  public:
    virtual void dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const override;
};

} // namespace

#endif // #ifndef __INET_PROTOCOLDISSECTOR_H_


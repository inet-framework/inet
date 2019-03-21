//
// Copyright (C) OpenSim Ltd.
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

#ifndef __INET_PACKETFILTER_H
#define __INET_PACKETFILTER_H

#include "inet/common/MatchableObject.h"
#include "inet/common/packet/dissector/PacketDissector.h"

namespace inet {

/**
 * This class provides a generic filter for packets. The filter is expressed
 * as two patterns using the cMatchExpression format. One filter is applied
 * to the Packet the other one is applied to each Chunk in the packet using
 * the PacketDissector.
 */
class INET_API PacketFilter
{
  protected:
    class INET_API PacketDissectorCallback : public PacketDissector::ICallback
    {
      protected:
        bool matches_ = false;
        const PacketFilter& packetFilter;

      public:
        PacketDissectorCallback(const PacketFilter& packetFilter);

        bool matches(const Packet *packet);

        virtual bool shouldDissectProtocolDataUnit(const Protocol *protocol) override { return true; }
        virtual void startProtocolDataUnit(const Protocol *protocol) override;
        virtual void endProtocolDataUnit(const Protocol *protocol) override;
        virtual void markIncorrect() override;
        virtual void visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol) override;
    };

  protected:
    cMatchExpression packetMatchExpression;
    cMatchExpression chunkMatchExpression;

  public:
    void setPattern(const char* packetPattern, const char* chunkPattern);

    bool matches(const cPacket *packet) const;
};

} // namespace inet

#endif // ifndef __INET_PACKETFILTER_H


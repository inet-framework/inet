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
 * as a pattern using the cMatchExpression format.
 */
class INET_API PacketFilter
{
  protected:
    class INET_API ChunkVisitor : public PacketDissector::ChunkVisitor
    {
      protected:
        mutable bool matches_ = false;
        const PacketFilter& packetFilter;

      public:
        ChunkVisitor(const PacketFilter& packetFilter);

        bool matches(const Packet *packet) const;
        virtual void visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol) const override;
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


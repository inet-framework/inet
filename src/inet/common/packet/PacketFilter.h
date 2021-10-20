//
// Copyright (C) 2020 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_PACKETFILTER_H
#define __INET_PACKETFILTER_H

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
    class INET_API PacketDissectorCallback : public PacketDissector::ICallback {
      protected:
        PacketFilter *packetFilter = nullptr;

      public:
        PacketDissectorCallback(PacketFilter *packetFilter) : packetFilter(packetFilter) { }
        virtual ~PacketDissectorCallback() { }

        virtual bool shouldDissectProtocolDataUnit(const Protocol *protocol) override { return true; }
        virtual void startProtocolDataUnit(const Protocol *protocol) override { }
        virtual void endProtocolDataUnit(const Protocol *protocol) override { }
        virtual void markIncorrect() override { }
        virtual void visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol) override;
    };

    class INET_API DynamicExpressionResolver : public cDynamicExpression::ResolverBase {
      protected:
        PacketFilter *packetFilter = nullptr;

      public:
        DynamicExpressionResolver(PacketFilter *packetFilter) : packetFilter(packetFilter) { }

        virtual IResolver *dup() const override { throw cRuntimeError("Not implemented"); }

        virtual cValue readVariable(cExpression::Context *context, const char *name) override;
        virtual cValue readVariable(cExpression::Context *context, const char *name, intval_t index) override;
        virtual cValue readMember(cExpression::Context *context, const cValue &object, const char *name) override;
        virtual cValue callMethod(cExpression::Context *context, const cValue& object, const char *name, cValue argv[], int argc) override;
    };

  protected:
    cDynamicExpression filterExpression;
    PacketDissectorCallback *packetDissectorCallback = nullptr;
    DynamicExpressionResolver *dynamicExpressionResolver = nullptr;
    mutable const cPacket *cpacket = nullptr;
    mutable std::multimap<const Protocol *, Chunk *> protocolToChunkMap;
    mutable std::multimap<std::string, Chunk *> classNameToChunkMap;

  public:
    virtual ~PacketFilter() { delete packetDissectorCallback; }

    void setExpression(const char *expression);

    bool matches(const cPacket *packet) const;
};

} // namespace inet

#endif


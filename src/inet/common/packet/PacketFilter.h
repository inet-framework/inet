//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

    class INET_API DynamicExpressionResolver : public cDynamicExpression::IResolver {
      protected:
        PacketFilter *packetFilter = nullptr;

      public:
        DynamicExpressionResolver(PacketFilter *packetFilter) : packetFilter(packetFilter) { }

        virtual IResolver *dup() const override { return new DynamicExpressionResolver(packetFilter); }

        virtual cValue readVariable(cExpression::Context *context, const char *name) override;
        virtual cValue readVariable(cExpression::Context *context, const char *name, intval_t index) override;
        virtual cValue readMember(cExpression::Context *context, const cValue& object, const char *name) override;
        virtual cValue readMember(cExpression::Context *context, const cValue& object, const char *name, intval_t index) override;
        virtual cValue callMethod(cExpression::Context *context, const cValue& object, const char *name, cValue argv[], int argc) override;
        virtual cValue callFunction(cExpression::Context *context, const char *name, cValue argv[], int argc) override;
    };

  protected:
    cDynamicExpression *filterExpression = nullptr;
    cMatchExpression *matchExpression = nullptr;
    PacketDissectorCallback *packetDissectorCallback = nullptr;
    mutable const cPacket *cpacket = nullptr;
    mutable std::multimap<const Protocol *, Ptr<const Chunk>> protocolToChunkMap;
    mutable std::multimap<std::string, Ptr<const Chunk>> classNameToChunkMap;

  public:
    PacketFilter();
    virtual ~PacketFilter();

    void setPattern(const char *pattern);
    void setExpression(const char *expression);
    void setExpression(cDynamicExpression *expression);
    void setExpression(cOwnedDynamicExpression *expression);
    void setExpression(cValueHolder *expression);
    void setExpression(cObject *expression);
    void setExpression(cValue& expression);

    bool matches(const cPacket *packet) const;

  protected:
    const cObject *findPacketTag(const char *className) const;
};

} // namespace inet

#endif


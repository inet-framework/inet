//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BEHAVIORAGGREGATECLASSIFIER_H
#define __INET_BEHAVIORAGGREGATECLASSIFIER_H

#include "inet/common/packet/Packet.h"
#include "inet/common/packet/dissector/PacketDissector.h"
#include "inet/queueing/base/PacketClassifierBase.h"

namespace inet {

/**
 * Behavior Aggregate Classifier.
 */
class INET_API BehaviorAggregateClassifier : public queueing::PacketClassifierBase
{
  protected:
    class INET_API PacketDissectorCallback : public PacketDissector::ICallback {
      protected:
        bool matches_ = false;
        bool dissect = true;

      public:
        int dscp = -1;

      public:
        PacketDissectorCallback() {}

        bool matches(Packet *packet);

        virtual bool shouldDissectProtocolDataUnit(const Protocol *protocol) override { return dissect; }
        virtual void startProtocolDataUnit(const Protocol *protocol) override {}
        virtual void endProtocolDataUnit(const Protocol *protocol) override {}
        virtual void markIncorrect() override {}
        virtual void visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol) override;
    };

    int numOutGates = 0;
    std::map<int, int> dscpToGateIndexMap;

    int numRcvd = 0;

    static simsignal_t pkClassSignal;

  public:
    BehaviorAggregateClassifier() {}

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual void pushPacket(Packet *packet, cGate *gate) override;
    virtual int classifyPacket(Packet *packet) override;
};

} // namespace inet

#endif


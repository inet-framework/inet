//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MULTIFIELDCLASSIFIER_H
#define __INET_MULTIFIELDCLASSIFIER_H

#include "inet/common/packet/Packet.h"
#include "inet/common/packet/dissector/PacketDissector.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/queueing/base/PacketClassifierBase.h"

namespace inet {

/**
 * Absolute dropper.
 */
class INET_API MultiFieldClassifier : public queueing::PacketClassifierBase
{
  protected:
    class INET_API PacketDissectorCallback : public PacketDissector::ICallback {
      protected:
        bool dissect = true;
        bool matchesL3 = false;
        bool matchesL4 = false;

      public:
        int gateIndex = -1;

        L3Address srcAddr;
        int srcPrefixLength = 0;
        L3Address destAddr;
        int destPrefixLength = 0;
        int protocolId = -1;
        int dscp = -1;
        int tos = -1;
        int tosMask = 0;
        int srcPortMin = -1;
        int srcPortMax = -1;
        int destPortMin = -1;
        int destPortMax = -1;

      public:
        PacketDissectorCallback() {}
        virtual ~PacketDissectorCallback() {}

        bool matches(Packet *packet);

        virtual bool shouldDissectProtocolDataUnit(const Protocol *protocol) override;
        virtual void startProtocolDataUnit(const Protocol *protocol) override {}
        virtual void endProtocolDataUnit(const Protocol *protocol) override {}
        virtual void markIncorrect() override {}
        virtual void visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol) override;
    };

  protected:
    int numOutGates = 0;
    std::vector<PacketDissectorCallback> filters;

    int numRcvd = 0;

    static simsignal_t pkClassSignal;

  protected:
    void addFilter(const PacketDissectorCallback& filter);
    void configureFilters(cXMLElement *config);

  public:
    MultiFieldClassifier() {}

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void pushPacket(Packet *packet, cGate *gate) override;
    virtual void refreshDisplay() const override;

    virtual int classifyPacket(Packet *packet) override;

    virtual void mapRegistrationForwardingGates(cGate *gate, std::function<void(cGate *)> f) override;

};

} // namespace inet

#endif


//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_QOSCLASSIFIER_H
#define __INET_QOSCLASSIFIER_H

#include "inet/queueing/common/PassivePacketSinkRef.h"
#include "inet/common/INETDefs.h"

namespace inet {

using namespace inet::queueing;

/**
 * This module classifies and assigns User Priority to packets.
 */
class INET_API QosClassifier : public cSimpleModule, public IPassivePacketSink
{
  protected:
    PassivePacketSinkRef outSink;

    int defaultUp;
    std::map<int, int> ipProtocolUpMap;
    std::map<int, int> udpPortUpMap;
    std::map<int, int> tcpPortUpMap;

    virtual int parseUserPriority(const char *text);
    virtual void parseUserPriorityMap(const char *text, std::map<int, int>& upMap);

    virtual int getUserPriority(cMessage *msg);

  public:
    QosClassifier() {}

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;


    virtual bool canPushSomePacket(const cGate *gate) const override { return gate->isName("in"); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return gate->isName("in"); }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }
};

} // namespace inet

#endif


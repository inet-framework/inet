//
// Copyright (C) 2010 Alfonso Ariza
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_EXAMPLEQOSCLASSIFIER_H
#define __INET_EXAMPLEQOSCLASSIFIER_H

#include "inet/queueing/common/PassivePacketSinkRef.h"
#include "inet/common/INETDefs.h"

namespace inet {

using namespace inet::queueing;

/**
 * An example packet classifier based on the UDP/TCP port number.
 */
class INET_API ExampleQosClassifier : public cSimpleModule, public IPassivePacketSink
{
  protected:
    PassivePacketSinkRef outSink;

  protected:
    virtual int getUserPriority(cMessage *msg);

  public:
    ExampleQosClassifier() {}
    virtual void initialize() override;
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


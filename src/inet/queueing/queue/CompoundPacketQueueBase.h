//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_COMPOUNDPACKETQUEUEBASE_H
#define __INET_COMPOUNDPACKETQUEUEBASE_H

#include "inet/queueing/base/PacketQueueBase.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"
#include "inet/queueing/common/PassivePacketSourceRef.h"
#include "inet/queueing/contract/IPacketCollection.h"
#include "inet/queueing/contract/IPacketDropperFunction.h"

namespace inet {
namespace queueing {

class INET_API CompoundPacketQueueBase : public PacketQueueBase, public cListener
{
  protected:
    int packetCapacity = -1;
    b dataCapacity = b(-1);

    PassivePacketSinkRef consumer;
    PassivePacketSourceRef provider;
    IPacketCollection *collection = nullptr;

    IPacketDropperFunction *packetDropperFunction = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual IPacketDropperFunction *createDropperFunction(const char *dropperClass) const;

    virtual bool isOverloaded() const;

  public:
    virtual ~CompoundPacketQueueBase() { delete packetDropperFunction; }

    virtual int getMaxNumPackets() const override { return packetCapacity; }
    virtual int getNumPackets() const override { return collection->getNumPackets(); }

    virtual b getMaxTotalLength() const override { return dataCapacity; }
    virtual b getTotalLength() const override { return collection->getTotalLength(); }

    virtual bool isEmpty() const override { return collection->isEmpty(); }
    virtual Packet *getPacket(int index) const override { return collection->getPacket(index); }
    virtual void removePacket(Packet *packet) override;
    virtual void removeAllPackets() override;

    virtual bool supportsPacketPushing(const cGate *gate) const override { return inputGate == gate; }
    virtual bool canPushSomePacket(const cGate *gate) const override;
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override;
    virtual void pushPacket(Packet *packet, const cGate *gate) override;

    virtual bool supportsPacketPulling(const cGate *gate) const override { return outputGate == gate; }
    virtual bool canPullSomePacket(const cGate *gate) const override { return provider.canPullSomePacket(); }
    virtual Packet *canPullPacket(const cGate *gate) const override { return provider.canPullPacket(); }
    virtual Packet *pullPacket(const cGate *gate) override;

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace queueing
} // namespace inet

#endif


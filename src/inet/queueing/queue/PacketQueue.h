//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETQUEUE_H
#define __INET_PACKETQUEUE_H

#include "inet/queueing/base/PacketQueueBase.h"
#include "inet/queueing/common/ActivePacketSinkRef.h"
#include "inet/queueing/common/ActivePacketSourceRef.h"
#include "inet/queueing/contract/IActivePacketSink.h"
#include "inet/queueing/contract/IActivePacketSource.h"
#include "inet/queueing/contract/IPacketBuffer.h"
#include "inet/queueing/contract/IPacketComparatorFunction.h"
#include "inet/queueing/contract/IPacketDropperFunction.h"

namespace inet {
namespace queueing {

class INET_API PacketQueue : public PacketQueueBase, public IPacketBuffer::ICallback
{
  protected:
    int packetCapacity = -1;
    b dataCapacity = b(-1);

    ActivePacketSourceRef producer;
    ActivePacketSinkRef collector;

    cPacketQueue queue;
    IPacketBuffer *buffer = nullptr;

    IPacketDropperFunction *packetDropperFunction = nullptr;
    IPacketComparatorFunction *packetComparatorFunction = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual IPacketDropperFunction *createDropperFunction(const char *dropperClass) const;
    virtual IPacketComparatorFunction *createComparatorFunction(const char *comparatorClass) const;

    virtual bool isOverloaded() const;

  public:
    virtual ~PacketQueue() { delete packetDropperFunction; }


    virtual int getMaxNumPackets() const override { return packetCapacity; }
    virtual int getNumPackets() const override;

    virtual b getMaxTotalLength() const override { return dataCapacity; }
    virtual b getTotalLength() const override { return b(queue.getBitLength()); }

    virtual bool isEmpty() const override { return getNumPackets() == 0; }
    virtual Packet *getPacket(int index) const override;
    virtual void removePacket(Packet *packet) override;
    virtual void removeAllPackets() override;

    virtual bool supportsPacketPushing(const cGate *gate) const override { return inputGate == gate; }
    virtual bool canPushSomePacket(const cGate *gate) const override;
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override;
    virtual void pushPacket(Packet *packet, const cGate *gate) override;

    virtual bool supportsPacketPulling(const cGate *gate) const override { return outputGate == gate; }
    virtual bool canPullSomePacket(const cGate *gate) const override { return !isEmpty(); }
    virtual Packet *canPullPacket(const cGate *gate) const override { return !isEmpty() ? getPacket(0) : nullptr; }
    virtual Packet *pullPacket(const cGate *gate) override;

    virtual void handlePacketRemoved(Packet *packet) override;
};

} // namespace queueing
} // namespace inet

#endif


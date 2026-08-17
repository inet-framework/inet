//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_WRRSCHEDULER_H
#define __INET_WRRSCHEDULER_H

#include "inet/queueing/base/PacketSchedulerBase.h"
#include "inet/queueing/contract/IPacketCollection.h"
#include "inet/queueing/contract/IPacketExtractor.h"

namespace inet {
namespace queueing {

/**
 * This module implements a Weighted Round Robin Scheduler.
 */
class INET_API WrrScheduler : public PacketSchedulerBase, public virtual IPacketCollection, public virtual IPacketExtractor
{
  protected:
    unsigned int *weights = nullptr; // array of weights (has numInputs elements)
    unsigned int *buckets = nullptr; // array of tokens in buckets (has numInputs elements)

    std::vector<IPacketCollection *> collections;

  protected:
    virtual void initialize(int stage) override;
    virtual int schedulePacket() override;
    virtual int findInput(const PacketPredicate& predicate) const;
    virtual void consumeBucket(int index);

  public:
    virtual ~WrrScheduler();

    virtual int getMaxNumPackets() const override { return -1; }
    virtual int getNumPackets() const override;

    virtual b getMaxTotalLength() const override { return b(-1); }
    virtual b getTotalLength() const override;

    virtual bool isEmpty() const override { return getNumPackets() == 0; }
    virtual Packet *getPacket(int index) const override;
    virtual void removePacket(Packet *packet) override;
    virtual Packet *findPacket(const PacketPredicate& predicate) const override;
    virtual Packet *dequeuePacket(const PacketPredicate& predicate) override;
    virtual void removeAllPackets() override;
};

} // namespace queueing
} // namespace inet

#endif

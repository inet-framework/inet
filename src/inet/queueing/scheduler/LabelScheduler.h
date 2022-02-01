//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LABELSCHEDULER_H
#define __INET_LABELSCHEDULER_H

#include "inet/queueing/base/PacketSchedulerBase.h"
#include "inet/queueing/contract/IPacketCollection.h"

namespace inet {
namespace queueing {

class INET_API LabelScheduler : public PacketSchedulerBase, public virtual IPacketCollection
{
  protected:
    int defaultGateIndex = -1;
    std::vector<std::string> labels;
    std::vector<IPacketCollection *> collections;

  protected:
    virtual void initialize(int stage) override;
    virtual int schedulePacket() override;

  public:
    virtual int getMaxNumPackets() const override { return -1; }
    virtual int getNumPackets() const override;

    virtual b getMaxTotalLength() const override { return b(-1); }
    virtual b getTotalLength() const override;

    virtual bool isEmpty() const override { return getNumPackets() == 0; }
    virtual Packet *getPacket(int index) const override;
    virtual void removePacket(Packet *packet) override;
    virtual void removeAllPackets() override;
};

} // namespace queueing
} // namespace inet

#endif


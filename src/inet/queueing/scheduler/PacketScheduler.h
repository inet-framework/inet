//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETSCHEDULER_H
#define __INET_PACKETSCHEDULER_H

#include "inet/queueing/base/PacketSchedulerBase.h"
#include "inet/queueing/contract/IPacketSchedulerFunction.h"

namespace inet {
namespace queueing {

class INET_API PacketScheduler : public PacketSchedulerBase
{
  protected:
    IPacketSchedulerFunction *packetSchedulerFunction = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual IPacketSchedulerFunction *createSchedulerFunction(const char *schedulerClass) const;
    virtual int schedulePacket() override;

  public:
    virtual ~PacketScheduler() { delete packetSchedulerFunction; }
};

} // namespace queueing
} // namespace inet

#endif


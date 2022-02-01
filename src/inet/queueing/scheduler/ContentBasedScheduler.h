//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CONTENTBASEDSCHEDULER_H
#define __INET_CONTENTBASEDSCHEDULER_H

#include "inet/common/packet/PacketFilter.h"
#include "inet/queueing/base/PacketSchedulerBase.h"

namespace inet {
namespace queueing {

class INET_API ContentBasedScheduler : public PacketSchedulerBase
{
  protected:
    int defaultGateIndex = -1;
    std::vector<PacketFilter *> filters;

  protected:
    virtual void initialize(int stage) override;
    virtual int schedulePacket() override;

  public:
    virtual ~ContentBasedScheduler();
};

} // namespace queueing
} // namespace inet

#endif


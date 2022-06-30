//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETGATE_H
#define __INET_PACKETGATE_H

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/base/PacketGateBase.h"

namespace inet {

extern template class ClockUserModuleMixin<queueing::PacketGateBase>;

namespace queueing {

class INET_API PacketGate : public ClockUserModuleMixin<PacketGateBase>
{
  protected:
    clocktime_t openTime;
    clocktime_t closeTime;

    ClockEvent *changeTimer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void scheduleChangeTimer();
    virtual void processChangeTimer();

  public:
    virtual ~PacketGate() { cancelAndDeleteClockEvent(changeTimer); }
};

} // namespace queueing
} // namespace inet

#endif


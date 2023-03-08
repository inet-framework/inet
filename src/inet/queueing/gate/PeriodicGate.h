//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PERIODICGATE_H
#define __INET_PERIODICGATE_H

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/base/PacketGateBase.h"

namespace inet {

extern template class ClockUserModuleMixin<queueing::PacketGateBase>;

namespace queueing {

class INET_API PeriodicGate : public ClockUserModuleMixin<PacketGateBase>
{
  protected:
    int index = 0;
    clocktime_t initialOffset;
    clocktime_t offset;
    std::vector<clocktime_t> durations;
    bool scheduleForAbsoluteTime = false;
    short openSchedulingPriority = 0;
    short closeSchedulingPriority = 0;

    ClockEvent *changeTimer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void handleParameterChange(const char *name) override;
    virtual bool canPacketFlowThrough(Packet *packet) const override;

    virtual void initializeGating();
    virtual void scheduleChangeTimer(bool toOpen);
    virtual void processChangeTimer();

  public:
    virtual ~PeriodicGate() { cancelAndDelete(changeTimer); }

    virtual clocktime_t getInitialOffset() const { return initialOffset; }
    virtual bool getInitiallyOpen() const { return par("initiallyOpen"); }
    virtual const cValueArray *getDurations() const { return check_and_cast<cValueArray *>(par("durations").objectValue()); }
};

} // namespace queueing
} // namespace inet

#endif


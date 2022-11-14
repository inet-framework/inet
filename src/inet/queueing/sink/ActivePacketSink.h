//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ACTIVEPACKETSINK_H
#define __INET_ACTIVEPACKETSINK_H

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/base/ActivePacketSinkBase.h"

namespace inet {

extern template class ClockUserModuleMixin<queueing::ActivePacketSinkBase>;

namespace queueing {

class INET_API ActivePacketSink : public ClockUserModuleMixin<ActivePacketSinkBase>
{
  protected:
    bool initialCollectionOffsetScheduled = false;
    clocktime_t initialCollectionOffset;
    cPar *collectionIntervalParameter = nullptr;
    ClockEvent *collectionTimer = nullptr;
    bool scheduleForAbsoluteTime = false;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void handleParameterChange(const char *name) override;

    virtual void scheduleCollectionTimer(clocktime_t delay);
    virtual void scheduleCollectionTimerAndCollectPacket();
    virtual void collectPacket();

  public:
    virtual ~ActivePacketSink() { cancelAndDeleteClockEvent(collectionTimer); }

    virtual void handleCanPullPacketChanged(cGate *gate) override;
    virtual void handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful) override;
};

} // namespace queueing
} // namespace inet

#endif


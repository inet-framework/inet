//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ELIGIBILITYTIMEGATE_H
#define __INET_ELIGIBILITYTIMEGATE_H

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/base/PacketGateBase.h"

namespace inet {

using namespace inet::queueing;

extern template class ClockUserModuleMixin<PacketGateBase>;

class INET_API EligibilityTimeGate : public ClockUserModuleMixin<PacketGateBase>
{
  public:
    static simsignal_t remainingEligibilityTimeChangedSignal;

  protected:
    ClockEvent *eligibilityTimer = nullptr;

    simtime_t lastRemainingEligibilityTimeSignalValue = -1;
    simtime_t lastRemainingEligibilityTimeSignalTime = -1;
    Packet *lastRemainingEligibilityTimePacket = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void finish() override;

    virtual void updateOpen();
    virtual void emitEligibilityTimeChangedSignal();

  public:
    virtual ~EligibilityTimeGate() { cancelAndDelete(eligibilityTimer); }

    virtual Packet *pullPacket(cGate *gate) override;

    virtual void handleCanPullPacketChanged(cGate *gate) override;
};

} // namespace inet

#endif


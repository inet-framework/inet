//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ELIGIBILITYTIMEMETER_H
#define __INET_ELIGIBILITYTIMEMETER_H

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/base/PacketMeterBase.h"

namespace inet {

using namespace inet::queueing;

extern template class ClockUserModuleMixin<PacketMeterBase>;

class INET_API EligibilityTimeMeter : public ClockUserModuleMixin<PacketMeterBase>
{
  protected:
    b packetOverheadLength = b(-1);
    bps committedInformationRate = bps(NaN);
    b committedBurstSize = b(-1);
    clocktime_t maxResidenceTime;

    clocktime_t groupEligibilityTime;
    clocktime_t bucketEmptyTime;

    double numTokens = 0;

  protected:
    virtual void initialize(int stage) override;

    virtual void meterPacket(Packet *packet) override;

    virtual void emitNumTokenChangedSignal(Packet *packet);
};

} // namespace inet

#endif


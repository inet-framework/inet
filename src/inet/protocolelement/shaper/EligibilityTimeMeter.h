//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_ELIGIBILITYTIMEMETER_H
#define __INET_ELIGIBILITYTIMEMETER_H

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/base/PacketMeterBase.h"

namespace inet {

using namespace inet::queueing;

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

    virtual void emitNumTokenChangedSignal();
};

} // namespace inet

#endif


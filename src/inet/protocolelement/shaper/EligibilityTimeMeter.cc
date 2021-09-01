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

#include "inet/protocolelement/shaper/EligibilityTimeMeter.h"

#include "inet/protocolelement/shaper/EligibilityTimeTag_m.h"

namespace inet {

Define_Module(EligibilityTimeMeter);

void EligibilityTimeMeter::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        committedInformationRate = bps(par("committedInformationRate"));
        committedBurstSize = b(par("committedBurstSize"));
        maxResidenceTime = par("maxResidenceTime");
        WATCH(groupEligibilityTime);
        WATCH(bucketEmptyTime);
        WATCH(maxResidenceTime);
    }
}

void EligibilityTimeMeter::meterPacket(Packet *packet)
{
    clocktime_t arrivalTime = getClockTime();
    clocktime_t lengthRecoveryDuration = s(packet->getDataLength() / committedInformationRate).get();
    clocktime_t emptyToFullDuration = s(committedBurstSize / committedInformationRate).get();
    clocktime_t schedulerEligibilityTime = bucketEmptyTime + lengthRecoveryDuration;
    clocktime_t bucketFullTime = bucketEmptyTime + emptyToFullDuration;
    clocktime_t eligibilityTime;
    eligibilityTime.setRaw(std::max(std::max(arrivalTime.raw(), groupEligibilityTime.raw()), schedulerEligibilityTime.raw()));
    if (eligibilityTime <= arrivalTime + maxResidenceTime) {
        groupEligibilityTime = eligibilityTime;
        bucketEmptyTime = eligibilityTime < bucketFullTime ? schedulerEligibilityTime : schedulerEligibilityTime + eligibilityTime - bucketFullTime;
        packet->addTagIfAbsent<EligibilityTimeTag>()->setEligibilityTime(eligibilityTime);
    }
}

} // namespace inet


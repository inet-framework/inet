//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/shaper/EligibilityTimeMeter.h"

#include "inet/protocolelement/shaper/EligibilityTimeTag_m.h"

namespace inet {

Define_Module(EligibilityTimeMeter);

void EligibilityTimeMeter::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        packetOverheadLength = b(par("packetOverheadLength"));
        committedInformationRate = bps(par("committedInformationRate"));
        committedBurstSize = b(par("committedBurstSize"));
        maxResidenceTime = par("maxResidenceTime");
        groupEligibilityTime = 0;
        bucketEmptyTime = s(-committedBurstSize / committedInformationRate).get();
        WATCH(groupEligibilityTime);
        WATCH(bucketEmptyTime);
        WATCH(maxResidenceTime);
        WATCH(numTokens);
    }
    else if (stage == INITSTAGE_QUEUEING)
        emitNumTokenChangedSignal();
}

void EligibilityTimeMeter::meterPacket(Packet *packet)
{
    emitNumTokenChangedSignal();
    clocktime_t arrivalTime = getClockTime();
    clocktime_t lengthRecoveryDuration = s((packet->getDataLength() + packetOverheadLength) / committedInformationRate).get();
    clocktime_t emptyToFullDuration = s(committedBurstSize / committedInformationRate).get();
    clocktime_t schedulerEligibilityTime = bucketEmptyTime + lengthRecoveryDuration;
    clocktime_t bucketFullTime = bucketEmptyTime + emptyToFullDuration;
    clocktime_t eligibilityTime;
    eligibilityTime.setRaw(std::max(std::max(arrivalTime.raw(), groupEligibilityTime.raw()), schedulerEligibilityTime.raw()));
    if (maxResidenceTime == -1 || eligibilityTime <= arrivalTime + maxResidenceTime) {
        groupEligibilityTime = eligibilityTime;
        bucketEmptyTime = eligibilityTime < bucketFullTime ? schedulerEligibilityTime : schedulerEligibilityTime + eligibilityTime - bucketFullTime;
        packet->addTagIfAbsent<EligibilityTimeTag>()->setEligibilityTime(eligibilityTime);
        emitNumTokenChangedSignal();
    }
}

void EligibilityTimeMeter::emitNumTokenChangedSignal()
{
    clocktime_t emptyToFullDuration = s(committedBurstSize / committedInformationRate).get();
    double alpha = (getClockTime() - bucketEmptyTime).dbl() / emptyToFullDuration.dbl();
    if (alpha > 1.0) {
        numTokens = b(committedBurstSize).get();
        cTimestampedValue value(CLOCKTIME_AS_SIMTIME(bucketEmptyTime + emptyToFullDuration), numTokens);
        emit(tokensChangedSignal, &value);
        emit(tokensChangedSignal, numTokens);
    }
    else {
        numTokens = b(committedBurstSize).get() * alpha;
        emit(tokensChangedSignal, numTokens);
    }
}

} // namespace inet


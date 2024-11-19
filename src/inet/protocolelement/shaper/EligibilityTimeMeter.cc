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
        bucketEmptyTime = (-committedBurstSize / committedInformationRate).get<s>();
        WATCH(groupEligibilityTime);
        WATCH(bucketEmptyTime);
        WATCH(maxResidenceTime);
        WATCH(numTokens);
    }
    else if (stage == INITSTAGE_QUEUEING)
        emitNumTokenChangedSignal(nullptr);
}

void EligibilityTimeMeter::meterPacket(Packet *packet)
{
    emitNumTokenChangedSignal(packet);
    clocktime_t arrivalTime = getClockTime();
    clocktime_t lengthRecoveryDuration = ((packet->getDataLength() + packetOverheadLength) / committedInformationRate).get<s>();
    clocktime_t emptyToFullDuration = (committedBurstSize / committedInformationRate).get<s>();
    clocktime_t schedulerEligibilityTime = bucketEmptyTime + lengthRecoveryDuration;
    clocktime_t bucketFullTime = bucketEmptyTime + emptyToFullDuration;
    clocktime_t eligibilityTime;
    eligibilityTime.setRaw(std::max(std::max(arrivalTime.raw(), groupEligibilityTime.raw()), schedulerEligibilityTime.raw()));
    if (maxResidenceTime == -1 || eligibilityTime <= arrivalTime + maxResidenceTime) {
        groupEligibilityTime = eligibilityTime;
        bucketEmptyTime = eligibilityTime < bucketFullTime ? schedulerEligibilityTime : schedulerEligibilityTime + eligibilityTime - bucketFullTime;
        packet->addTagIfAbsent<EligibilityTimeTag>()->setEligibilityTime(eligibilityTime);
        emitNumTokenChangedSignal(packet);
    }
}

void EligibilityTimeMeter::emitNumTokenChangedSignal(Packet *packet)
{
    clocktime_t emptyToFullDuration = (committedBurstSize / committedInformationRate).get<s>();
    double alpha = (getClockTime() - bucketEmptyTime).dbl() / emptyToFullDuration.dbl();
    if (alpha > 1.0) {
        numTokens = committedBurstSize.get<b>();
        cTimestampedValue value(CLOCKTIME_AS_SIMTIME(bucketEmptyTime + emptyToFullDuration), numTokens);
        emit(tokensChangedSignal, &value);
        emit(tokensChangedSignal, numTokens);
    }
    else if (alpha < 0) {
        cTimestampedValue value1(CLOCKTIME_AS_SIMTIME(bucketEmptyTime), (double)(packet->getDataLength() + packetOverheadLength).get<b>());
        cTimestampedValue value2(CLOCKTIME_AS_SIMTIME(bucketEmptyTime), 0.0);
        emit(tokensChangedSignal, &value1);
        emit(tokensChangedSignal, &value2);
    }
    else {
        numTokens = committedBurstSize.get<b>() * alpha;
        emit(tokensChangedSignal, numTokens);
    }
}

} // namespace inet


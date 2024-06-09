//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "GroupEligibilityTimeMeter.h"

#include "inet/common/DirectionTag_m.h"
#include "inet/linklayer/common/PcpTag_m.h"
#include "inet/protocolelement/shaper/EligibilityTimeTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"

namespace inet {

Define_Module(GroupEligibilityTimeMeter);

void GroupEligibilityTimeMeter::initialize(int stage)
{
    EligibilityTimeMeter::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
    {
        groupEligibilityTimeTable.reference(this, "groupEligibilityTimeTableModule", true);
    }
}

void GroupEligibilityTimeMeter::meterPacket(Packet *packet)
{
    emitNumTokenChangedSignal(packet);
    clocktime_t arrivalTime = getClockTime();
    clocktime_t lengthRecoveryDuration = s((packet->getDataLength() + packetOverheadLength) / committedInformationRate).get();
    clocktime_t emptyToFullDuration = s(committedBurstSize / committedInformationRate).get();
    clocktime_t schedulerEligibilityTime = bucketEmptyTime + lengthRecoveryDuration;
    clocktime_t bucketFullTime = bucketEmptyTime + emptyToFullDuration;
    clocktime_t eligibilityTime;

    //update groupEligibilityTime
    auto packetDirection = packet->findTag<DirectionTag>();
    int pcp = 0;
    if (packetDirection->getDirection() == DIRECTION_INBOUND)
    {
        auto  pcpTag = packet->findTag<PcpInd>();
        pcp = pcpTag->getPcp();
    }
    else
    {
        auto pcpTag = packet->findTag<PcpReq>();
        pcp = pcpTag->getPcp();
    }
    auto iterface = packet->findTag<InterfaceInd>();
    int port = iterface->getInterfaceId();
    std::string group = std::to_string(port) + "-" + std::to_string(pcp);
    groupEligibilityTime = groupEligibilityTimeTable->getGroupEligibilityTime(group);

    eligibilityTime.setRaw(std::max(std::max(arrivalTime.raw(), groupEligibilityTime.raw()), schedulerEligibilityTime.raw()));
    if (maxResidenceTime == -1 || eligibilityTime <= arrivalTime + maxResidenceTime) {
        groupEligibilityTime = eligibilityTime;
        // write groupEligibilityTime back into table
        groupEligibilityTimeTable->updateGroupEligibilityTime(group, groupEligibilityTime);

        bucketEmptyTime = eligibilityTime < bucketFullTime ? schedulerEligibilityTime : schedulerEligibilityTime + eligibilityTime - bucketFullTime;
        packet->addTagIfAbsent<EligibilityTimeTag>()->setEligibilityTime(eligibilityTime);
        emitNumTokenChangedSignal(packet);
    }

}

} //namespace inet

//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/aggregation/base/AggregatorBase.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

void AggregatorBase::initialize(int stage)
{
    PacketPusherBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        deleteSelf = par("deleteSelf");
        const char *aggregatorPolicyClass = par("aggregatorPolicyClass");
        if (*aggregatorPolicyClass != '\0')
            aggregatorPolicy = createAggregatorPolicy(aggregatorPolicyClass);
        else
            aggregatorPolicy = findModuleFromPar<IAggregatorPolicy>(par("aggregatorPolicyModule"), this);
    }
}

IAggregatorPolicy *AggregatorBase::createAggregatorPolicy(const char *aggregatorPolicyClass) const
{
    return check_and_cast<IAggregatorPolicy *>(createOne(aggregatorPolicyClass));
}

void AggregatorBase::startAggregation(Packet *packet)
{
    ASSERT(aggregatedPacket == nullptr);
    ASSERT(aggregatedSubpackets.size() == 0);
    aggregatedPacket = new Packet("");
    aggregatedPacket->copyTags(*packet); // TODO more complicated?
}

void AggregatorBase::continueAggregation(Packet *packet)
{
    std::string aggregatedName = aggregatedPacket->getName();
    if (aggregatedName.length() != 0)
        aggregatedName += "+";
    aggregatedName += packet->getName();
    aggregatedPacket->setName(aggregatedName.c_str());
    aggregatedSubpackets.push_back(packet);
    numProcessedPackets++;
    processedTotalLength += packet->getDataLength();
}

void AggregatorBase::endAggregation(Packet *packet)
{
    aggregatedSubpackets.clear();
    aggregatedPacket = nullptr;
    if (deleteSelf)
        deleteModule();
}

void AggregatorBase::pushPacket(Packet *subpacket, cGate *gate)
{
    Enter_Method("pushPacket");
    take(subpacket);
    if (!isAggregating())
        startAggregation(subpacket);
    else if (!aggregatorPolicy->isAggregatablePacket(aggregatedPacket, aggregatedSubpackets, subpacket)) {
        pushOrSendPacket(aggregatedPacket, outputGate, consumer);
        endAggregation(subpacket);
        startAggregation(subpacket);
    }
    continueAggregation(subpacket);
    EV_INFO << "Aggregating packet" << EV_FIELD(subpacket) << EV_FIELD(packet, *aggregatedPacket) << EV_ENDL;
    delete subpacket;
    updateDisplayString();
}

} // namespace inet


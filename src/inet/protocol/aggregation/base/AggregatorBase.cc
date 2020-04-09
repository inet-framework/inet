//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/common/ModuleAccess.h"
#include "inet/protocol/aggregation/base/AggregatorBase.h"

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
    aggregatedPacket->copyTags(*packet); // TODO: more complicated?
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

void AggregatorBase::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    if (!isAggregating())
        startAggregation(packet);
    else if (!aggregatorPolicy->isAggregatablePacket(aggregatedPacket, aggregatedSubpackets, packet)) {
        pushOrSendPacket(aggregatedPacket, outputGate, consumer);
        endAggregation(packet);
        startAggregation(packet);
    }
    continueAggregation(packet);
    EV_INFO << "Aggregating packet " << packet->getName() << " into packet " << aggregatedPacket->getName() << "." << endl;
    delete packet;
    updateDisplayString();
}

} // namespace inet


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

#include "inet/common/FlowTag.h"
#include "inet/common/PacketEventTag_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/queueing/timing/TimingMeasurementStarter.h"

namespace inet {
namespace queueing {

Define_Module(TimingMeasurementStarter);

static bool matchesString(cMatchExpression& matchExpression, const char *string)
{
    cMatchableString matchableString(string);
    return matchExpression.matches(&matchableString);
}

void TimingMeasurementStarter::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        packetFilter.setPattern(par("packetFilter"), par("packetDataFilter"));
        offset = b(par("offset"));
        length = b(par("length"));
        flowName = par("flowName");
        cMatchExpression measureMatcher;
        measureMatcher.setPattern(par("measure"), false, true, true);
        measureElapsedTime = matchesString(measureMatcher, "elapsedTime");
        measureDelayingTime = matchesString(measureMatcher, "delayingTime");
        measureQueueingTime = matchesString(measureMatcher, "queueingTime");
        measureProcessingTime = matchesString(measureMatcher, "processingTime");
        measureTransmissionTime = matchesString(measureMatcher, "transmissionTime");
        measurePropagationTime = matchesString(measureMatcher, "propagationTime");
        measurePacketEvents = matchesString(measureMatcher, "packetEvent");
    }
}

void TimingMeasurementStarter::processPacket(Packet *packet)
{
    if (packetFilter.matches(packet)) {
        startMeasurements(packet);
        startPacketFlow(this, packet, flowName);
    }
}

void TimingMeasurementStarter::startMeasurements(Packet *packet) const
{
    auto length = this->length == b(-1) ? packet->getTotalLength() : this->length;
    if (measureElapsedTime)
        startMeasurement<ElapsedTimeTag>(packet, offset, length, simTime());
    if (measureDelayingTime)
        startMeasurement<DelayingTimeTag>(packet, offset, length, 0);
    if (measureQueueingTime)
        startMeasurement<QueueingTimeTag>(packet, offset, length, 0);
    if (measureProcessingTime)
        startMeasurement<ProcessingTimeTag>(packet, offset, length, 0);
    if (measureTransmissionTime)
        startMeasurement<TransmissionTimeTag>(packet, offset, length, 0);
    if (measurePropagationTime)
        startMeasurement<PropagationTimeTag>(packet, offset, length, 0);
    if (measurePacketEvents) {
        EV_INFO << "Starting measurement on packet " << packet->getName() << ": "
                << "range (" << offset << ", " << offset + length << "), ";
        if (flowName != nullptr && *flowName != '\0')
            EV_INFO << "flowName = " << flowName << ", ";
        EV_INFO << "class = PacketEventTag" << std::endl;
        packet->addRegionTagsWhereAbsent<PacketEventTag>(offset, length);
    }
}

} // namespace queueing
} // namespace inet


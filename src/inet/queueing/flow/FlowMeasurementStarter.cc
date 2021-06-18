//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/flow/FlowMeasurementStarter.h"

#include "inet/common/FlowTag.h"
#include "inet/common/PacketEventTag_m.h"
#include "inet/common/TimeTag_m.h"

namespace inet {
namespace queueing {

Define_Module(FlowMeasurementStarter);

static bool matchesString(cMatchExpression& matchExpression, const char *string)
{
    cMatchableString matchableString(string);
    return matchExpression.matches(&matchableString);
}

void FlowMeasurementStarter::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        packetFilter.setExpression(par("packetFilter").objectValue());
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

cGate *FlowMeasurementStarter::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

void FlowMeasurementStarter::processPacket(Packet *packet)
{
    if (packetFilter.matches(packet)) {
        startMeasurements(packet);
        startPacketFlow(this, packet, flowName);
    }
}

void FlowMeasurementStarter::startMeasurements(Packet *packet) const
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
        EV_INFO << "Starting measurement on packet" << EV_FIELD(offset) << EV_FIELD(length);
        if (flowName != nullptr && *flowName != '\0')
            EV_INFO << EV_FIELD(flowName);
        EV_INFO << EV_FIELD(class, "PacketEventTag") << EV_FIELD(packet) << EV_ENDL;
        packet->addRegionTagsWhereAbsent<PacketEventTag>(offset, length);
    }
}

} // namespace queueing
} // namespace inet


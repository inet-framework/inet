//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/flow/FlowMeasurementRecorder.h"

#include "inet/common/FlowTag.h"

namespace inet {
namespace queueing {

simsignal_t FlowMeasurementRecorder::packetFlowMeasuredSignal = cComponent::registerSignal("packetFlowMeasured");

Define_Module(FlowMeasurementRecorder);

static bool matchesString(cMatchExpression& matchExpression, const char *string)
{
    cMatchableString matchableString(string);
    return matchExpression.matches(&matchableString);
}

cGate *FlowMeasurementRecorder::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

void FlowMeasurementRecorder::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        packetFilter.setExpression(par("packetFilter").objectValue());
        offset = b(par("offset"));
        length = b(par("length"));
        endMeasurement_ = par("endMeasurement");
        flowName = par("flowName");
        flowNameMatcher.setPattern(flowName, false, true, true);
        cMatchExpression measureMatcher;
        measureMatcher.setPattern(par("measure"), false, true, true);
        measureLifeTime = matchesString(measureMatcher, "lifeTime");
        measureElapsedTime = matchesString(measureMatcher, "elapsedTime");
        measureDelayingTime = matchesString(measureMatcher, "delayingTime");
        measureQueueingTime = matchesString(measureMatcher, "queueingTime");
        measureProcessingTime = matchesString(measureMatcher, "processingTime");
        measureTransmissionTime = matchesString(measureMatcher, "transmissionTime");
        measurePropagationTime = matchesString(measureMatcher, "propagationTime");
    }
}

void FlowMeasurementRecorder::processPacket(Packet *packet)
{
    if (packetFilter.matches(packet)) {
        makeMeasurements(packet);
        if (endMeasurement_)
            endMeasurements(packet);
    }
}

void FlowMeasurementRecorder::makeMeasurements(Packet *packet)
{
    emit(packetFlowMeasuredSignal, packet);
}

void FlowMeasurementRecorder::endMeasurements(Packet *packet)
{
    std::set<std::string> endedFlowNames;
    b length = this->length == b(-1) ? packet->getTotalLength() - offset : this->length;
    if (measureElapsedTime)
        endMeasurement<ElapsedTimeTag>(packet, offset, length);
    if (measureDelayingTime)
        endMeasurement<DelayingTimeTag>(packet, offset, length);
    if (measureQueueingTime)
        endMeasurement<QueueingTimeTag>(packet, offset, length);
    if (measureProcessingTime)
        endMeasurement<ProcessingTimeTag>(packet, offset, length);
    if (measureTransmissionTime)
        endMeasurement<TransmissionTimeTag>(packet, offset, length);
    if (measurePropagationTime)
        endMeasurement<PropagationTimeTag>(packet, offset, length);
    packet->mapAllRegionTagsForUpdate<FlowTag>(offset, length, [&] (b o, b l, const Ptr<FlowTag>& flowTag) {
        for (int i = 0; i < (int)flowTag->getNamesArraySize(); i++) {
            auto flowName = flowTag->getNames(i);
            cMatchableString matchableFlowName(flowName);
            if (flowNameMatcher.matches(&matchableFlowName)) {
                endedFlowNames.insert(flowName);
                flowTag->eraseNames(i);
                i--;
            }
        }
    });
    for (auto& flowName : endedFlowNames)
        endPacketFlow(this, packet, flowName.c_str());
}

} // namespace queueing
} // namespace inet


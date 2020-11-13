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

#include "inet/queueing/flow/FlowMeasurementRecorder.h"

#include "inet/common/FlowTag.h"

namespace inet {
namespace queueing {

simsignal_t FlowMeasurementRecorder::lifeTimeSignal = cComponent::registerSignal("lifeTime");
simsignal_t FlowMeasurementRecorder::elapsedTimeSignal = cComponent::registerSignal("elapsedTime");
simsignal_t FlowMeasurementRecorder::delayingTimeSignal = cComponent::registerSignal("delayingTime");
simsignal_t FlowMeasurementRecorder::queueingTimeSignal = cComponent::registerSignal("queueingTime");
simsignal_t FlowMeasurementRecorder::processingTimeSignal = cComponent::registerSignal("processingTime");
simsignal_t FlowMeasurementRecorder::transmissionTimeSignal = cComponent::registerSignal("transmissionTime");
simsignal_t FlowMeasurementRecorder::propagationTimeSignal = cComponent::registerSignal("propagationTime");

Define_Module(FlowMeasurementRecorder);

static bool matchesString(cMatchExpression& matchExpression, const char *string)
{
    cMatchableString matchableString(string);
    return matchExpression.matches(&matchableString);
}

void FlowMeasurementRecorder::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        packetFilter.setPattern(par("packetFilter"), par("packetDataFilter"));
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

void FlowMeasurementRecorder::makeMeasurement(Packet *packet, b offset, b length, const char *flowName, simsignal_t signal, simtime_t value)
{
    EV_INFO << "Making measurement on packet" << EV_FIELD(offset) << EV_FIELD(length);
    if (flowName != nullptr && *flowName != '\0')
        EV_INFO << EV_FIELD(flowName);
    EV_INFO << EV_FIELD(signal, cComponent::getSignalName(signal)) << EV_FIELD(value) << EV_FIELD(packet) << EV_ENDL;
    cNamedObject details(flowName);
    emit(signal, value, &details);
}

void FlowMeasurementRecorder::makeMeasurements(Packet *packet)
{
    b length = this->length == b(-1) ? packet->getTotalLength() - offset : this->length;
    if (measureLifeTime)
        packet->mapAllRegionTags<CreationTimeTag>(offset, length, [&] (b o, b l, const Ptr<const CreationTimeTag>& timeTag) {
            makeMeasurement(packet, o, l, nullptr, lifeTimeSignal, simTime() - timeTag->getCreationTime());
        });
    if (measureElapsedTime) {
        packet->mapAllRegionTags<ElapsedTimeTag>(offset, length, [&] (b o, b l, const Ptr<const ElapsedTimeTag>& timeTag) {
            for (int i = 0; i < (int)timeTag->getTotalTimesArraySize(); i++) {
                auto flowName = timeTag->getFlowNames(i);
                cMatchableString matchableFlowName(flowName);
                if (flowNameMatcher.matches(&matchableFlowName))
                    makeMeasurement(packet, o, l, flowName, elapsedTimeSignal, simTime() - timeTag->getTotalTimes(i));
            }
        });
    }
    if (measureDelayingTime)
        makeMeasurement<DelayingTimeTag>(packet, offset, length, delayingTimeSignal);
    if (measureQueueingTime)
        makeMeasurement<QueueingTimeTag>(packet, offset, length, queueingTimeSignal);
    if (measureProcessingTime)
        makeMeasurement<ProcessingTimeTag>(packet, offset, length, processingTimeSignal);
    if (measureTransmissionTime)
        makeMeasurement<TransmissionTimeTag>(packet, offset, length, transmissionTimeSignal);
    if (measurePropagationTime)
        makeMeasurement<PropagationTimeTag>(packet, offset, length, propagationTimeSignal);
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


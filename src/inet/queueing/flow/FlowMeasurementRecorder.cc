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

simsignal_t FlowMeasurementRecorder::packetFlowMeasuredSignal = cComponent::registerSignal("packetFlowMeasured");

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

void FlowMeasurementRecorder::makeMeasurement(Packet *packet, b offset, b length, const char *flowName, simsignal_t bitSignal, simsignal_t bitPerRegionSignal, simsignal_t packetPerBitSignal, simsignal_t packetPerRegionSignal, simtime_t bitValue, simtime_t packetValue)
{
    EV_INFO << "Making measurement on packet" << EV_FIELD(offset) << EV_FIELD(length);
    if (flowName != nullptr && *flowName != '\0')
        EV_INFO << EV_FIELD(flowName);
    EV_INFO << EV_FIELD(bitSignal, cComponent::getSignalName(bitSignal)) << EV_FIELD(bitValue) << EV_FIELD(packet) << EV_ENDL;
    cNamedObject details(flowName);
    // TODO: use weighted value when available in omnetpp
    for (int i = 0; i < length.get(); i++) {
        emit(bitSignal, bitValue, &details);
        if (packetPerBitSignal != -1)
            emit(packetPerBitSignal, packetValue, &details);
    }
    emit(bitPerRegionSignal, bitValue, &details);
    if (packetPerRegionSignal != -1)
        emit(packetPerRegionSignal, packetValue, &details);
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


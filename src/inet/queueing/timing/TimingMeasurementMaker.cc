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
#include "inet/queueing/timing/TimingMeasurementMaker.h"

namespace inet {
namespace queueing {

simsignal_t TimingMeasurementMaker::lifeTimeSignal = cComponent::registerSignal("lifeTime");
simsignal_t TimingMeasurementMaker::elapsedTimeSignal = cComponent::registerSignal("elapsedTime");
simsignal_t TimingMeasurementMaker::delayingTimeSignal = cComponent::registerSignal("delayingTime");
simsignal_t TimingMeasurementMaker::queueingTimeSignal = cComponent::registerSignal("queueingTime");
simsignal_t TimingMeasurementMaker::processingTimeSignal = cComponent::registerSignal("processingTime");
simsignal_t TimingMeasurementMaker::transmissionTimeSignal = cComponent::registerSignal("transmissionTime");
simsignal_t TimingMeasurementMaker::propagationTimeSignal = cComponent::registerSignal("propagationTime");

Define_Module(TimingMeasurementMaker);

static bool matchesString(cMatchExpression& matchExpression, const char *string)
{
    cMatchableString matchableString(string);
    return matchExpression.matches(&matchableString);
}

void TimingMeasurementMaker::initialize(int stage)
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

void TimingMeasurementMaker::processPacket(Packet *packet)
{
    if (packetFilter.matches(packet)) {
        makeMeasurements(packet);
        if (endMeasurement_)
            endMeasurements(packet);
    }
}

void TimingMeasurementMaker::makeMeasurement(Packet *packet, b offset, b length, const char *flowName, simsignal_t signal, simtime_t value)
{
    EV_INFO << "Making measurement on packet " << packet->getName() << ": "
            << "range (" << offset << ", " << offset + length << "), ";
    if (flowName != nullptr && *flowName != '\0')
        EV_INFO << "flowName = " << flowName << ", ";
    EV_INFO << "signal = " << cComponent::getSignalName(signal) << ", "
            << "value = " << value << std::endl;
    cNamedObject details(flowName);
    emit(signal, value, &details);
}

void TimingMeasurementMaker::makeMeasurements(Packet *packet)
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

void TimingMeasurementMaker::endMeasurements(Packet *packet)
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


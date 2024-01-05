//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/flow/FlowMeasurementRecorder.h"

#include "inet/common/FlowTag.h"
#include "inet/common/PacketEventTag.h"

namespace inet {
namespace queueing {

simsignal_t FlowMeasurementRecorder::packetFlowMeasuredSignal = cComponent::registerSignal("packetFlowMeasured");

Define_Module(FlowMeasurementRecorder);

static bool matchesString(cMatchExpression& matchExpression, const char *string)
{
    cMatchableString matchableString(string);
    return matchExpression.matches(&matchableString);
}

FlowMeasurementRecorder::~FlowMeasurementRecorder()
{
    if (measurePacketEvent) {
        packetEventFile.closeArray();
        packetEventFile.close();
    }
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
        measurePacketEvent = matchesString(measureMatcher, "packetEvent");
        if (measurePacketEvent) {
            packetEventFile.open(par("packetEventFileName").stringValue(), std::ios::out);
            packetEventFile.openArray();
        }
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
    if (measurePacketEvent) {
        packetEventFile.openObject();
        packetEventFile.writeInt("eventNumber", cSimulation::getActiveSimulation()->getEventNumber());
        packetEventFile.writeRaw("simulationTime", simTime().str());
        packetEventFile.writeString("module", getFullPath());
        packetEventFile.writeInt("packetId", packet->getId());
        packetEventFile.writeInt("packetTreeId", packet->getTreeId());
        packetEventFile.writeString("packetName", packet->getName());
        std::stringstream s;
        packet->peekAll()->printToStream(s, 0);
        packetEventFile.writeString("packetData", s.str());
        packetEventFile.openArray("lifeTimes");
        packet->peekData()->mapAllTags<CreationTimeTag>(b(0), b(-1), [&] (b o, b l, const Ptr<const CreationTimeTag>& creationTimeTag) {
            simtime_t lifeTime = simTime() - creationTimeTag->getCreationTime();
            packetEventFile.openObject();
            packetEventFile.writeInt("offset", b(o).get());
            packetEventFile.writeInt("length", b(l).get());
            packetEventFile.writeRaw("lifeTime", lifeTime.str());
            packetEventFile.closeObject();
        });
        packetEventFile.closeArray();
        packetEventFile.openArray("packetEvents");
        packet->mapAllRegionTags<PacketEventTag>(offset, length, [&] (b o, b l, const Ptr<const PacketEventTag>& packetEventTag) {
            simtime_t totalDuration = 0;
            packetEventFile.openObject();
            packetEventFile.writeInt("offset", b(o).get());
            packetEventFile.writeInt("length", b(l).get());
            packetEventFile.openArray("events");
            for (int i = 0; i < packetEventTag->getPacketEventsArraySize(); i++) {
                auto packetEvent = packetEventTag->getPacketEvents(i);
                auto kind = packetEvent->getKind();
                auto kindName = cEnum::get("inet::PacketEventKind")->getStringFor(kind);
                auto bitDuration = packetEvent->getBitDuration();
                auto packetDuration = packetEvent->getPacketDuration();
                simtime_t duration = bitDuration * b(((PacketTransmittedEvent *)packetEvent)->getPacketLength()).get() + packetDuration;
                totalDuration += duration;
                packetEventFile.openObject();
                packetEventFile.writeInt("eventNumber", packetEvent->getEventNumber());
                packetEventFile.writeRaw("simulationTime", packetEvent->getSimulationTime().str());
                packetEventFile.writeString("type", kindName + 4);
                packetEventFile.writeString("module", packetEvent->getModulePath());
                packetEventFile.writeInt("packetLength", b(packetEvent->getPacketLength()).get());
                packetEventFile.writeRaw("duration", duration.str());
                if (kind == PEK_TRANSMITTED) {
                    auto packetTransmittedEvent = static_cast<const PacketTransmittedEvent *>(packetEvent);
                    packetEventFile.writeDouble("datarate", bps(packetTransmittedEvent->getDatarate()).get());
                }
                packetEventFile.closeObject();
            }
            packetEventFile.closeArray();
            packetEventFile.writeRaw("totalDuration", totalDuration.str());
            packetEventFile.closeObject();
        });
        packetEventFile.closeArray();
        packetEventFile.closeObject();
    }
    packet->mapAllRegionTagsForUpdate<FlowTag>(offset, length, [&] (b o, b l, const Ptr<FlowTag>& flowTag) {
        for (size_t i = 0; i < flowTag->getNamesArraySize(); i++) {
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


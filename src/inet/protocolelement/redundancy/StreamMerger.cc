//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/redundancy/StreamMerger.h"

#include "inet/common/stlutils.h"
#include "inet/common/SequenceNumberTag_m.h"
#include "inet/protocolelement/redundancy/StreamTag_m.h"

namespace inet {

Define_Module(StreamMerger);

void StreamMerger::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        mapping = check_and_cast<cValueMap *>(par("mapping").objectValue());
        bufferSize = par("bufferSize");
    }
}

void StreamMerger::handleParameterChange(const char *name)
{
    if (!strcmp(name, "mapping"))
        mapping = check_and_cast<cValueMap *>(par("mapping").objectValue());
}

cGate *StreamMerger::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

void StreamMerger::processPacket(Packet *packet)
{
    const auto& streamInd = packet->findTag<StreamInd>();
    if (streamInd != nullptr) {
        auto inputStreamName = streamInd->getStreamName();
        if (mapping->containsKey(inputStreamName)) {
            auto outputStreamName = mapping->get(inputStreamName).stringValue();
            auto& it = sequenceNumbers[outputStreamName];
            it.push_back(packet->getTag<SequenceNumberInd>()->getSequenceNumber());
            if (it.size() > bufferSize)
                it.erase(it.begin(), it.begin() + it.size() - bufferSize);
            if (*outputStreamName != '\0')
                packet->getTagForUpdate<StreamInd>()->setStreamName(outputStreamName);
            else {
                packet->removeTag<StreamInd>();
                packet->removeTag<SequenceNumberInd>();
            }
        }
    }
}

bool StreamMerger::matchesPacket(const Packet *packet) const
{
    const auto& streamInd = packet->findTag<StreamInd>();
    if (streamInd == nullptr)
        return true;
    else {
        auto inputStreamName = streamInd->getStreamName();
        if (!mapping->containsKey(inputStreamName))
            return true;
        else {
            auto outputStreamName = mapping->get(inputStreamName).stringValue();
            if (!matchesInputStream(inputStreamName))
                throw cRuntimeError("Unknown stream");
            else
                return matchesSequenceNumber(outputStreamName, packet->getTag<SequenceNumberInd>()->getSequenceNumber());
        }
    }
}

bool StreamMerger::matchesInputStream(const char *streamName) const
{
    return mapping->containsKey(streamName);
}

bool StreamMerger::matchesSequenceNumber(const char *streamName, int sequenceNumber) const
{
    auto it = sequenceNumbers.find(streamName);
    if (it == sequenceNumbers.end())
        return true;
    else
        return !contains(it->second, sequenceNumber);
}

} // namespace inet


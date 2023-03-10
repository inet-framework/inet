//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/redundancy/StreamIdentifier.h"

#include "inet/common/packet/PacketFilter.h"
#include "inet/common/SequenceNumberTag_m.h"
#include "inet/protocolelement/redundancy/StreamTag_m.h"

namespace inet {

Define_Module(StreamIdentifier);

void StreamIdentifier::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        hasSequenceNumbering = par("hasSequenceNumbering");
        mapping = check_and_cast<cValueArray *>(par("mapping").objectValue());
    }
}

void StreamIdentifier::handleParameterChange(const char *name)
{
    if (!strcmp(name, "mapping"))
        mapping = check_and_cast<cValueArray *>(par("mapping").objectValue());
}

cGate *StreamIdentifier::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

int StreamIdentifier::incrementSequenceNumber(const char *stream)
{
    auto it = sequenceNumbers.find(stream);
    if (it == sequenceNumbers.end())
        it = sequenceNumbers.insert({stream, 0}).first;
    return it->second++;
}

void StreamIdentifier::processPacket(Packet *packet)
{
    const char *streamName = nullptr;
    auto streamReq = packet->findTag<StreamReq>();
    if (streamReq != nullptr)
        streamName = streamReq->getStreamName();
    else {
        for (int i = 0; i < mapping->size(); i++) {
            auto element = check_and_cast<cValueMap *>(mapping->get(i).objectValue());
            PacketFilter packetFilter;
            auto packetPattern = element->containsKey("packetFilter") ? element->get("packetFilter") : cValue("*");
            packetFilter.setExpression(packetPattern);
            if (packetFilter.matches(packet)) {
                streamName = element->get("stream").stringValue();
                packet->addTag<StreamReq>()->setStreamName(streamName);
                auto sequenceNumber = incrementSequenceNumber(streamName);
                if (hasSequenceNumbering && element->containsKey("sequenceNumbering") && element->get("sequenceNumbering").boolValue())
                    packet->addTag<SequenceNumberReq>()->setSequenceNumber(sequenceNumber);
                break;
            }
        }
    }
    handlePacketProcessed(packet);
    updateDisplayString();
}

} // namespace inet


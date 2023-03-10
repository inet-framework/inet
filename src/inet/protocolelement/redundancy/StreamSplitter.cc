//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/redundancy/StreamSplitter.h"

#include "inet/protocolelement/redundancy/StreamTag_m.h"

namespace inet {

Define_Module(StreamSplitter);

void StreamSplitter::initialize(int stage)
{
    PacketDuplicatorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        mapping = check_and_cast<cValueMap *>(par("mapping").objectValue());
}

void StreamSplitter::handleParameterChange(const char *name)
{
    if (!strcmp(name, "mapping"))
        mapping = check_and_cast<cValueMap *>(par("mapping").objectValue());
}

cGate *StreamSplitter::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

void StreamSplitter::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    auto streamReq = packet->findTag<StreamReq>();
    if (streamReq != nullptr) {
        auto streamName = streamReq->getStreamName();
        if (mapping->containsKey(streamName)) {
            cValueArray *outputStreams = check_and_cast<cValueArray *>(mapping->get(streamName).objectValue());
            for (int i = 0; i < outputStreams->size(); i++) {
                const char *splitStreamName = outputStreams->get(i).stringValue();
                auto duplicate = packet->dup();
                duplicate->addTagIfAbsent<StreamReq>()->setStreamName(splitStreamName);
                pushOrSendPacket(duplicate, outputGate, consumer);
            }
            handlePacketProcessed(packet);
            delete packet;
        }
        else {
            handlePacketProcessed(packet);
            pushOrSendPacket(packet, outputGate, consumer);
        }
    }
    else {
        handlePacketProcessed(packet);
        pushOrSendPacket(packet, outputGate, consumer);
    }
    updateDisplayString();
}

int StreamSplitter::getNumPacketDuplicates(Packet *packet)
{
    auto streamReq = packet->findTag<StreamReq>();
    auto streamName = streamReq != nullptr ? streamReq->getStreamName() : "";
    if (mapping->containsKey(streamName)) {
        cValueArray *outputStreams = check_and_cast<cValueArray *>(mapping->get(streamName).objectValue());
        return outputStreams->size() - 1;
    }
    else
        return 0;
}

} // namespace inet


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

#include "inet/protocolelement/redundancy/StreamSplitter.h"

#include "inet/protocolelement/redundancy/StreamTag_m.h"

namespace inet {

Define_Module(StreamSplitter);

void StreamSplitter::initialize(int stage)
{
    PacketDuplicatorBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        streamMapping = check_and_cast<cValueMap *>(par("streamMapping").objectValue());
}

void StreamSplitter::handleParameterChange(const char *name)
{
    if (name != nullptr) {
        if (!strcmp(name, "streamMapping"))
            streamMapping = check_and_cast<cValueMap *>(par("streamMapping").objectValue());
   }
}

std::vector<cGate *> StreamSplitter::getRegistrationForwardingGates(cGate *gate)
{
    if (gate == outputGate)
        return std::vector<cGate *>({inputGate});
    else if (gate == inputGate)
        return std::vector<cGate *>({outputGate});
    else
        throw cRuntimeError("Unknown gate");
}

void StreamSplitter::pushPacket(Packet *packet, cGate *gate)
{
    Enter_Method("pushPacket");
    take(packet);
    auto streamReq = packet->findTag<StreamReq>();
    auto streamName = streamReq != nullptr ? streamReq->getStreamName() : "";
    if (streamMapping->containsKey(streamName)) {
        cValueArray *outputStreams = check_and_cast<cValueArray *>(streamMapping->get(streamName).objectValue());
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
    updateDisplayString();
}

int StreamSplitter::getNumPacketDuplicates(Packet *packet)
{
    auto streamReq = packet->findTag<StreamReq>();
    auto streamName = streamReq != nullptr ? streamReq->getStreamName() : "";
    if (streamMapping->containsKey(streamName)) {
        cValueArray *outputStreams = check_and_cast<cValueArray *>(streamMapping->get(streamName).objectValue());
        return outputStreams->size() - 1;
    }
    else
        return 0;
}

} // namespace inet


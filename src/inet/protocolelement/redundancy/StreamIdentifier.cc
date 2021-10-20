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

#include "inet/protocolelement/redundancy/StreamIdentifier.h"

#include "inet/common/packet/PacketFilter.h"
#include "inet/protocolelement/redundancy/SequenceNumberTag_m.h"
#include "inet/protocolelement/redundancy/StreamTag_m.h"

namespace inet {

Define_Module(StreamIdentifier);

void StreamIdentifier::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        mapping = check_and_cast<cValueArray *>(par("mapping").objectValue());
}

void StreamIdentifier::handleParameterChange(const char *name)
{
    if (name != nullptr) {
        if (!strcmp(name, "mapping"))
            mapping = check_and_cast<cValueArray *>(par("mapping").objectValue());
   }
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
            auto packetPattern = element->containsKey("packetFilter") ? element->get("packetFilter").stringValue() : "true";
            packetFilter.setExpression(packetPattern);
            if (packetFilter.matches(packet)) {
                streamName = element->get("stream").stringValue();
                packet->addTag<StreamReq>()->setStreamName(streamName);
                auto sequenceNumber = incrementSequenceNumber(streamName);
                packet->addTag<SequenceNumberReq>()->setSequenceNumber(sequenceNumber);
                break;
            }
        }
    }
    if (streamName != nullptr) {
        if (auto dispatchProtocolReq = packet->findTag<DispatchProtocolReq>()) {
            auto encapsulationReq = packet->addTagIfAbsent<EncapsulationProtocolReq>();
            encapsulationReq->insertProtocols(0, dispatchProtocolReq->getProtocol());
        }
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ieee8021rTag);
    }
    handlePacketProcessed(packet);
    updateDisplayString();
}

} // namespace inet


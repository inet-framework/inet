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

#include "inet/protocolelement/redundancy/StreamEncoder.h"

#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/protocolelement/redundancy/StreamTag_m.h"

namespace inet {

Define_Module(StreamEncoder);

void StreamEncoder::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        streamNameToVlanIdMapping = check_and_cast<cValueMap *>(par("streamNameToVlanIdMapping").objectValue());
}

void StreamEncoder::handleParameterChange(const char *name)
{
    if (name != nullptr) {
        if (!strcmp(name, "streamNameToVlanIdMapping"))
            streamNameToVlanIdMapping = check_and_cast<cValueMap *>(par("streamNameToVlanIdMapping").objectValue());
   }
}

std::vector<cGate *> StreamEncoder::getRegistrationForwardingGates(cGate *gate)
{
    if (gate == outputGate)
        return std::vector<cGate *>({inputGate});
    else if (gate == inputGate)
        return std::vector<cGate *>({outputGate});
    else
        throw cRuntimeError("Unknown gate");
}

void StreamEncoder::processPacket(Packet *packet)
{
    auto streamReq = packet->getTag<StreamReq>();
    auto streamName = streamReq->getStreamName();
    auto vlanId = streamNameToVlanIdMapping->get(streamName).intValue();
    packet->addTagIfAbsent<VlanReq>()->setVlanId(vlanId);
    std::string packetName = packet->getName();
    packetName = packetName + "-" + streamName;
    packet->setName(packetName.c_str());
}

} // namespace inet


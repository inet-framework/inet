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

#include "inet/protocolelement/redundancy/StreamDecoder.h"

#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/protocolelement/redundancy/StreamTag_m.h"

namespace inet {

Define_Module(StreamDecoder);

void StreamDecoder::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        streamMappings = check_and_cast<cValueArray *>(par("streamMappings").objectValue());
        interfaceTable.reference(this, "interfaceTableModule", true);
    }
}

void StreamDecoder::handleParameterChange(const char *name)
{
    if (name != nullptr) {
        if (!strcmp(name, "streamMappings"))
            streamMappings = check_and_cast<cValueArray *>(par("streamMappings").objectValue());
   }
}

std::vector<cGate *> StreamDecoder::getRegistrationForwardingGates(cGate *gate)
{
    if (gate == outputGate)
        return std::vector<cGate *>({inputGate});
    else if (gate == inputGate)
        return std::vector<cGate *>({outputGate});
    else
        throw cRuntimeError("Unknown gate");
}

void StreamDecoder::processPacket(Packet *packet)
{
    auto vlanInd = packet->getTag<VlanInd>();
    auto interfaceInd = packet->getTag<InterfaceInd>();
    auto interfaceName = interfaceTable->getInterfaceById(interfaceInd->getInterfaceId())->getInterfaceName();
    for (int i = 0; i < streamMappings->size(); i++) {
        auto streamMapping = check_and_cast<cValueMap *>(streamMappings->get(i).objectValue());
        auto interfaceNamePattern = streamMapping->containsKey("interface") ? streamMapping->get("interface").stringValue() : "*";
        auto vlanId = streamMapping->containsKey("vlan") ? std::to_string(streamMapping->get("vlan").intValue()) : "*";
        cPatternMatcher interfaceNameMatcher;
        interfaceNameMatcher.setPattern(interfaceNamePattern, false, false, false);
        if (interfaceNameMatcher.matches(interfaceName) && vlanId == std::to_string(vlanInd->getVlanId())) {
            auto streamName = streamMapping->get("stream");
            packet->addTag<StreamInd>()->setStreamName(streamName);
            break;
        }
    }
    handlePacketProcessed(packet);
    updateDisplayString();
}

} // namespace inet


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
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/protocolelement/redundancy/StreamTag_m.h"

namespace inet {

Define_Module(StreamDecoder);

void StreamDecoder::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        interfaceTable.reference(this, "interfaceTableModule", true);
    else if (stage == INITSTAGE_QUEUEING)
        configureStreams();
}

void StreamDecoder::handleParameterChange(const char *name)
{
    if (name != nullptr) {
        if (!strcmp(name, "mapping"))
            configureStreams();
   }
}

void StreamDecoder::configureStreams()
{
    auto mappingParameter = check_and_cast<cValueArray *>(par("mapping").objectValue());
    L3AddressResolver addressResolver;
    for (int i = 0; i < mappingParameter->size(); i++) {
        auto element = check_and_cast<cValueMap *>(mappingParameter->get(i).objectValue());
        Mapping mapping;
        L3Address l3Address;
        L3AddressResolver addressResolver;
        if (element->containsKey("source")) {
            addressResolver.tryResolve(element->get("source").stringValue(), l3Address, L3AddressResolver::ADDR_MAC);
            mapping.source = l3Address.toMac();
        }
        if (element->containsKey("destination")) {
            addressResolver.tryResolve(element->get("destination").stringValue(), l3Address, L3AddressResolver::ADDR_MAC);
            mapping.destination = l3Address.toMac();
        }
        mapping.vlanId = element->containsKey("vlan") ? element->get("vlan").intValue() : -1;
        mapping.pcp = element->containsKey("pcp") ? element->get("pcp").intValue() : -1;
        mapping.name = element->get("stream").stringValue();
        if (element->containsKey("interface")) {
            auto interfaceNamePattern = element->get("interface").stringValue();
            mapping.interfaceNameMatcher = new cPatternMatcher();
            mapping.interfaceNameMatcher->setPattern(interfaceNamePattern, false, false, false);
        }
        mappings.push_back(mapping);
    }
}

cGate *StreamDecoder::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

void StreamDecoder::processPacket(Packet *packet)
{
    auto vlanInd = packet->findTag<VlanInd>();
    for (auto& stream : mappings) {
        bool matches = true;
        const auto& macAddressInd = packet->findTag<MacAddressInd>();
        const auto& vlanInd = packet->findTag<VlanInd>();
        const auto& interfaceInd = packet->findTag<InterfaceInd>();
        if (stream.interfaceNameMatcher != nullptr) {
            auto interfaceName = interfaceInd != nullptr ? interfaceTable->getInterfaceById(interfaceInd->getInterfaceId())->getInterfaceName() : nullptr;
            matches &= interfaceInd != nullptr && stream.interfaceNameMatcher->matches(interfaceName);
        }
        if (!stream.source.isUnspecified())
            matches &= macAddressInd != nullptr && macAddressInd->getSrcAddress() == stream.source;
        if (!stream.destination.isUnspecified())
            matches &= macAddressInd != nullptr && macAddressInd->getDestAddress() == stream.destination;
        if (stream.vlanId != -1)
            matches &= vlanInd != nullptr && vlanInd->getVlanId() == stream.vlanId;
        if (stream.pcp != -1)
            matches &= true; // TODO
        if (matches) {
            packet->addTag<StreamInd>()->setStreamName(stream.name.c_str());
            break;
        }
    }
    handlePacketProcessed(packet);
    updateDisplayString();
}

} // namespace inet


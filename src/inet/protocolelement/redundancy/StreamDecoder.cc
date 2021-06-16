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
        if (!strcmp(name, "streams"))
            configureStreams();
   }
}

void StreamDecoder::configureStreams()
{
    auto streamConfigurations = check_and_cast<cValueArray *>(par("streamMappings").objectValue());
    L3AddressResolver addressResolver;
    for (int i = 0; i < streamConfigurations->size(); i++) {
        auto streamConfiguration = check_and_cast<cValueMap *>(streamConfigurations->get(i).objectValue());
        Stream stream;
        L3Address l3Address;
        L3AddressResolver addressResolver;
        if (streamConfiguration->containsKey("source")) {
            addressResolver.tryResolve(streamConfiguration->get("source").stringValue(), l3Address, L3AddressResolver::ADDR_MAC);
            stream.source = l3Address.toMac();
        }
        if (streamConfiguration->containsKey("destination")) {
            addressResolver.tryResolve(streamConfiguration->get("destination").stringValue(), l3Address, L3AddressResolver::ADDR_MAC);
            stream.destination = l3Address.toMac();
        }
        stream.vlanId = streamConfiguration->containsKey("vlan") ? streamConfiguration->get("vlan").intValue() : -1;
        stream.pcp = streamConfiguration->containsKey("pcp") ? streamConfiguration->get("pcp").intValue() : -1;
        stream.name = streamConfiguration->get("stream").stringValue();
        if (streamConfiguration->containsKey("interface")) {
            auto interfaceNamePattern = streamConfiguration->get("interface").stringValue();
            stream.interfaceNameMatcher = new cPatternMatcher();
            stream.interfaceNameMatcher->setPattern(interfaceNamePattern, false, false, false);
        }
        streams.push_back(stream);
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
    for (auto& stream : streams) {
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


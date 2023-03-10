//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/redundancy/StreamDecoder.h"

#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/common/PcpTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/protocolelement/redundancy/StreamTag_m.h"

namespace inet {

Define_Module(StreamDecoder);

StreamDecoder::~StreamDecoder()
{
    for (auto& it : mappings)
        delete it.interfaceNameMatcher;
}

void StreamDecoder::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        interfaceTable.reference(this, "interfaceTableModule", true);
    else if (stage == INITSTAGE_QUEUEING)
        configureMappings();
}

void StreamDecoder::handleParameterChange(const char *name)
{
    if (!strcmp(name, "mapping"))
        configureMappings();
}

void StreamDecoder::configureMappings()
{
    auto mappingParameter = check_and_cast<cValueArray *>(par("mapping").objectValue());
    L3AddressResolver addressResolver;
    mappings.resize(mappingParameter->size());
    for (int i = 0; i < mappingParameter->size(); i++) {
        auto element = check_and_cast<cValueMap *>(mappingParameter->get(i).objectValue());
        Mapping& mapping = mappings[i];
        cValue packetPattern = element->containsKey("packetFilter") ? element->get("packetFilter") : cValue("*");
        mapping.packetFilter.setExpression(packetPattern);
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
        mapping.stream = element->get("stream").stringValue();
        if (element->containsKey("interface")) {
            auto interfaceNamePattern = element->get("interface").stringValue();
            mapping.interfaceNameMatcher = new cPatternMatcher();
            mapping.interfaceNameMatcher->setPattern(interfaceNamePattern, false, false, false);
        }
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
    for (auto& mapping : mappings) {
        bool matches = true;
        const auto& macAddressInd = packet->findTag<MacAddressInd>();
        const auto& pcpInd = packet->findTag<PcpInd>();
        const auto& vlanInd = packet->findTag<VlanInd>();
        const auto& interfaceInd = packet->findTag<InterfaceInd>();
        matches &= mapping.packetFilter.matches(packet);
        if (mapping.interfaceNameMatcher != nullptr) {
            auto interfaceName = interfaceInd != nullptr ? interfaceTable->getInterfaceById(interfaceInd->getInterfaceId())->getInterfaceName() : nullptr;
            matches &= interfaceInd != nullptr && mapping.interfaceNameMatcher->matches(interfaceName);
        }
        if (!mapping.source.isUnspecified())
            matches &= macAddressInd != nullptr && macAddressInd->getSrcAddress() == mapping.source;
        if (!mapping.destination.isUnspecified())
            matches &= macAddressInd != nullptr && macAddressInd->getDestAddress() == mapping.destination;
        if (mapping.vlanId != -1)
            matches &= vlanInd != nullptr && vlanInd->getVlanId() == mapping.vlanId;
        if (mapping.pcp != -1)
            matches &= pcpInd != nullptr && pcpInd->getPcp() == mapping.pcp;
        if (matches) {
            packet->addTag<StreamInd>()->setStreamName(mapping.stream.c_str());
            break;
        }
    }
    handlePacketProcessed(packet);
    updateDisplayString();
}

} // namespace inet


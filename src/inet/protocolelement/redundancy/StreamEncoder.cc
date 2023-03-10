//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/redundancy/StreamEncoder.h"

#include "inet/common/ProtocolUtils.h"
#include "inet/common/SequenceNumberTag_m.h"
#include "inet/linklayer/common/PcpTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/protocolelement/redundancy/StreamTag_m.h"

namespace inet {

Define_Module(StreamEncoder);

void StreamEncoder::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_QUEUEING)
        configureMappings();
}

void StreamEncoder::handleParameterChange(const char *name)
{
    if (!strcmp(name, "mapping"))
        configureMappings();
}

void StreamEncoder::configureMappings()
{
    auto mappingParameter = check_and_cast<cValueArray *>(par("mapping").objectValue());
    mappings.resize(mappingParameter->size());
    for (int i = 0; i < mappingParameter->size(); i++) {
        auto element = check_and_cast<cValueMap *>(mappingParameter->get(i).objectValue());
        Mapping& mapping = mappings[i];
        mapping.vlanId = element->containsKey("vlan") ? element->get("vlan").intValue() : -1;
        mapping.pcp = element->containsKey("pcp") ? element->get("pcp").intValue() : -1;
        mapping.stream = element->get("stream").stringValue();
    }
}

cGate *StreamEncoder::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

void StreamEncoder::processPacket(Packet *packet)
{
    auto streamReq = packet->findTag<StreamReq>();
    if (streamReq != nullptr) {
        auto streamName = streamReq->getStreamName();
        for (auto& mapping : mappings) {
            if (!strcmp(mapping.stream.c_str(), streamName)) {
                if (mapping.pcp != -1)
                    packet->addTagIfAbsent<PcpReq>()->setPcp(mapping.pcp);
                if (mapping.vlanId != -1)
                    packet->addTagIfAbsent<VlanReq>()->setVlanId(mapping.vlanId);
                if (packet->hasTag<SequenceNumberReq>())
                    ensureEncapsulationProtocolReq(packet, &Protocol::ieee8021rTag);
                if (mapping.pcp != -1 || mapping.vlanId != -1)
                    ensureEncapsulationProtocolReq(packet, &Protocol::ieee8021qCTag);
                setDispatchProtocol(packet);
                break;
            }
        }
    }
}

} // namespace inet


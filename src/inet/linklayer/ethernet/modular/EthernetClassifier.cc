//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/physicallayer/wired/ethernet/EthernetPhyHeader_m.h"
#include "inet/protocolelement/fragmentation/tag/FragmentTag_m.h"
#include "inet/queueing/function/PacketClassifierFunction.h"

namespace inet {

using namespace inet::physicallayer;

static int classifyPacketByCVlanReq(Packet *packet)
{
    const auto& vlanReq = packet->findTag<CVlanReq>();
    return vlanReq != nullptr ? vlanReq->getVlanId() : 0;
}

Register_Packet_Classifier_Function(PacketCVlanReqClassifier, classifyPacketByCVlanReq);

static int classifyPacketByCVlanInd(Packet *packet)
{
    const auto& vlanInd = packet->findTag<CVlanInd>();
    return vlanInd != nullptr ? vlanInd->getVlanId() : 0;
}

Register_Packet_Classifier_Function(PacketCVlanIndClassifier, classifyPacketByCVlanInd);

static int classifyPacketBySVlanReq(Packet *packet)
{
    const auto& vlanReq = packet->findTag<SVlanReq>();
    return vlanReq != nullptr ? vlanReq->getVlanId() : 0;
}

Register_Packet_Classifier_Function(PacketSVlanReqClassifier, classifyPacketBySVlanReq);

static int classifyPacketBySVlanInd(Packet *packet)
{
    const auto& vlanInd = packet->findTag<SVlanInd>();
    return vlanInd != nullptr ? vlanInd->getVlanId() : 0;
}

Register_Packet_Classifier_Function(PacketSVlanIndClassifier, classifyPacketBySVlanInd);

static int classifyPacketByFragmentTag(Packet *packet)
{
    auto fragmentTag = packet->findTag<FragmentTag>();
    return fragmentTag == nullptr ? 0 : 1;
}

Register_Packet_Classifier_Function(PacketFragmentTagClassifier, classifyPacketByFragmentTag);

static int classifyPacketByEthernetPreambleType(Packet *packet)
{
    auto header = packet->peekAtFront<EthernetPhyHeaderBase>();
    if (auto fragmentHeader = dynamicPtrCast<const EthernetFragmentPhyHeader>(header))
        return fragmentHeader->getPreambleType() == SFD ? 0 : 1;
    else
        return 0;
}

Register_Packet_Classifier_Function(PacketEthernetPreambleTypeClassifier, classifyPacketByEthernetPreambleType);

static int classifyPacketByLlcProtocol(Packet *packet)
{
    auto protocolTag = packet->findTag<PacketProtocolTag>();
    return (protocolTag != nullptr && *protocolTag->getProtocol() == Protocol::ieee8022llc) ? 1 : 0;
}

Register_Packet_Classifier_Function(EthernetLlcClassifier, classifyPacketByLlcProtocol);

} // namespace inet


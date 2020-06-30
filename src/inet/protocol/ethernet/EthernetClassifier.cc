//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/linklayer/ethernet/EtherPhyFrame_m.h"
#include "inet/protocol/fragmentation/tag/FragmentTag_m.h"
#include "inet/queueing/function/PacketClassifierFunction.h"

namespace inet {

static int classifyPacketByVlanReq(Packet *packet)
{
    const auto& vlanReq = packet->findTag<VlanReq>();
    return vlanReq != nullptr ? vlanReq->getVlanId() : 0;
}

Register_Packet_Classifier_Function(PacketVlanReqClassifier, classifyPacketByVlanReq);

static int classifyPacketByVlanInd(Packet *packet)
{
    const auto& vlanInd = packet->findTag<VlanInd>();
    return vlanInd != nullptr ? vlanInd->getVlanId() : 0;
}

Register_Packet_Classifier_Function(PacketVlanIndClassifier, classifyPacketByVlanInd);

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
    return (protocolTag != nullptr && *protocolTag->getProtocol() == Protocol::ieee8022) ? 1 : 0;
}

Register_Packet_Classifier_Function(EthernetLlcClassifier, classifyPacketByLlcProtocol);

} // namespace inet


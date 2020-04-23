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

#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/ethernet/EtherPhyFrame_m.h"
#include "inet/networklayer/common/DscpTag_m.h"
#include "inet/queueing/function/PacketClassifierFunction.h"

namespace inet {

static int classifyEthernetExpressOverNormal(Packet *packet)
{
    auto dscpInd = packet->findTag<DscpInd>();
    return dscpInd != nullptr ? dscpInd->getDifferentiatedServicesCodePoint() : 0;
}

Register_Packet_Classifier_Function(EthernetExpressOverNormalClassifier, classifyEthernetExpressOverNormal);

static int classifyEthernetPreamble(Packet *packet)
{
    auto header = packet->peekAtFront<EthernetPhyHeaderBase>();
    if (auto fragmentHeader = dynamicPtrCast<const EthernetFragmentPhyHeader>(header))
        return fragmentHeader->getPreambleType() == SFD ? 0 : 1;
    else
        return 0;
}

Register_Packet_Classifier_Function(EthernetPreambleClassifier, classifyEthernetPreamble);

static int classifyPacketUserPriorityInd(Packet *packet)
{
    auto userPriorityInd = packet->findTag<UserPriorityInd>();
    return userPriorityInd != nullptr ? userPriorityInd->getUserPriority() : 0;
}

Register_Packet_Classifier_Function(PacketUserPriorityIndClassifier, classifyPacketUserPriorityInd);

} // namespace inet


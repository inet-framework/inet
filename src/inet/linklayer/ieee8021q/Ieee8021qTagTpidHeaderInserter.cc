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

#include "inet/linklayer/ieee8021q/Ieee8021qTagTpidHeaderInserter.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/linklayer/ieee8021q/Ieee8021qTagHeader_m.h"

namespace inet {

Define_Module(Ieee8021qTagTpidHeaderInserter);

void Ieee8021qTagTpidHeaderInserter::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *vlanTagType = par("vlanTagType");
        if (!strcmp("s", vlanTagType))
            tpid = 0x88A8;
        else if (!strcmp("c", vlanTagType))
            tpid = 0x8100;
        else
            throw cRuntimeError("Unknown tag type");
        const char *nextProtocolAsString = par("nextProtocol");
        if (*nextProtocolAsString != '\0')
            nextProtocol = Protocol::getProtocol(nextProtocolAsString);
        defaultVlanId = par("defaultVlanId");
        defaultUserPriority = par("defaultUserPriority");
    }
}

void Ieee8021qTagTpidHeaderInserter::processPacket(Packet *packet)
{
    auto header = makeShared<Ieee8021qTagTpidHeader>();
    header->setTpid(tpid);
    auto userPriorityReq = packet->removeTagIfPresent<UserPriorityReq>();
    auto userPriority = userPriorityReq != nullptr ? userPriorityReq->getUserPriority() : defaultUserPriority;
    if (userPriority != -1) {
        EV_INFO << "Setting PCP" << EV_FIELD(pcp, userPriority) << EV_ENDL;
        header->setPcp(userPriority);
        packet->addTagIfAbsent<UserPriorityInd>()->setUserPriority(userPriority);
    }
    auto vlanReq = packet->removeTagIfPresent<VlanReq>();
    auto vlanId = vlanReq != nullptr ? vlanReq->getVlanId() : defaultVlanId;
    if (vlanId != -1) {
        EV_INFO << "Setting VID" << EV_FIELD(vid, vlanId) << EV_ENDL;
        header->setVid(vlanId);
        packet->addTagIfAbsent<VlanInd>()->setVlanId(vlanId);
    }
    packet->insertAtFront(header);
    auto& packetProtocolTag = packet->getTagForUpdate<PacketProtocolTag>();
    packetProtocolTag->setFrontOffset(packetProtocolTag->getFrontOffset() + header->getChunkLength());
    if (nextProtocol != nullptr)
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(nextProtocol);
}

} // namespace inet


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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include <algorithm>
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/protocol/ieee8021q/Ieee8021qInserter.h"

namespace inet {

Define_Module(Ieee8021qInserter);

void Ieee8021qInserter::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *vlanTagType = par("vlanTagType");
        if (!strcmp("s", vlanTagType))
            etherType = 0x88A8;
        else if (!strcmp("c", vlanTagType))
            etherType = 0x8100;
        else
            throw cRuntimeError("Unknown tag type");
    }
}

void Ieee8021qInserter::processPacket(Packet *packet)
{
    auto vlanHeader = makeShared<Ieee8021qHeader>();
    vlanHeader->setTypeOrLength(etherType);
    auto userPriorityReq = packet->removeTagIfPresent<UserPriorityReq>();
    if (userPriorityReq != nullptr) {
        auto userPriority = userPriorityReq->getUserPriority();
        EV_INFO << "Setting PCP to " << userPriority << ".\n";
        vlanHeader->setPcp(userPriority);
        packet->addTagIfAbsent<UserPriorityInd>()->setUserPriority(userPriority);
    }
    auto vlanReq = packet->removeTagIfPresent<VlanReq>();
    if (vlanReq != nullptr) {
        auto vlanId = vlanReq->getVlanId();
        EV_INFO << "Setting VLAN ID to " << vlanId << ".\n";
        vlanHeader->setVid(vlanId);
        packet->addTagIfAbsent<VlanInd>()->setVlanId(vlanId);
    }
    packet->insertAtFront(vlanHeader);
    auto packetProtocolTag = packet->getTag<PacketProtocolTag>();
    packetProtocolTag->setFrontOffset(packetProtocolTag->getFrontOffset() + vlanHeader->getChunkLength());
}

} // namespace inet


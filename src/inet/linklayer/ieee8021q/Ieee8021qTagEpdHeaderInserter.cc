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

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/linklayer/ieee8021q/Ieee8021qTagEpdHeaderInserter.h"
#include "inet/linklayer/ieee8021q/Ieee8021qTagHeader_m.h"

namespace inet {

Define_Module(Ieee8021qTagEpdHeaderInserter);

void Ieee8021qTagEpdHeaderInserter::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *vlanTagType = par("vlanTagType");
        if (!strcmp("s", vlanTagType))
            qtagProtocol = &Protocol::ieee8021qSTag;
        else if (!strcmp("c", vlanTagType))
            qtagProtocol = &Protocol::ieee8021qCTag;
        else
            throw cRuntimeError("Unknown tag type");
        registerService(Protocol::ethernetMac, inputGate, nullptr);
    }
}

void Ieee8021qTagEpdHeaderInserter::processPacket(Packet *packet)
{
    auto header = makeShared<Ieee8021qTagEpdHeader>();
    auto userPriorityReq = packet->removeTagIfPresent<UserPriorityReq>();
    if (userPriorityReq != nullptr) {
        auto userPriority = userPriorityReq->getUserPriority();
        EV_INFO << "Setting PCP to " << userPriority << ".\n";
        header->setPcp(userPriority);
        packet->addTagIfAbsent<UserPriorityInd>()->setUserPriority(userPriority);
    }
    auto vlanReq = packet->removeTagIfPresent<VlanReq>();
    if (vlanReq != nullptr) {
        auto vlanId = vlanReq->getVlanId();
        EV_INFO << "Setting VLAN ID to " << vlanId << ".\n";
        header->setVid(vlanId);
        packet->addTagIfAbsent<VlanInd>()->setVlanId(vlanId);
    }
    auto& packetProtocolTag = packet->getTagForUpdate<PacketProtocolTag>();
    auto protocol = packetProtocolTag->getProtocol();
    if (protocol == &Protocol::ieee8022llc)
        header->setTypeOrLength(packet->getByteLength());
    else
        header->setTypeOrLength(ProtocolGroup::ethertype.findProtocolNumber(protocol));
    packet->insertAtFront(header);
    packetProtocolTag->setProtocol(qtagProtocol);
    packetProtocolTag->setFrontOffset(b(0));
}

} // namespace inet


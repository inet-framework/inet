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
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/protocol/ieee8021q/Ieee8021qTagger.h"

namespace inet {

Define_Module(Ieee8021qTagger);

void Ieee8021qTagger::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *vlanTagType = par("vlanTagType");
        if (!strcmp("s", vlanTagType))
            etherType = 0x88A8;
        else if (!strcmp("c", vlanTagType))
            etherType = 0x8100;
        else
            throw cRuntimeError("Unknown tag type");
        cStringTokenizer filterTokenizer(par("vlanIdFilter"));
        while (filterTokenizer.hasMoreTokens())
            vlanIdFilter.push_back(atoi(filterTokenizer.nextToken()));
        cStringTokenizer mapTokenizer(par("vlanIdMap"));
        while (mapTokenizer.hasMoreTokens()) {
            auto fromVlanId = atoi(mapTokenizer.nextToken());
            auto toVlanId = atoi(mapTokenizer.nextToken());
            vlanIdMap[fromVlanId] = toVlanId;
        }
        WATCH_VECTOR(vlanIdFilter);
        WATCH_MAP(vlanIdMap);
    }
}

void Ieee8021qTagger::processPacket(Packet *packet)
{
    const auto& typeOrLengthHeader = packet->peekAtFront<Ieee8023TypeOrLength>();
    auto oldVlanId = typeOrLengthHeader->getTypeOrLength() == etherType ? packet->peekAtFront<Ieee8021QHeader>()->getVid() : -1;
    auto vlanReq = packet->findTag<VlanReq>();
    auto newVlanId = vlanReq != nullptr ? vlanReq->getVlanId() : oldVlanId;
    auto it = vlanIdMap.find(newVlanId);
    if (it != vlanIdMap.end())
        newVlanId = it->second;
    if (newVlanId != oldVlanId) {
        EV_WARN << "Changing VLAN ID: new = " << newVlanId << ", old = " << oldVlanId << ".\n";
        if (oldVlanId == -1 && newVlanId != -1) {
            auto vlanHeader = makeShared<Ieee8021QHeader>();
            vlanHeader->setVid(newVlanId);
            packet->insertAtFront(vlanHeader);
        }
        else if (oldVlanId != -1 && newVlanId == -1)
            packet->removeAtFront<Ieee8021QHeader>();
        else {
            auto vlanHeader = packet->removeAtFront<Ieee8021QHeader>();
            vlanHeader->setVid(newVlanId);
            packet->insertAtFront(vlanHeader);
        }
    }
    packet->addTagIfAbsent<VlanInd>()->setVlanId(newVlanId);
}

void Ieee8021qTagger::dropPacket(Packet *packet)
{
    EV_WARN << "Received packet " << packet->getName() << " is not accepted, dropping packet.\n";
    PacketFilterBase::dropPacket(packet, OTHER_PACKET_DROP);
}

bool Ieee8021qTagger::matchesPacket(const Packet *packet) const
{
    const auto& typeOrLengthHeader = packet->peekAtFront<Ieee8023TypeOrLength>();
    auto oldVlanId = typeOrLengthHeader->getTypeOrLength() == etherType ? packet->peekAtFront<Ieee8021QHeader>()->getVid() : -1;
    auto vlanReq = packet->findTag<VlanReq>();
    auto newVlanId = vlanReq != nullptr ? vlanReq->getVlanId() : oldVlanId;
    return vlanIdFilter.empty() || std::find(vlanIdFilter.begin(), vlanIdFilter.end(), newVlanId) != vlanIdFilter.end();
}

} // namespace inet


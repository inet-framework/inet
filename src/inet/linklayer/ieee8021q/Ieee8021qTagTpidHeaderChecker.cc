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

#include "inet/linklayer/ieee8021q/Ieee8021qTagTpidHeaderChecker.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/linklayer/ieee8021q/Ieee8021qTagHeader_m.h"

namespace inet {

Define_Module(Ieee8021qTagTpidHeaderChecker);

void Ieee8021qTagTpidHeaderChecker::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *vlanTagType = par("vlanTagType");
        if (!strcmp("s", vlanTagType))
            tpid = 0x88A8;
        else if (!strcmp("c", vlanTagType))
            tpid = 0x8100;
        else
            throw cRuntimeError("Unknown tag type");
        cStringTokenizer filterTokenizer(par("vlanIdFilter"));
        while (filterTokenizer.hasMoreTokens())
            vlanIdFilter.push_back(atoi(filterTokenizer.nextToken()));
        WATCH_VECTOR(vlanIdFilter);
    }
}

void Ieee8021qTagTpidHeaderChecker::processPacket(Packet *packet)
{
    const auto& header = packet->popAtFront<Ieee8021qTagTpidHeader>();
    packet->addTagIfAbsent<UserPriorityInd>()->setUserPriority(header->getPcp());
    packet->addTagIfAbsent<VlanInd>()->setVlanId(header->getVid());
    auto packetProtocolTag = packet->addTagIfAbsent<PacketProtocolTag>();
    packetProtocolTag->setFrontOffset(packetProtocolTag->getFrontOffset() - header->getChunkLength());
}

void Ieee8021qTagTpidHeaderChecker::dropPacket(Packet *packet)
{
    EV_WARN << "Received packet " << packet->getName() << " is not accepted, dropping packet.\n";
    PacketFilterBase::dropPacket(packet, OTHER_PACKET_DROP);
}

bool Ieee8021qTagTpidHeaderChecker::matchesPacket(const Packet *packet) const
{
    const auto& header = packet->peekAtFront<Ieee8021qTagTpidHeader>();
    if (header->getTpid() != tpid)
        return false;
    else {
        auto vlanId = header->getVid();
        return vlanIdFilter.empty() || std::find(vlanIdFilter.begin(), vlanIdFilter.end(), vlanId) != vlanIdFilter.end();
    }
}

} // namespace inet


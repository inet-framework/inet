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

#include "inet/linklayer/ieee8021q/Ieee8021qTagEpdHeaderChecker.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/EtherType_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/linklayer/ieee8021q/Ieee8021qTagHeader_m.h"

namespace inet {

Define_Module(Ieee8021qTagEpdHeaderChecker);

void Ieee8021qTagEpdHeaderChecker::initialize(int stage)
{
    PacketFilterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *vlanTagType = par("vlanTagType");
        if (!strcmp("s", vlanTagType))
            qtagProtocol = &Protocol::ieee8021qSTag;
        else if (!strcmp("c", vlanTagType))
            qtagProtocol = &Protocol::ieee8021qCTag;
        else
            throw cRuntimeError("Unknown tag type");
        cStringTokenizer filterTokenizer(par("vlanIdFilter"));
        while (filterTokenizer.hasMoreTokens())
            vlanIdFilter.push_back(atoi(filterTokenizer.nextToken()));
        WATCH_VECTOR(vlanIdFilter);
    }
    else if (stage == INITSTAGE_LINK_LAYER)
        registerProtocol(*qtagProtocol, nullptr, inputGate);
}

void Ieee8021qTagEpdHeaderChecker::processPacket(Packet *packet)
{
    const auto& header = packet->popAtFront<Ieee8021qTagEpdHeader>();
    packet->addTagIfAbsent<UserPriorityInd>()->setUserPriority(header->getPcp());
    packet->addTagIfAbsent<VlanInd>()->setVlanId(header->getVid());
    auto typeOrLength = header->getTypeOrLength();
    const Protocol *protocol;
    if (isIeee8023Length(typeOrLength))
        protocol = &Protocol::ieee8022llc;
    else
        protocol = ProtocolGroup::ethertype.getProtocol(typeOrLength);
    auto packetProtocolTag = packet->addTagIfAbsent<PacketProtocolTag>();
    packetProtocolTag->setFrontOffset(b(0));
    packetProtocolTag->setBackOffset(b(0));
    packetProtocolTag->setProtocol(protocol);
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(protocol);
}

void Ieee8021qTagEpdHeaderChecker::dropPacket(Packet *packet)
{
    EV_WARN << "Received packet " << packet->getName() << " is not accepted, dropping packet.\n";
    PacketFilterBase::dropPacket(packet, OTHER_PACKET_DROP);
    // TODO: or PacketFilterBase::dropPacket(packet, NO_PROTOCOL_FOUND);
}

bool Ieee8021qTagEpdHeaderChecker::matchesPacket(const Packet *packet) const
{
    const auto& header = packet->peekAtFront<Ieee8021qTagEpdHeader>();
    auto typeOrLength = header->getTypeOrLength();
    if (!isIeee8023Length(typeOrLength) && ProtocolGroup::ethertype.findProtocol(typeOrLength) == nullptr)
        return false;
    else {
        auto vlanId = header->getVid();
        return vlanIdFilter.empty() || std::find(vlanIdFilter.begin(), vlanIdFilter.end(), vlanId) != vlanIdFilter.end();
    }
}

} // namespace inet


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

#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/vlan/VlanTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/physicallayer/common/packetlevel/SignalTag_m.h"
#include "inet/queueing/marker/PacketTagger.h"

namespace inet {
namespace queueing {

Define_Module(PacketTagger);

PacketTagger::~PacketTagger()
{
    delete packetFilterFunction;
}

void PacketTagger::initialize(int stage)
{
    PacketMarkerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        hopLimit = par("hopLimit");
        vlanId = par("vlanId");
        userPriority = par("userPriority");
        transmissionPower = W(par("transmissionPower"));
        const char *filterClass = par("filterClass");
        packetFilterFunction = check_and_cast<IPacketFilterFunction *>(createOne(filterClass));
    }
    else if (stage == INITSTAGE_LOCAL) {
        const char *interfaceName = par("interfaceName");
        if (strlen(interfaceName) != 0) {
            auto interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
            auto interface = interfaceTable->getInterfaceByName(interfaceName);
            interfaceId = interface->getId();
        }
    }
}

void PacketTagger::markPacket(Packet *packet)
{
    if (packetFilterFunction->matchesPacket(packet)) {
        if (interfaceId != -1) {
            EV_DEBUG << "Attaching InterfaceReq to " << packet->getName() << " with interfaceId = " << interfaceId << std::endl;
            packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(interfaceId);
        }
        if (hopLimit != -1) {
            EV_DEBUG << "Attaching HopLimitReq to " << packet->getName() << " with hopLimit = " << hopLimit << std::endl;
            packet->addTagIfAbsent<HopLimitReq>()->setHopLimit(hopLimit);
        }
        if (vlanId != -1) {
            EV_DEBUG << "Attaching VlanReq to " << packet->getName() << " with vlanId = " << vlanId << std::endl;
            packet->addTagIfAbsent<VlanReq>()->setVlanId(vlanId);
        }
        if (userPriority != -1) {
            EV_DEBUG << "Attaching UserPriorityReq to " << packet->getName() << " with userPriority = " << userPriority << std::endl;
            packet->addTagIfAbsent<UserPriorityReq>()->setUserPriority(userPriority);
        }
        if (!std::isnan(transmissionPower.get())) {
            EV_DEBUG << "Attaching SignalPowerReq to " << packet->getName() << " with power = " << transmissionPower << std::endl;
            packet->addTagIfAbsent<SignalPowerReq>()->setPower(transmissionPower);
        }
    }
}

} // namespace queueing
} // namespace inet


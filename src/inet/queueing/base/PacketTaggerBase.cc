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

#include "inet/queueing/base/PacketTaggerBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/networklayer/common/DscpTag_m.h"
#include "inet/networklayer/common/EcnTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/common/TosTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"

namespace inet {
namespace queueing {

void PacketTaggerBase::initialize(int stage)
{
    PacketMarkerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        dscp = par("dscp");
        ecn = par("ecn");
        tos = par("tos");
        if (tos != -1 && dscp != -1)
            throw cRuntimeError("parameter error: 'tos' and 'dscp' parameters specified together");
        hopLimit = par("hopLimit");
        vlanId = par("vlanId");
        userPriority = par("userPriority");
        transmissionPower = W(par("transmissionPower"));
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        const char *interfaceName = par("interfaceName");
        if (strlen(interfaceName) != 0) {
            auto interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
            auto interface = CHK(interfaceTable->findInterfaceByName(interfaceName));
            interfaceId = interface->getInterfaceId();
        }
    }
}

void PacketTaggerBase::markPacket(Packet *packet)
{
    if (dscp != -1) {
        EV_DEBUG << "Attaching DscpReq" << EV_FIELD(packet) << EV_FIELD(dscp) << EV_ENDL;
        packet->addTagIfAbsent<DscpReq>()->setDifferentiatedServicesCodePoint(dscp);
    }
    if (ecn != -1) {
        EV_DEBUG << "Attaching EcnReq" << EV_FIELD(packet) << EV_FIELD(ecn) << EV_ENDL;
        packet->addTagIfAbsent<EcnReq>()->setExplicitCongestionNotification(ecn);
    }
    if (tos != -1) {
        EV_DEBUG << "Attaching TosReq" << EV_FIELD(packet) << EV_FIELD(tos) << EV_ENDL;
        packet->addTagIfAbsent<TosReq>()->setTos(tos);
    }
    if (interfaceId != -1) {
        EV_DEBUG << "Attaching InterfaceReq" << EV_FIELD(packet) << EV_FIELD(interfaceId) << EV_ENDL;
        packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(interfaceId);
    }
    if (hopLimit != -1) {
        EV_DEBUG << "Attaching HopLimitReq" << EV_FIELD(hopLimit) << EV_FIELD(packet) << EV_ENDL;
        packet->addTagIfAbsent<HopLimitReq>()->setHopLimit(hopLimit);
    }
    if (vlanId != -1) {
        EV_DEBUG << "Attaching VlanReq" << EV_FIELD(packet) << EV_FIELD(vlanId) << EV_ENDL;
        packet->addTagIfAbsent<VlanReq>()->setVlanId(vlanId);
    }
    if (userPriority != -1) {
        EV_DEBUG << "Attaching UserPriorityReq" << EV_FIELD(packet) << EV_FIELD(userPriority) << EV_ENDL;
        packet->addTagIfAbsent<UserPriorityReq>()->setUserPriority(userPriority);
    }
    if (!std::isnan(transmissionPower.get())) {
        EV_DEBUG << "Attaching SignalPowerReq" << EV_FIELD(packet) << EV_FIELD(transmissionPower) << EV_ENDL;
        packet->addTagIfAbsent<SignalPowerReq>()->setPower(transmissionPower);
    }
}

} // namespace queueing
} // namespace inet


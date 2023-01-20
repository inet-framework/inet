//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/base/PacketTaggerBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/ProtocolUtils.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/PcpTag_m.h"
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
        pcp = par("pcp");
        userPriority = par("userPriority");
        transmissionPower = W(par("transmissionPower"));
        auto encapsulationProtocolsAsArray = check_and_cast<cValueArray *>(par("encapsulationProtocols").objectValue());
        for (int i = 0; i < encapsulationProtocolsAsArray->size(); i++) {
            auto protocol = Protocol::getProtocol(encapsulationProtocolsAsArray->get(i).stringValue());
            encapsulationProtocols.push_back(protocol);
        }
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
    if (pcp != -1) {
        EV_DEBUG << "Attaching PcpReq" << EV_FIELD(packet) << EV_FIELD(pcp) << EV_ENDL;
        packet->addTagIfAbsent<PcpReq>()->setPcp(pcp);
    }
    if (userPriority != -1) {
        EV_DEBUG << "Attaching UserPriorityReq" << EV_FIELD(packet) << EV_FIELD(userPriority) << EV_ENDL;
        packet->addTagIfAbsent<UserPriorityReq>()->setUserPriority(userPriority);
    }
    if (!std::isnan(transmissionPower.get())) {
        EV_DEBUG << "Attaching SignalPowerReq" << EV_FIELD(packet) << EV_FIELD(transmissionPower) << EV_ENDL;
        packet->addTagIfAbsent<SignalPowerReq>()->setPower(transmissionPower);
    }
    for (int i = 0; i < encapsulationProtocols.size(); i++) {
        auto protocol = encapsulationProtocols[encapsulationProtocols.size() - i - 1];
        ensureEncapsulationProtocolReq(packet, protocol);
    }
}

} // namespace queueing
} // namespace inet


//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// VRRPv2 (RFC 3768) ported from the ANSAINET project.
// Original authors: Petr Vitek, Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/vrrp/Vrrp.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/routing/vrrp/VrrpVirtualRouter.h"

namespace inet {
namespace vrrp {

Define_Module(Vrrp);

static bool parseXmlBool(const char *value)
{
    if (value == nullptr)
        return false;
    return !strcmp(value, "yes") || !strcmp(value, "true") || !strcmp(value, "on")
           || !strcmp(value, "enabled") || !strcmp(value, "1");
}

void Vrrp::initialize(int stage)
{
    RoutingProtocolBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        host = getContainingNode(this);
        ift.reference(this, "interfaceTableModule", true);
        rt.reference(this, "routingTableModule", true);
        arp.reference(this, "arpModule", true);
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        registerProtocol(Protocol::vrrp, gate("ipOut"), gate("ipIn"));
        loadConfig();
    }
}

void Vrrp::loadConfig()
{
    cXMLElement *config = par("configData");
    if (config == nullptr)
        return;

    for (cXMLElement *interfaceElem : config->getChildrenByTagName("Interface")) {
        const char *ifName = interfaceElem->getAttribute("name");
        if (!ifName)
            throw cRuntimeError("VRRP <Interface> element without a 'name' attribute at %s", interfaceElem->getSourceLocation());
        NetworkInterface *ie = ift->findInterfaceByName(ifName);
        if (!ie)
            throw cRuntimeError("VRRP configuration refers to unknown interface '%s'", ifName);

        for (cXMLElement *groupElem : interfaceElem->getChildrenByTagName("Group")) {
            const char *idAttr = groupElem->getAttribute("id");
            int vrid = idAttr ? atoi(idAttr) : -1;
            if (vrid < 1 || vrid > 255)
                throw cRuntimeError("VRRP <Group> on interface '%s' has invalid id '%s' (must be 1..255)", ifName, idAttr ? idAttr : "");
            createVirtualRouter(ie->getInterfaceId(), vrid, groupElem);
        }
    }
}

VrrpVirtualRouter *Vrrp::createVirtualRouter(int interfaceId, int vrid, cXMLElement *group)
{
    // collect the virtual IP addresses: the <IPAddress> primary first, then any <IPSecondary>
    std::string virtualIps;
    if (cXMLElement *e = group->getFirstChildWithTag("IPAddress"))
        virtualIps = e->getNodeValue();
    else
        throw cRuntimeError("VRRP group %d has no <IPAddress> element", vrid);
    for (cXMLElement *e : group->getChildrenByTagName("IPSecondary"))
        virtualIps += std::string(" ") + e->getNodeValue();

    cModuleType *moduleType = cModuleType::get("inet.routing.vrrp.VrrpVirtualRouter");
    std::string name = std::string("vr_") + ift->getInterfaceById(interfaceId)->getInterfaceName() + "_" + std::to_string(vrid);
    cModule *module = moduleType->create(name.c_str(), this);

    // identity and group-specific configuration (from XML)
    module->par("vrid").setIntValue(vrid);
    module->par("interfaceId").setIntValue(interfaceId);
    module->par("virtualIps").setStringValue(virtualIps.c_str());
    if (cXMLElement *e = group->getFirstChildWithTag("Priority"))
        module->par("priority").setIntValue(atoi(e->getNodeValue()));
    if (cXMLElement *e = group->getFirstChildWithTag("Description"))
        module->par("description").setStringValue(e->getNodeValue());
    if (cXMLElement *e = group->getFirstChildWithTag("Preempt")) {
        module->par("preempt").setBoolValue(parseXmlBool(e->getNodeValue()));
        if (const char *delay = e->getAttribute("delay"))
            module->par("preemptDelay").setDoubleValue(atof(delay));
    }
    if (cXMLElement *e = group->getFirstChildWithTag("TimerAdvertise"))
        module->par("advertiseInterval").setDoubleValue(atof(e->getNodeValue()));
    if (cXMLElement *e = group->getFirstChildWithTag("TimerLearn"))
        module->par("learn").setBoolValue(parseXmlBool(e->getNodeValue()));

    // defaults propagated from this module's parameters
    module->par("version").setIntValue(par("version").intValue());
    module->par("priorityOwner").setIntValue(par("priorityOwner").intValue());
    module->par("timeToLive").setIntValue(par("timeToLive").intValue());
    module->par("multicastAddress").setStringValue(par("multicastAddress").stringValue());
    module->par("virtualMacOui").setStringValue(par("virtualMacOui").stringValue());
    module->par("arpType").setIntValue(par("arpType").intValue());
    module->par("arpDelay").setDoubleValue(par("arpDelay").doubleValue());

    module->finalizeParameters();
    module->buildInside();
    module->callInitialize();

    VrrpVirtualRouter *vr = check_and_cast<VrrpVirtualRouter *>(module);
    virtualRouters.push_back(vr);
    return vr;
}

VrrpVirtualRouter *Vrrp::findVirtualRouter(int interfaceId, int vrid)
{
    for (auto vr : virtualRouters)
        if (vr->getInterfaceId() == interfaceId && vr->getVrid() == vrid)
            return vr;
    return nullptr;
}

void Vrrp::handleMessageWhenUp(cMessage *msg)
{
    Packet *packet = check_and_cast<Packet *>(msg);
    auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    if (protocol == &Protocol::vrrp) {
        const auto& adv = packet->peekAtFront<VrrpAdvertisement>();
        int interfaceId = packet->getTag<InterfaceInd>()->getInterfaceId();
        Ipv4Address srcAddr = packet->getTag<L3AddressInd>()->getSrcAddress().toIpv4();
        int ttl = packet->findTag<HopLimitInd>() ? packet->getTag<HopLimitInd>()->getHopLimit() : -1;
        if (VrrpVirtualRouter *vr = findVirtualRouter(interfaceId, adv->getVrid()))
            vr->processReceivedAdvertisement(adv, srcAddr, ttl);
        else
            EV_WARN << "No virtual router for VRID " << (int)adv->getVrid() << " on interface " << interfaceId << ", discarding\n";
    }
    delete packet;
}

void Vrrp::sendAdvertisement(VrrpVirtualRouter *vr, const Ptr<VrrpAdvertisement>& adv)
{
    Enter_Method("sendAdvertisement");

    int interfaceId = vr->getInterfaceId();
    NetworkInterface *ie = ift->getInterfaceById(interfaceId);

    auto packet = new Packet("VrrpAdvertisement");
    packet->insertAtBack(adv);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::vrrp);
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
    packet->addTagIfAbsent<InterfaceReq>()->setInterfaceId(interfaceId);
    auto addresses = packet->addTagIfAbsent<L3AddressReq>();
    addresses->setSrcAddress(ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress());
    addresses->setDestAddress(vr->getMulticastAddress());
    packet->addTagIfAbsent<HopLimitReq>()->setHopLimit(vr->getTimeToLive());
    send(packet, "ipOut");
}

void Vrrp::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));
    // TODO react to interface up/down (the state machine currently self-detects at timer events)
}

void Vrrp::handleStartOperation(LifecycleOperation *operation)
{
}

void Vrrp::handleStopOperation(LifecycleOperation *operation)
{
}

void Vrrp::handleCrashOperation(LifecycleOperation *operation)
{
}

} // namespace vrrp
} // namespace inet

//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/pmipv6/Pmipv6.h"

#include <cstdlib>
#include <cstring>

#include "inet/common/ModuleAccess.h"
#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#include "inet/networklayer/ipv6/Ipv6Route.h"
#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"
#include "inet/networklayer/mipv6/MobilityHeader_m.h"
#include "inet/networklayer/mipv6/MobilityHeaderSerializer.h"

namespace inet {

Define_Module(Pmipv6);

Pmipv6::~Pmipv6()
{
}

void Pmipv6::initialize(int stage)
{
    OperationalBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        isLma = par("isLocalMobilityAnchor");
        isMag = par("isMobileAccessGateway");
        if (isLma == isMag)
            throw cRuntimeError("Pmipv6: exactly one of isLocalMobilityAnchor / isMobileAccessGateway must be set");

        bindingLifetime = par("bindingLifetime");
        advValidLifetime = par("homeNetworkPrefixAdvValidLifetime");
        advPreferredLifetime = par("homeNetworkPrefixAdvPreferredLifetime");
        const char *lma = par("localMobilityAnchorAddress");
        if (lma[0])
            localMobilityAnchorAddress = Ipv6Address(lma);

        cModule *host = getContainingNode(this);
        rt6 = L3AddressResolver().getIpv6RoutingTableOf(host);

        if (isMag) {
            parseMobileNodeProfiles();
            // detect mobile nodes attaching to / leaving this access gateway's links
            host->subscribe(l2ApAssociatedSignal, this);
            host->subscribe(l2ApDisassociatedSignal, this);
        }

        WATCH(magTunnelId);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        ift.reference(this, "interfaceTableModule", true);
        ipv6nd.reference(this, "ipv6NeighbourDiscoveryModule", true);
        // (both LMA and MAG are routers; forwarding is enabled by the node type)
    }
}

void Pmipv6::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage())
        throw cRuntimeError("Pmipv6: unexpected self-message");
    else if (auto packet = dynamic_cast<Packet *>(msg)) {
        auto protocolTag = packet->findTag<PacketProtocolTag>();
        if (protocolTag && protocolTag->getProtocol() == &Protocol::mobileipv6)
            processMobilityMessage(packet);
        else {
            EV_WARN << "Discarding unexpected packet " << packet->getName() << endl;
            delete packet;
        }
    }
    else if (auto indication = dynamic_cast<Indication *>(msg)) {
        EV_WARN << "Received an error indication (" << indication->getName() << "); ignoring it" << endl;
        delete indication;
    }
    else
        throw cRuntimeError("Pmipv6: unknown message '%s'", msg->getName());
}

void Pmipv6::processMobilityMessage(Packet *packet)
{
    auto mh = packet->peekAtFront<MobilityHeader>();
    switch (mh->getMobilityHeaderType()) {
        case BINDING_UPDATE: {
            auto bu = packet->peekAtFront<BindingUpdate>();
            if (isLma && bu->getProxyRegistrationFlag())
                processProxyBindingUpdate(packet, bu.get());
            else
                EV_WARN << "Ignoring Binding Update (not a Proxy Binding Update for an LMA)" << endl;
            break;
        }
        case BINDING_ACKNOWLEDGEMENT: {
            auto ba = packet->peekAtFront<BindingAcknowledgement>();
            if (isMag && ba->getProxyRegistrationFlag())
                processProxyBindingAcknowledgement(packet, ba.get());
            else
                EV_WARN << "Ignoring Binding Acknowledgement (not a Proxy Binding Acknowledgement for a MAG)" << endl;
            break;
        }
        default:
            EV_WARN << "Ignoring unsupported Mobility Header type " << mh->getMobilityHeaderType() << endl;
            break;
    }
    delete packet;
}

//
// Common helpers
//

void Pmipv6::sendMobilityMessage(Packet *packet, const Ipv6Address& destAddress, const Ipv6Address& srcAddress)
{
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv6);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::mobileipv6);
    packet->addTagIfAbsent<L3AddressReq>()->setSrcAddress(srcAddress);
    packet->addTagIfAbsent<L3AddressReq>()->setDestAddress(destAddress);
    packet->addTagIfAbsent<HopLimitReq>()->setHopLimit(64);
    send(packet, "toIPv6");
}

Ipv6Address Pmipv6::getEgressAddressFor(const Ipv6Address& destination)
{
    if (const Ipv6Route *route = rt6->doLongestPrefixMatch(destination)) {
        if (NetworkInterface *ie = route->getInterface()) {
            Ipv6Address addr = ie->getProtocolData<Ipv6InterfaceData>()->getPreferredAddress();
            if (!addr.isUnspecified())
                return addr;
        }
    }
    // fall back to the first global address on any interface
    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        NetworkInterface *ie = ift->getInterface(i);
        if (auto ipv6Data = ie->findProtocolData<Ipv6InterfaceData>()) {
            Ipv6Address addr = ipv6Data->getPreferredAddress();
            if (!addr.isUnspecified())
                return addr;
        }
    }
    return Ipv6Address::UNSPECIFIED_ADDRESS;
}

int Pmipv6::getOrCreateTunnel(const Ipv6Address& localEndpoint, const Ipv6Address& remoteEndpoint, std::map<Ipv6Address, int>& tunnelMap)
{
    auto it = tunnelMap.find(remoteEndpoint);
    if (it != tunnelMap.end())
        return it->second;
    NetworkInterface *tunnel = rt6->createTunnelNetworkInterface(localEndpoint, remoteEndpoint);
    int id = tunnel->getInterfaceId();
    tunnelMap[remoteEndpoint] = id;
    EV_INFO << "Created PMIPv6 tunnel " << localEndpoint << " -> " << remoteEndpoint
            << " (interface id " << id << ")" << endl;
    return id;
}

//
// Local Mobility Anchor
//

void Pmipv6::processProxyBindingUpdate(Packet *packet, const BindingUpdate *pbu)
{
    auto addresses = packet->getTag<L3AddressInd>();
    Ipv6Address magAddress = addresses->getSrcAddress().toIpv6(); // Proxy care-of address
    Ipv6Address lmaAddress = addresses->getDestAddress().toIpv6();
    std::string mnId = pbu->getMobileNodeIdentifier();
    Ipv6Address hnp = pbu->getHomeNetworkPrefix();
    int hnpLen = pbu->getHomeNetworkPrefixLength();
    unsigned int seq = pbu->getSequence();
    unsigned int lifetime = pbu->getLifetime();

    EV_INFO << "LMA received Proxy Binding Update from MAG " << magAddress << " for MN '" << mnId
            << "' prefix " << hnp << "/" << hnpLen << " lifetime " << lifetime << "s" << endl;

    BaStatus status = BINDING_UPDATE_ACCEPTED;
    auto it = bindingCache.find(mnId);

    if (lifetime == 0) {
        // Deregistration. Only tear down if the request comes from the MAG that
        // currently serves the mobile node; otherwise this is a stale request
        // from a previous MAG after the node has already moved (handover race).
        if (it != bindingCache.end() && it->second.servingMagAddress == magAddress) {
            if (it->second.downlinkRoute)
                rt6->deleteRoute(it->second.downlinkRoute);
            bindingCache.erase(it);
            EV_INFO << "LMA removed binding for MN '" << mnId << "'" << endl;
        }
        else
            EV_INFO << "LMA ignoring stale deregistration for MN '" << mnId << "'" << endl;
    }
    else {
        // Registration / re-registration / handover.
        int tunnelId = getOrCreateTunnel(lmaAddress, magAddress, lmaTunnelByMag);
        NetworkInterface *tunnel = ift->getInterfaceById(tunnelId);

        BindingCacheEntry& entry = bindingCache[mnId];
        bool retargeted = entry.downlinkRoute && entry.tunnelInterfaceId != tunnelId;
        if (retargeted) {
            // mobile node moved to a different MAG: re-point its prefix route
            rt6->deleteRoute(entry.downlinkRoute);
            entry.downlinkRoute = nullptr;
            EV_INFO << "LMA handover: re-pointing prefix " << hnp << "/" << hnpLen
                    << " toward MAG " << magAddress << endl;
        }
        if (entry.downlinkRoute == nullptr) {
            auto *route = new Ipv6Route(hnp, hnpLen, IRoute::MANUAL);
            route->setInterface(tunnel);
            route->setNextHop(Ipv6Address::UNSPECIFIED_ADDRESS);
            route->setMetric(1);
            rt6->addRoute(route);
            entry.downlinkRoute = route;
        }
        entry.homeNetworkPrefix = hnp;
        entry.homeNetworkPrefixLength = hnpLen;
        entry.servingMagAddress = magAddress;
        entry.sequenceNumber = seq;
        entry.expiry = simTime() + lifetime;
        entry.tunnelInterfaceId = tunnelId;
    }

    // Send the Proxy Binding Acknowledgement back to the MAG.
    auto reply = new Packet("ProxyBindingAck");
    const auto& pba = makeShared<BindingAcknowledgement>();
    pba->setMobilityHeaderType(BINDING_ACKNOWLEDGEMENT);
    pba->setProxyRegistrationFlag(true);
    pba->setStatus(status);
    pba->setSequenceNumber(seq);
    pba->setLifetime(lifetime);
    pba->setMobileNodeIdentifier(mnId.c_str());
    pba->setHomeNetworkPrefix(hnp);
    pba->setHomeNetworkPrefixLength(hnpLen);
    pba->setChunkLength(MobilityHeaderSerializer::getProxyBindingAcknowledgementLength(mnId.size()));
    reply->insertAtFront(pba);
    sendMobilityMessage(reply, magAddress, lmaAddress);
}

//
// Mobile Access Gateway
//

void Pmipv6::parseMobileNodeProfiles()
{
    cXMLElement *root = par("mobileNodeProfiles");
    if (!root)
        return;
    for (cXMLElement *child : root->getChildrenByTagName("mobileNode")) {
        MobileNodeProfile profile;
        if (const char *ai = child->getAttribute("accessInterface"))
            profile.accessInterfaceName = ai;
        const char *id = child->getAttribute("id");
        const char *hnp = child->getAttribute("homeNetworkPrefix");
        if (!id || !hnp)
            throw cRuntimeError("Pmipv6: <mobileNode> needs at least 'id' and 'homeNetworkPrefix' attributes");
        profile.mnIdentifier = id;
        profile.homeNetworkPrefix = Ipv6Address(hnp);
        if (const char *plen = child->getAttribute("prefixLength"))
            profile.homeNetworkPrefixLength = atoi(plen);
        mobileNodeProfiles.push_back(profile);
    }
}

const Pmipv6::MobileNodeProfile *Pmipv6::findProfileForInterface(NetworkInterface *accessInterface) const
{
    const char *name = accessInterface->getInterfaceName();
    const MobileNodeProfile *wildcard = nullptr;
    for (const auto& profile : mobileNodeProfiles) {
        if (profile.accessInterfaceName == name)
            return &profile;
        if (profile.accessInterfaceName.empty() && wildcard == nullptr)
            wildcard = &profile;
    }
    return wildcard;
}

void Pmipv6::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));
    if (signalID == l2ApAssociatedSignal || signalID == l2ApDisassociatedSignal) {
        // the signal is emitted by an access point management module; the access
        // link is the network interface containing it
        NetworkInterface *accessInterface = getContainingNicModule(check_and_cast<cModule *>(source));
        if (accessInterface == nullptr)
            return;
        if (signalID == l2ApAssociatedSignal)
            handleMobileNodeAttached(accessInterface);
        else
            handleMobileNodeDetached(accessInterface);
    }
}

void Pmipv6::handleMobileNodeAttached(NetworkInterface *accessInterface)
{
    const MobileNodeProfile *profile = findProfileForInterface(accessInterface);
    if (profile == nullptr) {
        EV_DETAIL << "No mobile-node profile for access interface " << accessInterface->getInterfaceName()
                  << "; ignoring attachment" << endl;
        return;
    }
    EV_INFO << "MAG: mobile node '" << profile->mnIdentifier << "' attached on "
            << accessInterface->getInterfaceName() << "; sending Proxy Binding Update" << endl;

    MagBinding& binding = magBindings[profile->mnIdentifier];
    binding.mnIdentifier = profile->mnIdentifier;
    binding.homeNetworkPrefix = profile->homeNetworkPrefix;
    binding.homeNetworkPrefixLength = profile->homeNetworkPrefixLength;
    binding.accessInterfaceId = accessInterface->getInterfaceId();
    binding.sequenceNumber++;
    binding.registered = false;
    sendProxyBindingUpdate(binding, bindingLifetime);
}

void Pmipv6::handleMobileNodeDetached(NetworkInterface *accessInterface)
{
    for (auto& kv : magBindings) {
        MagBinding& binding = kv.second;
        if (binding.accessInterfaceId == accessInterface->getInterfaceId()) {
            EV_INFO << "MAG: mobile node '" << binding.mnIdentifier << "' detached from "
                    << accessInterface->getInterfaceName() << "; deregistering" << endl;
            // stop advertising the prefix and remove the local delivery route
            if (auto ipv6Data = accessInterface->findProtocolDataForUpdate<Ipv6InterfaceData>()) {
                for (int i = 0; i < ipv6Data->getNumAdvPrefixes(); i++) {
                    if (ipv6Data->getAdvPrefix(i).prefix == binding.homeNetworkPrefix) {
                        ipv6Data->removeAdvPrefix(i);
                        break;
                    }
                }
            }
            if (binding.downlinkRoute) {
                rt6->deleteRoute(binding.downlinkRoute);
                binding.downlinkRoute = nullptr;
            }
            binding.sequenceNumber++;
            sendProxyBindingUpdate(binding, 0); // lifetime 0 = deregistration
            magBindings.erase(kv.first);
            return;
        }
    }
}

void Pmipv6::sendProxyBindingUpdate(MagBinding& binding, simtime_t lifetime)
{
    if (localMobilityAnchorAddress.isUnspecified())
        throw cRuntimeError("Pmipv6 MAG: localMobilityAnchorAddress is not configured");
    Ipv6Address magAddress = getEgressAddressFor(localMobilityAnchorAddress);

    auto packet = new Packet(lifetime == 0 ? "ProxyBindingUpdate(dereg)" : "ProxyBindingUpdate");
    const auto& pbu = makeShared<BindingUpdate>();
    pbu->setMobilityHeaderType(BINDING_UPDATE);
    pbu->setProxyRegistrationFlag(true);
    pbu->setAckFlag(true);
    pbu->setHomeRegistrationFlag(true);
    pbu->setLifetime(lifetime.dbl() < 0 ? 0 : (unsigned int)lifetime.dbl());
    pbu->setSequence(binding.sequenceNumber);
    pbu->setMobileNodeIdentifier(binding.mnIdentifier.c_str());
    pbu->setHomeNetworkPrefix(binding.homeNetworkPrefix);
    pbu->setHomeNetworkPrefixLength(binding.homeNetworkPrefixLength);
    pbu->setHandoffIndicator(1);       // attachment over a new interface
    pbu->setAccessTechnologyType(4);   // IEEE 802.11 (RFC 5213 access technology type)
    pbu->setTimestampValue(0);         // ordered by sequence number in this model
    pbu->setChunkLength(MobilityHeaderSerializer::getProxyBindingUpdateLength(binding.mnIdentifier.size()));
    packet->insertAtFront(pbu);
    sendMobilityMessage(packet, localMobilityAnchorAddress, magAddress);
}

void Pmipv6::ensureMagTunnel()
{
    if (magTunnelId != -1)
        return;
    Ipv6Address magAddress = getEgressAddressFor(localMobilityAnchorAddress);
    NetworkInterface *tunnel = rt6->createTunnelNetworkInterface(magAddress, localMobilityAnchorAddress);
    magTunnelId = tunnel->getInterfaceId();
    // route mobile-node uplink traffic (everything not on-link) into the tunnel to the LMA
    magUplinkRoute = new Ipv6Route(Ipv6Address::UNSPECIFIED_ADDRESS, 0, IRoute::MANUAL);
    magUplinkRoute->setInterface(tunnel);
    magUplinkRoute->setNextHop(Ipv6Address::UNSPECIFIED_ADDRESS);
    magUplinkRoute->setMetric(256);
    rt6->addRoute(magUplinkRoute);
    EV_INFO << "MAG: created tunnel " << magAddress << " -> " << localMobilityAnchorAddress
            << " and default route into it (interface id " << magTunnelId << ")" << endl;
}

void Pmipv6::processProxyBindingAcknowledgement(Packet *packet, const BindingAcknowledgement *pba)
{
    std::string mnId = pba->getMobileNodeIdentifier();
    BaStatus status = pba->getStatus();
    unsigned int lifetime = pba->getLifetime();

    auto it = magBindings.find(mnId);
    if (it == magBindings.end()) {
        EV_INFO << "MAG: Proxy Binding Acknowledgement for unknown MN '" << mnId << "'; ignoring" << endl;
        return;
    }
    MagBinding& binding = it->second;

    if (status != BINDING_UPDATE_ACCEPTED) {
        EV_WARN << "MAG: Proxy Binding Update for MN '" << mnId << "' rejected (status " << status << ")" << endl;
        return;
    }
    if (lifetime == 0) {
        EV_INFO << "MAG: deregistration acknowledged for MN '" << mnId << "'" << endl;
        return;
    }

    EV_INFO << "MAG: binding accepted for MN '" << mnId << "' prefix " << binding.homeNetworkPrefix
            << "/" << binding.homeNetworkPrefixLength << endl;

    ensureMagTunnel();

    NetworkInterface *accessInterface = ift->getInterfaceById(binding.accessInterfaceId);
    if (accessInterface == nullptr) {
        EV_WARN << "MAG: access interface for MN '" << mnId << "' no longer exists" << endl;
        return;
    }

    // route decapsulated downlink traffic for the mobile node out the access link
    if (binding.downlinkRoute == nullptr) {
        auto *route = new Ipv6Route(binding.homeNetworkPrefix, binding.homeNetworkPrefixLength, IRoute::MANUAL);
        route->setInterface(accessInterface);
        route->setNextHop(Ipv6Address::UNSPECIFIED_ADDRESS);
        route->setMetric(1);
        rt6->addRoute(route);
        binding.downlinkRoute = route;
    }

    // advertise the mobile node's home network prefix on the access link, so the
    // (unmodified) mobile node keeps its address via stateless autoconfiguration
    auto ipv6Data = accessInterface->getProtocolDataForUpdate<Ipv6InterfaceData>();
    bool alreadyAdvertised = false;
    for (int i = 0; i < ipv6Data->getNumAdvPrefixes(); i++) {
        if (ipv6Data->getAdvPrefix(i).prefix == binding.homeNetworkPrefix) {
            alreadyAdvertised = true;
            break;
        }
    }
    if (!alreadyAdvertised) {
        Ipv6InterfaceData::AdvPrefix advPrefix;
        advPrefix.prefix = binding.homeNetworkPrefix;
        advPrefix.prefixLength = binding.homeNetworkPrefixLength;
        advPrefix.advOnLinkFlag = true;
        advPrefix.advAutonomousFlag = true;
        advPrefix.advValidLifetime = advValidLifetime;
        advPrefix.advPreferredLifetime = advPreferredLifetime;
        ipv6Data->addAdvPrefix(advPrefix);
    }
    // push the prefix to the mobile node immediately rather than waiting for the next periodic RA
    ipv6nd->sendUnsolicitedRa(accessInterface);

    binding.registered = true;
}

//
// Lifecycle
//

void Pmipv6::handleStopOperation(LifecycleOperation *operation)
{
}

void Pmipv6::handleCrashOperation(LifecycleOperation *operation)
{
}

} // namespace inet

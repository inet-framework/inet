//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/pmipv6/Pmipv6.h"

#include <cstdlib>
#include <cstring>

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtAp.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#include "inet/networklayer/ipv6/Ipv6Route.h"
#include "inet/networklayer/ipv6/Ipv6.h"
#include "inet/networklayer/ipv6/Ipv6Header_m.h"
#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"
#include "inet/networklayer/mipv6/MobilityHeader_m.h"
#include "inet/networklayer/mipv6/MobilityHeaderSerializer.h"

namespace inet {

Define_Module(Pmipv6);

// RFC 5213 Section 8.5: Access Technology Type values. Only the one this model
// can observe is named here.
static constexpr uint8_t ACCESS_TECHNOLOGY_IEEE_802_11 = 4;

// RFC 5213 Section 8.4: Handoff Indicator values.
static constexpr uint8_t HANDOFF_STATE_UNKNOWN = 4;   // the gateway cannot tell
static constexpr uint8_t HANDOFF_REREGISTRATION = 5;  // nothing changed, the lifetime is being extended

// RFC 5213 Section 8.8: the Timestamp option is 64 bits wide, of which the first 48
// hold the integer number of seconds and the remaining 16 hold a fraction of a second
// in units of 1/65536. That is not the NTP 32.32 layout, which it is easy to mistake
// it for. The clock a gateway reads here is simulation time.
// RFC 6275 Section 9.5.1 compares binding update sequence numbers modulo 2^16, so
// that the counter can wrap without a later update looking older than an earlier one.
static bool isSequenceNumberNewer(unsigned int candidate, unsigned int accepted)
{
    unsigned int difference = (candidate - accepted) & 0xFFFF;
    return difference != 0 && difference < 0x8000;
}

static uint64_t timestampOf(simtime_t t)
{
    double seconds = t.dbl();
    uint64_t wholeSeconds = (uint64_t)seconds;
    uint64_t fraction = (uint64_t)((seconds - (double)wholeSeconds) * 65536.0);
    return (wholeSeconds << 16) | (fraction & 0xFFFF);
}

simsignal_t Pmipv6::proxyBindingUpdateSentSignal = registerSignal("proxyBindingUpdateSent");
simsignal_t Pmipv6::proxyBindingAcknowledgementReceivedSignal = registerSignal("proxyBindingAcknowledgementReceived");
simsignal_t Pmipv6::proxyBindingUpdateReceivedSignal = registerSignal("proxyBindingUpdateReceived");
simsignal_t Pmipv6::homeNetworkPrefixReanchoredSignal = registerSignal("homeNetworkPrefixReanchored");
simsignal_t Pmipv6::mobileNodeDetachedSignal = registerSignal("mobileNodeDetached");
simsignal_t Pmipv6::bindingCacheSizeSignal = registerSignal("bindingCacheSize");

bool Pmipv6::MobilitySessionKey::operator<(const MobilitySessionKey& other) const
{
    if (mnIdentifier != other.mnIdentifier)
        return mnIdentifier < other.mnIdentifier;
    if (accessTechnologyType != other.accessTechnologyType)
        return accessTechnologyType < other.accessTechnologyType;
    return mnLinkLayerIdentifier.compareTo(other.mnLinkLayerIdentifier) < 0;
}

std::ostream& operator<<(std::ostream& os, const Pmipv6::MobilitySessionKey& key)
{
    os << "'" << key.mnIdentifier << "'";
    if (!key.mnLinkLayerIdentifier.isUnspecified())
        os << " at " << key.mnLinkLayerIdentifier;
    return os << ", access technology " << (int)key.accessTechnologyType;
}

std::ostream& operator<<(std::ostream& os, const Pmipv6::BindingCacheEntry& entry)
{
    return os << "prefix " << entry.homeNetworkPrefix << "/" << entry.homeNetworkPrefixLength
              << ", serving gateway " << entry.servingMagAddress
              << ", tunnel interface id " << entry.tunnelInterfaceId
              << ", sequence " << entry.sequenceNumber
              << ", expires at " << entry.expiry;
}

std::ostream& operator<<(std::ostream& os, const Pmipv6::MobileNodeProfile& profile)
{
    return os << "'" << profile.mnIdentifier << "'"
              << ", link-layer address "
              << (profile.linkLayerAddress.isUnspecified() ? std::string("(any)") : profile.linkLayerAddress.str())
              << ", prefix " << profile.homeNetworkPrefix << "/" << profile.homeNetworkPrefixLength
              << ", access interface "
              << (profile.accessInterfaceName.empty() ? std::string("(any)") : profile.accessInterfaceName);
}

std::ostream& operator<<(std::ostream& os, const Pmipv6::MagBinding& binding)
{
    if (!binding.mnLinkLayerIdentifier.isUnspecified())
        os << "at " << binding.mnLinkLayerIdentifier << ", ";
    return os << "prefix " << binding.homeNetworkPrefix << "/" << binding.homeNetworkPrefixLength
              << ", access interface id " << binding.accessInterfaceId
              << ", sequence " << binding.sequenceNumber
              << (binding.registered ? ", registered" : ", registration pending");
}

Pmipv6::~Pmipv6()
{
    cancelAndDelete(presenceCheckTimer);
    for (auto& element : bindingCache) {
        cancelAndDelete(element.second.deleteTimer);
        cancelAndDelete(element.second.expiryTimer);
    }
    for (auto& element : magBindings) {
        cancelAndDelete(element.second.retransmitTimer);
        cancelAndDelete(element.second.refreshTimer);
    }
}

void Pmipv6::initialize(int stage)
{
    OperationalBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        isLma = par("isLocalMobilityAnchor");
        isMag = par("isMobileAccessGateway");
        if (isLma == isMag)
            throw cRuntimeError("Pmipv6: exactly one of isLocalMobilityAnchor / isMobileAccessGateway must be set");

        timestampBasedOrdering = par("timestampBasedOrdering");
        detectTransmissionFailure = par("detectTransmissionFailure");
        presenceProbeDelay = par("presenceProbeDelay");
        presenceCheckInterval = par("presenceCheckInterval");
        initialBindingAckTimeout = par("initialBindingAckTimeout");
        maxBindingAckTimeout = par("maxBindingAckTimeout");
        minDelayBeforeBindingCacheEntryDelete = par("minDelayBeforeBindingCacheEntryDelete");
        bindingLifetime = par("bindingLifetime");
        maxBindingLifetime = par("maxBindingLifetime");
        bindingRefreshRatio = par("bindingRefreshRatio");
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
            if (detectTransmissionFailure)
                host->subscribe(linkBrokenSignal, this);
            if (presenceProbeDelay > 0) {
                // only then does the gateway need to know when it last heard from a node
                host->subscribe(packetReceivedFromLowerSignal, this);
                presenceCheckTimer = new cMessage("presenceCheck", MAG_PRESENCE_CHECK);
            }
        }

        if (isLma) {
            WATCH_MAP(bindingCache);
            WATCH_MAP(lmaTunnelByMag);
        }
        else {
            WATCH_VECTOR(mobileNodeProfiles);
            WATCH_MAP(magBindings);
            WATCH(magTunnelId);
        }
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        ift.reference(this, "interfaceTableModule", true);
        ipv6nd.reference(this, "ipv6NeighbourDiscoveryModule", true);
        // (both LMA and MAG are routers; forwarding is enabled by the node type)
        // receive Mobility Header messages (Proxy Binding Updates / Acknowledgements)
        registerProtocol(Protocol::mobileipv6, gate("toIPv6"), gate("fromIPv6"));
        if (isMag) {
            // RFC 5213 Section 6.10.5: the gateway decides what to do with a mobile
            // node's packets by their source, which no route can express, so it has to
            // see them before they are routed
            auto *ipv6 = check_and_cast<Ipv6 *>(getModuleByPath("^.ipv6"));
            ipv6->registerHook(0, this);
        }
        if (isLma)
            emit(bindingCacheSizeSignal, (intval_t)bindingCache.size()); // so the recorded series starts at zero
        if (presenceCheckTimer)
            scheduleAfter(presenceCheckInterval, presenceCheckTimer);
    }
}

void Pmipv6::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage())
        handleTimer(msg);
    else if (auto packet = dynamic_cast<Packet *>(msg)) {
        auto protocolTag = packet->findTag<PacketProtocolTag>();
        if (protocolTag && protocolTag->getProtocol() == &Protocol::mobileipv6)
            processMobilityMessage(packet);
        else {
            EV_WARN << "Discarding unexpected packet " << packet->getName() << endl;
            dropPacket(packet, NO_PROTOCOL_FOUND);
        }
    }
    else if (auto indication = dynamic_cast<Indication *>(msg)) {
        EV_WARN << "Received an error indication (" << indication->getName() << "); ignoring it" << endl;
        delete indication;
    }
    else
        throw cRuntimeError("Pmipv6: unknown message '%s'", msg->getName());
}

void Pmipv6::handleTimer(cMessage *timer)
{
    switch (timer->getKind()) {
        case MAG_PRESENCE_CHECK:
            checkMobileNodePresence();
            scheduleAfter(presenceCheckInterval, timer);
            break;
        case LMA_BINDING_DELETE: {
            // RFC 5213 Section 5.3.5 step 2: the wait ended without the mobile node
            // reappearing anywhere, so the mobility session is over
            auto& session = check_and_cast<Pmipv6Timer *>(timer)->session;
            auto it = bindingCache.find(session);
            if (it != bindingCache.end()) {
                EV_INFO << "LMA removed binding for MN '" << session.mnIdentifier
                        << "'; the deletion delay expired without a new registration" << endl;
                it->second.deleteTimer = nullptr;
                deleteBindingCacheEntry(it);
            }
            delete timer;
            break;
        }
        case MAG_BINDING_RETRANSMIT:
            retransmitProxyBindingUpdate(check_and_cast<Pmipv6Timer *>(timer)->session);
            break;
        case MAG_BINDING_REFRESH:
            refreshProxyBinding(check_and_cast<Pmipv6Timer *>(timer)->session);
            break;
        case LMA_BINDING_EXPIRY: {
            // RFC 6275 Section 9.6: a binding cache entry MUST be deleted when its
            // lifetime expires. RFC 5213 inherits the rule with the data structure.
            auto& session = check_and_cast<Pmipv6Timer *>(timer)->session;
            auto it = bindingCache.find(session);
            if (it != bindingCache.end()) {
                EV_INFO << "LMA removed binding for MN '" << session.mnIdentifier
                        << "'; its lifetime expired without a re-registration" << endl;
                it->second.expiryTimer = nullptr;
                deleteBindingCacheEntry(it);
            }
            delete timer;
            break;
        }
        default:
            throw cRuntimeError("Pmipv6: unknown timer '%s' of kind %d", timer->getName(), timer->getKind());
    }
}

void Pmipv6::processMobilityMessage(Packet *packet)
{
    bool accepted = false;
    auto mh = packet->peekAtFront<MobilityHeader>();
    switch (mh->getMobilityHeaderType()) {
        case BINDING_UPDATE: {
            auto bu = packet->peekAtFront<BindingUpdate>();
            if (isLma && bu->getProxyRegistrationFlag()) {
                processProxyBindingUpdate(packet, bu.get());
                accepted = true;
            }
            else
                EV_WARN << "Ignoring Binding Update (not a Proxy Binding Update for an LMA)" << endl;
            break;
        }
        case BINDING_ACKNOWLEDGEMENT: {
            auto ba = packet->peekAtFront<BindingAcknowledgement>();
            if (isMag && ba->getProxyRegistrationFlag()) {
                processProxyBindingAcknowledgement(packet, ba.get());
                accepted = true;
            }
            else
                EV_WARN << "Ignoring Binding Acknowledgement (not a Proxy Binding Acknowledgement for a MAG)" << endl;
            break;
        }
        default:
            EV_WARN << "Ignoring unsupported Mobility Header type " << mh->getMobilityHeaderType() << endl;
            break;
    }
    if (!accepted) {
        dropPacket(packet, NOT_ADDRESSED_TO_US);
        return;
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

void Pmipv6::dropPacket(Packet *packet, PacketDropReason reason)
{
    PacketDropDetails details;
    details.setReason(reason);
    emit(packetDroppedSignal, packet, &details);
    delete packet;
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

//
// RFC 5213 Section 5.4.1: locating the Binding Cache entry for a request is a
// chain of three branches, tried in the order the section specifies, and which
// one applies is decided by the options the request carries.
//
Pmipv6::BindingCache::iterator Pmipv6::lookupBindingCacheEntry(const BindingUpdate *pbu)
{
    // Section 5.4.1.1: the request names a real home network prefix, so the
    // prefix locates the entry regardless of who is asking.
    if (!pbu->getHomeNetworkPrefix().isUnspecified()) {
        for (auto it = bindingCache.begin(); it != bindingCache.end(); ++it)
            if (it->second.homeNetworkPrefix == pbu->getHomeNetworkPrefix())
                return it;
        return bindingCache.end();
    }
    // Section 5.4.1.2: no prefix, but the attached interface is named, so the full
    // mobility session key applies. A gateway here always names the prefix, which it
    // takes from the policy profile, so this branch is unreachable in that
    // configuration; it is the branch a gateway with no configured prefix would need.
    if (!pbu->getMobileNodeLinkLayerIdentifier().isUnspecified()) {
        MobilitySessionKey key;
        key.mnIdentifier = pbu->getMobileNodeIdentifier();
        key.accessTechnologyType = pbu->getAccessTechnologyType();
        key.mnLinkLayerIdentifier = pbu->getMobileNodeLinkLayerIdentifier();
        return bindingCache.find(key);
    }
    // Section 5.4.1.3: neither, so only the mobile node's identifier is left.
    // (Only this section's lookup key is implemented; its multihoming rules,
    // which decide between several sessions of one node, are not.)
    for (auto it = bindingCache.begin(); it != bindingCache.end(); ++it)
        if (it->second.session.mnIdentifier == pbu->getMobileNodeIdentifier())
            return it;
    return bindingCache.end();
}

void Pmipv6::deleteBindingCacheEntry(BindingCache::iterator it)
{
    int tunnelInterfaceId = it->second.tunnelInterfaceId;
    if (it->second.downlinkRoute)
        rt6->deleteRoute(it->second.downlinkRoute);
    cancelAndDelete(it->second.deleteTimer);
    cancelAndDelete(it->second.expiryTimer);
    bindingCache.erase(it);
    emit(bindingCacheSizeSignal, (intval_t)bindingCache.size());
    releaseLmaTunnelIfUnused(tunnelInterfaceId);
}

//
// RFC 5213 Section 5.3.4 step 2 and Section 5.6.1: the tunnel to a gateway exists for
// the mobile nodes reached through it, and goes when the last of them does.
//
void Pmipv6::releaseLmaTunnelIfUnused(int tunnelInterfaceId)
{
    if (tunnelInterfaceId == -1)
        return;
    for (const auto& element : bindingCache)
        if (element.second.tunnelInterfaceId == tunnelInterfaceId)
            return;
    for (auto it = lmaTunnelByMag.begin(); it != lmaTunnelByMag.end(); ++it) {
        if (it->second == tunnelInterfaceId) {
            lmaTunnelByMag.erase(it);
            break;
        }
    }
    if (NetworkInterface *tunnel = ift->getInterfaceById(tunnelInterfaceId)) {
        EV_INFO << "LMA removed the tunnel on interface id " << tunnelInterfaceId
                << "; no mobile node is reached through it" << endl;
        rt6->deleteTunnelNetworkInterface(tunnel);
    }
}

void Pmipv6::sendProxyBindingAcknowledgement(const BindingUpdate *pbu, BaStatus status,
        unsigned int lifetime, uint64_t timestamp, const Ipv6Address& magAddress, const Ipv6Address& lmaAddress)
{
    std::string mnId = pbu->getMobileNodeIdentifier();
    auto reply = new Packet("ProxyBindingAck");
    const auto& pba = makeShared<BindingAcknowledgement>();
    pba->setMobilityHeaderType(BINDING_ACKNOWLEDGEMENT);
    pba->setProxyRegistrationFlag(true);
    pba->setStatus(status);
    pba->setSequenceNumber(pbu->getSequence());
    pba->setLifetime(lifetime);
    // RFC 5213 Section 6.9.1.2 step 6: return the received options unchanged
    pba->setMobileNodeIdentifier(mnId.c_str());
    pba->setMobileNodeLinkLayerIdentifier(pbu->getMobileNodeLinkLayerIdentifier());
    pba->setHomeNetworkPrefix(pbu->getHomeNetworkPrefix());
    pba->setHomeNetworkPrefixLength(pbu->getHomeNetworkPrefixLength());
    pba->setHandoffIndicator(pbu->getHandoffIndicator());
    pba->setAccessTechnologyType(pbu->getAccessTechnologyType());
    pba->setTimestampValue(timestamp);
    pba->setChunkLength(MobilityHeaderSerializer::getProxyBindingAcknowledgementLength(mnId.size()));
    reply->insertAtFront(pba);
    sendMobilityMessage(reply, magAddress, lmaAddress);
}

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
            << "' at " << pbu->getMobileNodeLinkLayerIdentifier()
            << " prefix " << hnp << "/" << hnpLen << " lifetime " << lifetime
            << "s, handoff indicator " << (int)pbu->getHandoffIndicator() << endl;
    emit(proxyBindingUpdateReceivedSignal, (intval_t)lifetime);

    // RFC 6275 Section 10.3.1: the granted lifetime may be shorter than the requested
    // one, and MUST NOT be longer.
    unsigned int grantedLifetime = std::min(lifetime, (unsigned int)maxBindingLifetime.dbl());

    // RFC 5213 Section 5.3.1 step 4: a request that does not say which mobile
    // node it is about cannot be served.
    if (mnId.empty()) {
        EV_WARN << "LMA rejecting Proxy Binding Update without a mobile node identifier" << endl;
        sendProxyBindingAcknowledgement(pbu, MISSING_MN_IDENTIFIER_OPTION, 0, pbu->getTimestampValue(), magAddress, lmaAddress);
        return;
    }

    auto it = lookupBindingCacheEntry(pbu);

    // RFC 5213 Section 5.4.1.1 step 3: a home network prefix belongs to one
    // mobile node, so a request that names another node's prefix is refused.
    if (it != bindingCache.end() && it->second.session.mnIdentifier != mnId) {
        EV_WARN << "LMA rejecting Proxy Binding Update: prefix " << hnp << "/" << hnpLen
                << " belongs to MN '" << it->second.session.mnIdentifier << "', not to '" << mnId << "'" << endl;
        sendProxyBindingAcknowledgement(pbu, NOT_AUTHORIZED_FOR_HOME_NETWORK_PREFIX, 0, pbu->getTimestampValue(), magAddress, lmaAddress);
        return;
    }

    if (lifetime == 0) {
        // RFC 5213 Section 5.3.5 step 1: a deregistration is only honoured from the
        // gateway currently serving the node. Anything else is a stale request from a
        // previous gateway after the node has already moved, and is ignored outright,
        // without an acknowledgement.
        if (it == bindingCache.end() || it->second.servingMagAddress != magAddress) {
            EV_INFO << "LMA ignoring deregistration for MN '" << mnId << "' from " << magAddress
                    << ", which is not the gateway currently serving it" << endl;
            return;
        }
        BindingCacheEntry& entry = it->second;
        // Section 5.3.5 step 2: stop forwarding the mobile node's traffic at once -- the
        // anchor is not on the data path, so removing the prefix route is what drops it --
        // but hold the entry, because the node may be re-registering elsewhere right now.
        if (entry.downlinkRoute) {
            rt6->deleteRoute(entry.downlinkRoute);
            entry.downlinkRoute = nullptr;
        }
        cancelAndDelete(entry.expiryTimer);
        entry.expiryTimer = nullptr;
        if (entry.deleteTimer == nullptr) {
            entry.deleteTimer = new Pmipv6Timer("bindingDelete", LMA_BINDING_DELETE);
            entry.deleteTimer->session = entry.session;
        }
        rescheduleAfter(minDelayBeforeBindingCacheEntryDelete, entry.deleteTimer);
        EV_INFO << "LMA accepted deregistration for MN '" << mnId << "'; holding the binding for "
                << minDelayBeforeBindingCacheEntryDelete << " before deleting it" << endl;
    }
    else {
        // RFC 5213 Section 5.5: a Proxy Binding Update that is not newer than the last
        // one accepted for this mobility session is refused. Without this, a registration
        // delayed in the network -- from the gateway the mobile node has just left, say --
        // would re-point the node's prefix backwards. A rejection carries the anchor's own
        // clock, not the value it was sent, so that the gateway can resynchronise, and a
        // timestamp that merely repeats the last one is a mismatch rather than an older
        // message.
        if (timestampBasedOrdering && it != bindingCache.end()) {
            uint64_t offered = pbu->getTimestampValue();
            if (offered == it->second.timestamp) {
                EV_WARN << "LMA rejecting Proxy Binding Update for MN '" << mnId
                        << "': its timestamp repeats the last one accepted" << endl;
                sendProxyBindingAcknowledgement(pbu, TIMESTAMP_MISMATCH, 0, timestampOf(simTime()), magAddress, lmaAddress);
                return;
            }
            if (offered < it->second.timestamp) {
                EV_WARN << "LMA rejecting Proxy Binding Update for MN '" << mnId
                        << "': it is older than the last one accepted for this mobility session" << endl;
                sendProxyBindingAcknowledgement(pbu, TIMESTAMP_LOWER_THAN_PREV_ACCEPTED, 0, timestampOf(simTime()), magAddress, lmaAddress);
                return;
            }
        }
        // With the timestamp scheme switched off, RFC 5213 Section 9.3 requires the
        // per-session sequence number to order the messages instead. It can only order
        // messages from one gateway: a gateway the mobile node has just moved to counts
        // from its own zero, and how it would learn where the previous gateway had got
        // to is, in the words of Section 5.5, "outside the scope of this document". So
        // the comparison applies while the serving gateway stays the same, and a request
        // from a different gateway is ordered by arrival, which is the whole reason the
        // timestamp scheme is the default.
        else if (!timestampBasedOrdering && it != bindingCache.end()
                && it->second.servingMagAddress == magAddress
                && !isSequenceNumberNewer(seq, it->second.sequenceNumber))
        {
            EV_WARN << "LMA rejecting Proxy Binding Update for MN '" << mnId
                    << "': its sequence number is not newer than the last one accepted" << endl;
            sendProxyBindingAcknowledgement(pbu, SEQUENCE_NUMBER_OUT_OF_WINDOW, 0, 0, magAddress, lmaAddress);
            return;
        }

        // Registration / re-registration / handover.
        int tunnelId = getOrCreateTunnel(lmaAddress, magAddress, lmaTunnelByMag);
        NetworkInterface *tunnel = ift->getInterfaceById(tunnelId);

        MobilitySessionKey key;
        key.mnIdentifier = mnId;
        key.accessTechnologyType = pbu->getAccessTechnologyType();
        key.mnLinkLayerIdentifier = pbu->getMobileNodeLinkLayerIdentifier();
        if (it == bindingCache.end())
            it = bindingCache.insert({ key, BindingCacheEntry() }).first;
        BindingCacheEntry& entry = it->second;
        entry.session = key;

        // Section 5.3.5 step 2: a registration arriving while the entry is being held
        // ends the wait -- the mobility session continues rather than being replaced.
        if (entry.deleteTimer != nullptr) {
            cancelAndDelete(entry.deleteTimer);
            entry.deleteTimer = nullptr;
            EV_INFO << "LMA: MN '" << mnId << "' re-registered within the deletion delay; keeping its binding" << endl;
        }

        // Section 5.3.4: the mobile node is now reached through a different gateway, so
        // its prefix route moves to that gateway's tunnel.
        bool retargeted = entry.tunnelInterfaceId != -1 && entry.tunnelInterfaceId != tunnelId;
        if (retargeted) {
            if (entry.downlinkRoute) {
                rt6->deleteRoute(entry.downlinkRoute);
                entry.downlinkRoute = nullptr;
            }
            emit(homeNetworkPrefixReanchoredSignal, (intval_t)1);
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
        entry.timestamp = pbu->getTimestampValue();
        entry.expiry = simTime() + grantedLifetime;
        int previousTunnelId = entry.tunnelInterfaceId;
        entry.tunnelInterfaceId = tunnelId;
        if (previousTunnelId != -1 && previousTunnelId != tunnelId)
            releaseLmaTunnelIfUnused(previousTunnelId);
        // RFC 6275 Section 9.6: the entry lives exactly as long as the lifetime granted
        if (entry.expiryTimer == nullptr) {
            entry.expiryTimer = new Pmipv6Timer("bindingExpiry", LMA_BINDING_EXPIRY);
            entry.expiryTimer->session = entry.session;
        }
        rescheduleAfter(SimTime(grantedLifetime, SIMTIME_S), entry.expiryTimer);
        emit(bindingCacheSizeSignal, (intval_t)bindingCache.size());
    }

    sendProxyBindingAcknowledgement(pbu, BINDING_UPDATE_ACCEPTED, grantedLifetime, pbu->getTimestampValue(), magAddress, lmaAddress);
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
        if (const char *lla = child->getAttribute("linkLayerAddress"))
            profile.linkLayerAddress = MacAddress(lla);
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

//
// RFC 5213 Section 6.2: a policy profile belongs to a mobile node, so the
// station that attached selects it. A profile without a link-layer address
// matches any station on a matching access interface, which keeps a
// single-mobile-node configuration working without naming the station;
// with more than one station on a link, such a profile is ambiguous and the
// caller refuses the attachment rather than serving two nodes as one.
//
const Pmipv6::MobileNodeProfile *Pmipv6::findProfile(NetworkInterface *accessInterface, const MacAddress& stationAddress) const
{
    const char *name = accessInterface->getInterfaceName();
    const MobileNodeProfile *anyStation = nullptr;
    for (const auto& profile : mobileNodeProfiles) {
        if (!profile.accessInterfaceName.empty() && profile.accessInterfaceName != name)
            continue;
        if (profile.linkLayerAddress == stationAddress)
            return &profile;
        if (profile.linkLayerAddress.isUnspecified() && anyStation == nullptr)
            anyStation = &profile;
    }
    return anyStation;
}

Pmipv6::MagBinding *Pmipv6::findBinding(int accessInterfaceId, const MacAddress& stationAddress)
{
    for (auto& element : magBindings) {
        MagBinding& binding = element.second;
        if (binding.accessInterfaceId == accessInterfaceId && binding.mnLinkLayerIdentifier == stationAddress)
            return &binding;
    }
    return nullptr;
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
        // the notification names the station, which is how the gateway learns
        // which mobile node attached rather than merely that one did
        auto notification = dynamic_cast<ieee80211::Ieee80211MgmtAp::NotificationInfoSta *>(obj);
        if (notification == nullptr) {
            EV_WARN << "Attachment notification on " << accessInterface->getInterfaceName()
                    << " does not name a station; ignoring it" << endl;
            return;
        }
        if (signalID == l2ApAssociatedSignal)
            handleMobileNodeAttached(accessInterface, notification->getStaAddress());
        else
            handleMobileNodeDetached(accessInterface, notification->getStaAddress());
    }
    else if (signalID == linkBrokenSignal) {
        // the access link gave up on a frame addressed to a station: RFC 5213
        // Section 6.13 lists a link-layer event as one way to learn that a
        // mobile node is no longer on the connected link
        NetworkInterface *accessInterface = getContainingNicModule(check_and_cast<cModule *>(source));
        auto packet = dynamic_cast<Packet *>(obj);
        if (accessInterface == nullptr || packet == nullptr)
            return;
        // other medium access control modules emit this signal too, carrying their own
        // frame formats, and an access gateway's radio need not be 802.11
        const auto& header = packet->peekAtFront<ieee80211::Ieee80211DataOrMgmtHeader>(b(-1), Chunk::PF_ALLOW_NULLPTR);
        if (header == nullptr) {
            EV_DETAIL << "Link break on " << accessInterface->getInterfaceName()
                      << " reported for a frame this module cannot read; ignoring it" << endl;
            return;
        }
        handleMobileNodeDetached(accessInterface, header->getReceiverAddress());
    }
    else if (signalID == packetReceivedFromLowerSignal) {
        // anything arriving from a station is evidence that it is still there
        auto packet = dynamic_cast<Packet *>(obj);
        if (packet == nullptr)
            return;
        auto interfaceInd = packet->findTag<InterfaceInd>();
        auto macAddressInd = packet->findTag<MacAddressInd>();
        if (interfaceInd != nullptr && macAddressInd != nullptr)
            noteMobileNodePresence(interfaceInd->getInterfaceId(), macAddressInd->getSrcAddress());
    }
}

//
// RFC 5213 Section 6.13 requires the gateway to know whether a mobile node is
// still on the connected link, both before it extends a binding and before it
// retransmits an unanswered Proxy Binding Update. This is that question, asked
// in one place, rather than a one-shot reaction to a detachment event.
//
// The answer is whatever the last evidence said. Silence is not evidence, which
// is why nothing here times out: a node that has gone quiet is asked (see
// checkMobileNodePresence) and only an unanswered question makes it absent.
//
bool Pmipv6::isMobileNodePresent(const MagBinding& binding) const
{
    return !binding.detached;
}

void Pmipv6::noteMobileNodePresence(int accessInterfaceId, const MacAddress& stationAddress)
{
    if (MagBinding *binding = findBinding(accessInterfaceId, stationAddress)) {
        binding->lastPresence = simTime();
        binding->probeDeadline = 0; // it spoke, so whatever was asked is answered
    }
}

//
// RFC 5213 Section 6.13 leaves the detection method to the access technology and
// names four acceptable classes, one of which is an IPv6 Neighbour Unreachability
// Detection event. That is the one used here, and it is used because it is the
// only one that produces evidence on demand: waiting for a mobile node to say
// something of its own accord cannot distinguish an idle node from a departed
// one, and a gateway with nothing to send it never finds out either way.
//
// So when a node has been quiet for presenceProbeDelay, the gateway asks. A node
// that is there answers the solicitation and is seen again through the ordinary
// evidence path; a node that does not answer within presenceProbeTimeout has
// gone.
//
void Pmipv6::checkMobileNodePresence()
{
    std::vector<std::string> gone;
    for (auto& element : magBindings) {
        MagBinding& binding = element.second;
        if (binding.deregistering || binding.detached)
            continue;
        if (simTime() - binding.lastPresence <= presenceProbeDelay) {
            binding.probeDeadline = 0;
            continue;
        }
        if (binding.probeDeadline == 0) {
            // Ask, and wait as long as the answer can take. Neighbour Discovery
            // reports that, because it depends on the access link's own constants:
            // an exchange for a neighbour it already knows about begins with a
            // five-second delay before any solicitation is sent, and a deadline
            // chosen here would have to know that.
            simtime_t answerBy = ipv6nd->probeNeighbourReachability(binding.mnLinkLocalAddress,
                    binding.accessInterfaceId);
            if (answerBy <= SIMTIME_ZERO) {
                EV_WARN << "MAG: cannot ask whether mobile node '" << binding.mnIdentifier
                        << "' is still there; leaving its binding alone" << endl;
                continue;
            }
            EV_DETAIL << "MAG: nothing heard from mobile node '" << binding.mnIdentifier << "' for "
                      << presenceProbeDelay << "; asking, and allowing " << answerBy << " for an answer" << endl;
            binding.probeDeadline = simTime() + answerBy;
        }
        else if (simTime() >= binding.probeDeadline)
            gone.push_back(element.first);
    }
    for (const auto& mnIdentifier : gone) {
        MagBinding& binding = magBindings[mnIdentifier];
        EV_INFO << "MAG: mobile node '" << binding.mnIdentifier << "' (" << binding.mnLinkLayerIdentifier
                << ") did not answer; treating it as detached" << endl;
        binding.detached = true;
        binding.probeDeadline = 0;
        deregisterMobileNode(binding);
    }
}

void Pmipv6::handleMobileNodeAttached(NetworkInterface *accessInterface, const MacAddress& stationAddress)
{
    // RFC 5213 Section 6.9.1.1 step 1: identify the mobile node first; only then
    // is there anything to register.
    const MobileNodeProfile *profile = findProfile(accessInterface, stationAddress);
    if (profile == nullptr) {
        EV_DETAIL << "No mobile-node profile for station " << stationAddress << " on "
                  << accessInterface->getInterfaceName() << "; ignoring attachment" << endl;
        return;
    }

    auto existing = magBindings.find(profile->mnIdentifier);
    if (existing != magBindings.end() && existing->second.mnLinkLayerIdentifier != stationAddress) {
        EV_WARN << "Station " << stationAddress << " on " << accessInterface->getInterfaceName()
                << " resolves to mobile node '" << profile->mnIdentifier << "', which is already served for station "
                << existing->second.mnLinkLayerIdentifier
                << "; give the two stations separate profiles. Ignoring attachment" << endl;
        return;
    }

    EV_INFO << "MAG: mobile node '" << profile->mnIdentifier << "' (" << stationAddress << ") attached on "
            << accessInterface->getInterfaceName() << "; sending Proxy Binding Update" << endl;

    MagBinding& binding = magBindings[profile->mnIdentifier];
    // A mobile node that comes back is not one that never left. Whatever the previous
    // attachment left behind has to go, or the binding stays wedged: a deregistration
    // still marked outstanding suppresses every later detachment and every refresh,
    // and an access-link route left pointing at the interface the node used to be on
    // sends its traffic out of the wrong radio.
    binding.deregistering = false;
    cancelAndDelete(binding.refreshTimer);
    binding.refreshTimer = nullptr;
    if (binding.downlinkRoute != nullptr && binding.accessInterfaceId != accessInterface->getInterfaceId()) {
        rt6->deleteRoute(binding.downlinkRoute);
        binding.downlinkRoute = nullptr;
    }

    binding.mnIdentifier = profile->mnIdentifier;
    binding.mnLinkLayerIdentifier = stationAddress;
    // The address a presence probe asks about. A mobile node's link-local address is
    // formed from its interface's link-layer address by the same rule the node itself
    // uses, so the gateway can name it without ever having been told it.
    //
    // LIMITATION: that rule is the one INET's own IPv6 host follows. A node that forms
    // its link-local address some other way -- a stable privacy identifier, or one
    // configured by hand -- would not answer to this address, and the gateway would
    // conclude it had gone. Such a node needs presenceProbeDelay set to 0, leaving
    // detection to the link-layer events.
    binding.mnLinkLocalAddress = Ipv6Address::formLinkLocalAddress(stationAddress.formInterfaceIdentifier());
    binding.probeDeadline = 0;
    binding.accessTechnologyType = ACCESS_TECHNOLOGY_IEEE_802_11;
    binding.homeNetworkPrefix = profile->homeNetworkPrefix;
    binding.homeNetworkPrefixLength = profile->homeNetworkPrefixLength;
    binding.accessInterfaceId = accessInterface->getInterfaceId();
    binding.sequenceNumber++;
    binding.registered = false;
    binding.detached = false;
    binding.lastPresence = simTime();
    binding.retransmitInterval = initialBindingAckTimeout;
    // RFC 5213 Section 6.9.1.1 step 4 offers four values for an attachment, and step 5
    // forbids claiming a handoff between interfaces or between gateways unless the
    // gateway can establish that it happened. A gateway here learns of an attachment
    // from the access link alone; nothing tells it whether the node is new to the
    // domain or has just left another gateway. Only "handoff state unknown" is
    // truthful, and it is also the value the standard provides for exactly this case.
    sendProxyBindingUpdate(binding, bindingLifetime, HANDOFF_STATE_UNKNOWN);
}

void Pmipv6::handleMobileNodeDetached(NetworkInterface *accessInterface, const MacAddress& stationAddress)
{
    MagBinding *binding = findBinding(accessInterface->getInterfaceId(), stationAddress);
    if (binding == nullptr || binding->deregistering)
        return;
    EV_INFO << "MAG: mobile node '" << binding->mnIdentifier << "' (" << stationAddress
            << ") is no longer on " << accessInterface->getInterfaceName() << endl;
    binding->detached = true;
    deregisterMobileNode(*binding);
}

//
// RFC 5213 Section 6.9.1.4: stop serving the mobile node's home network prefix
// on the access link and tell the anchor, with a Proxy Binding Update whose
// lifetime is zero. The binding is kept until the anchor answers, because the
// acknowledgement is what releases the rest of the state.
//
void Pmipv6::deregisterMobileNode(MagBinding& binding)
{
    emit(mobileNodeDetachedSignal, (intval_t)1);
    EV_INFO << "MAG: deregistering mobile node '" << binding.mnIdentifier << "'" << endl;
    withdrawHomeNetworkPrefix(binding);
    binding.registered = false;
    binding.deregistering = true;
    binding.sequenceNumber++;
    // the deregistration waits INITIAL_BINDACK_TIMEOUT, not whatever interval a
    // preceding unanswered registration had backed off to
    binding.retransmitInterval = initialBindingAckTimeout;
    // RFC 5213 Section 6.9.1.4: lifetime 0 deregisters, the prefixes are named in
    // full rather than left all-zero, and the handoff state is unknown
    sendProxyBindingUpdate(binding, 0, HANDOFF_STATE_UNKNOWN);
}

//
// Stop emulating the mobile node's home network on the access link: the prefix leaves
// the Router Advertisements and the route that delivered its traffic goes.
//
void Pmipv6::withdrawHomeNetworkPrefix(MagBinding& binding)
{
    if (NetworkInterface *accessInterface = ift->getInterfaceById(binding.accessInterfaceId)) {
        if (auto ipv6Data = accessInterface->findProtocolDataForUpdate<Ipv6InterfaceData>()) {
            for (int i = 0; i < ipv6Data->getNumAdvPrefixes(); i++) {
                if (ipv6Data->getAdvPrefix(i).prefix == binding.homeNetworkPrefix) {
                    ipv6Data->removeAdvPrefix(i);
                    break;
                }
            }
        }
    }
    if (binding.downlinkRoute) {
        rt6->deleteRoute(binding.downlinkRoute);
        binding.downlinkRoute = nullptr;
    }
}

//
// RFC 5213 Section 6.9.1.4 cleanup: drop the Binding Update List entry, and with
// it the tunnel to the anchor once no mobile node is using that tunnel any more.
//
void Pmipv6::releaseMagBinding(MagBinding& binding)
{
    std::string mnIdentifier = binding.mnIdentifier;
    cancelAndDelete(binding.retransmitTimer);
    binding.retransmitTimer = nullptr;
    cancelAndDelete(binding.refreshTimer);
    binding.refreshTimer = nullptr;
    if (binding.downlinkRoute) {
        rt6->deleteRoute(binding.downlinkRoute);
        binding.downlinkRoute = nullptr;
    }
    magBindings.erase(mnIdentifier);
    if (magBindings.empty() && magTunnelId != -1) {
        if (magUplinkRoute) {
            rt6->deleteRoute(magUplinkRoute);
            magUplinkRoute = nullptr;
        }
        if (NetworkInterface *tunnel = ift->getInterfaceById(magTunnelId))
            rt6->deleteTunnelNetworkInterface(tunnel);
        EV_INFO << "MAG: removed the tunnel to the LMA; no mobile node is using it" << endl;
        magTunnelId = -1;
    }
}

void Pmipv6::sendProxyBindingUpdate(MagBinding& binding, simtime_t lifetime, uint8_t handoffIndicator)
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
    pbu->setMobileNodeLinkLayerIdentifier(binding.mnLinkLayerIdentifier);
    pbu->setHomeNetworkPrefix(binding.homeNetworkPrefix);
    pbu->setHomeNetworkPrefixLength(binding.homeNetworkPrefixLength);
    pbu->setHandoffIndicator(handoffIndicator);
    pbu->setAccessTechnologyType(binding.accessTechnologyType);
    pbu->setTimestampValue(timestampBasedOrdering ? timestampOf(simTime()) : 0);
    pbu->setChunkLength(MobilityHeaderSerializer::getProxyBindingUpdateLength(binding.mnIdentifier.size()));
    packet->insertAtFront(pbu);
    emit(proxyBindingUpdateSentSignal, (intval_t)pbu->getLifetime());
    sendMobilityMessage(packet, localMobilityAnchorAddress, magAddress);

    // RFC 5213 Section 6.9.4: wait for the acknowledgement, and try again if none comes
    binding.pendingLifetime = lifetime;
    binding.pendingHandoffIndicator = handoffIndicator;
    if (binding.retransmitTimer == nullptr) {
        binding.retransmitTimer = new Pmipv6Timer("bindingAckTimeout", MAG_BINDING_RETRANSMIT);
        binding.retransmitTimer->session.mnIdentifier = binding.mnIdentifier;
        binding.retransmitTimer->session.accessTechnologyType = binding.accessTechnologyType;
        binding.retransmitTimer->session.mnLinkLayerIdentifier = binding.mnLinkLayerIdentifier;
        binding.retransmitInterval = initialBindingAckTimeout;
    }
    rescheduleAfter(binding.retransmitInterval, binding.retransmitTimer);
}

//
// RFC 5213 Section 6.9.4: an unanswered Proxy Binding Update is sent again, with
// the interval doubling each time up to MAX_BINDACK_TIMEOUT -- but only after the
// gateway has confirmed that the mobile node is still on the access link, which
// step 2 of that section makes a MUST.
//
void Pmipv6::retransmitProxyBindingUpdate(const MobilitySessionKey& session)
{
    auto it = magBindings.find(session.mnIdentifier);
    if (it == magBindings.end())
        return;
    MagBinding& binding = it->second;

    if (binding.deregistering) {
        // Section 6.9.1.4: the local state goes either on the acknowledgement or
        // after INITIAL_BINDACK_TIMEOUT without one. The node has gone either way.
        EV_INFO << "MAG: no acknowledgement for the deregistration of MN '" << binding.mnIdentifier
                << "'; releasing its state anyway" << endl;
        releaseMagBinding(binding);
        return;
    }
    if (!isMobileNodePresent(binding)) {
        EV_INFO << "MAG: mobile node '" << binding.mnIdentifier
                << "' is no longer on the access link; abandoning its registration" << endl;
        releaseMagBinding(binding);
        return;
    }
    binding.retransmitInterval = std::min(binding.retransmitInterval * 2, maxBindingAckTimeout);
    binding.sequenceNumber++; // Section 6.9.4 step 4: strictly greater than the previous attempt
    EV_INFO << "MAG: no acknowledgement for MN '" << binding.mnIdentifier << "'; sending the Proxy Binding Update again"
            << " (next attempt in " << binding.retransmitInterval << ")" << endl;
    sendProxyBindingUpdate(binding, binding.pendingLifetime, binding.pendingHandoffIndicator);
}

//
// RFC 5213 Section 6.9.1.3: extending a binding is an ordinary Proxy Binding
// Update with the same options and a handoff indicator saying nothing changed.
// Section 6.13 forbids extending it at all for a node whose presence the
// gateway cannot confirm, so the presence check comes first and a node that has
// gone is deregistered instead.
//
void Pmipv6::refreshProxyBinding(const MobilitySessionKey& session)
{
    auto it = magBindings.find(session.mnIdentifier);
    if (it == magBindings.end())
        return;
    MagBinding& binding = it->second;
    if (binding.deregistering)
        return;
    if (!isMobileNodePresent(binding)) {
        EV_INFO << "MAG: cannot confirm that mobile node '" << binding.mnIdentifier
                << "' is still attached; deregistering it instead of extending its binding" << endl;
        binding.detached = true;
        deregisterMobileNode(binding);
        return;
    }
    EV_INFO << "MAG: extending the binding of MN '" << binding.mnIdentifier << "'" << endl;
    binding.sequenceNumber++;
    binding.retransmitInterval = initialBindingAckTimeout;
    sendProxyBindingUpdate(binding, bindingLifetime, HANDOFF_REREGISTRATION);
}

Pmipv6::MagBinding *Pmipv6::findBindingForSource(int accessInterfaceId, const Ipv6Address& sourceAddress)
{
    for (auto& element : magBindings) {
        MagBinding& binding = element.second;
        if (binding.accessInterfaceId == accessInterfaceId
                && sourceAddress.matches(binding.homeNetworkPrefix, binding.homeNetworkPrefixLength))
            return &binding;
    }
    return nullptr;
}

//
// RFC 5213 Section 6.10.5. A packet a gateway forwards off an access link gets two
// decisions, and both are about where it came FROM, not where it is going:
//
//   - it must come from a mobile node this gateway is serving, or it is not forwarded
//     at all. Without that check the access link is an open relay: anything attached
//     to the radio can source a packet and have it routed.
//
//   - and it goes to the mobile node's anchor, through the tunnel, whatever its
//     destination. That is what "reverse tunnelling" is, and it is why a route cannot
//     express it: IPv6 forwarding chooses an interface from the destination, so the
//     default route into the tunnel only ever wins for destinations no other route
//     covers -- which, with an address configurator running, is none of them.
//
// This is why the gateway registers a pre-routing hook rather than installing more
// routes. The hook nominates the tunnel as the output interface and lets the ordinary
// routing take it from there.
//
INetfilter::IHook::Result Pmipv6::datagramPreRoutingHook(Packet *datagram)
{
    auto interfaceInd = datagram->findTag<InterfaceInd>();
    if (interfaceInd == nullptr)
        return ACCEPT;
    int arrivalInterfaceId = interfaceInd->getInterfaceId();

    // only the access links this gateway serves are subject to any of this; the
    // backhaul, the tunnel and everything else route as they always did
    bool isAccessLink = false;
    for (const auto& element : magBindings)
        if (element.second.accessInterfaceId == arrivalInterfaceId)
            isAccessLink = true;
    if (!isAccessLink)
        return ACCEPT;

    const auto& ipv6Header = datagram->peekAtFront<Ipv6Header>();
    Ipv6Address source = ipv6Header->getSourceAddress().toIpv6();
    Ipv6Address destination = ipv6Header->getDestinationAddress().toIpv6();

    // Neighbour and Router Discovery live on the link and are addressed to it, so they
    // are not traffic the gateway forwards anywhere and none of this applies to them.
    if (source.isUnspecified() || source.isLinkLocal() || destination.isLinkLocal()
            || destination.isMulticast())
        return ACCEPT;

    MagBinding *binding = findBindingForSource(arrivalInterfaceId, source);
    if (binding == nullptr) {
        EV_WARN << "Dropping a packet from " << source << " on "
                << ift->getInterfaceById(arrivalInterfaceId)->getInterfaceName()
                << ": no mobile node served here owns that address" << endl;
        emit(packetDroppedSignal, datagram);
        return DROP;
    }
    if (!binding->registered || magTunnelId == -1) {
        EV_WARN << "Dropping a packet from mobile node '" << binding->mnIdentifier
                << "': its registration is not complete" << endl;
        emit(packetDroppedSignal, datagram);
        return DROP;
    }

    EV_DETAIL << "Reverse-tunnelling a packet from mobile node '" << binding->mnIdentifier
              << "' to " << destination << endl;
    datagram->addTagIfAbsent<InterfaceReq>()->setInterfaceId(magTunnelId);
    return ACCEPT;
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
    emit(proxyBindingAcknowledgementReceivedSignal, (intval_t)lifetime);

    auto it = magBindings.find(mnId);
    if (it == magBindings.end()) {
        EV_INFO << "MAG: Proxy Binding Acknowledgement for unknown MN '" << mnId << "'; ignoring" << endl;
        return;
    }
    MagBinding& binding = it->second;

    // RFC 5213 Section 6.9.1.2 step 6 has the anchor return the options it received,
    // with identical values. An acknowledgement that does not match what this gateway
    // asked for is not an answer to it.
    if (pba->getHomeNetworkPrefix() != binding.homeNetworkPrefix
            || pba->getMobileNodeLinkLayerIdentifier() != binding.mnLinkLayerIdentifier
            || pba->getAccessTechnologyType() != binding.accessTechnologyType
            || pba->getHandoffIndicator() != binding.pendingHandoffIndicator)
    {
        EV_WARN << "MAG: Proxy Binding Acknowledgement for MN '" << mnId
                << "' does not echo the options that were sent; ignoring it" << endl;
        return;
    }

    cancelAndDelete(binding.retransmitTimer);
    binding.retransmitTimer = nullptr;
    binding.retransmitInterval = initialBindingAckTimeout;

    if (status != BINDING_UPDATE_ACCEPTED) {
        // RFC 5213 Section 6.9.1.2 step 11 and Section 6.9.2 step 2: with the
        // registration refused the gateway must not go on emulating the node's home
        // network. A re-registration that is refused leaves state from the registration
        // before it, and that state has to go too.
        EV_WARN << "MAG: Proxy Binding Update for MN '" << mnId << "' rejected (status " << status
                << "); withdrawing its home network prefix" << endl;
        withdrawHomeNetworkPrefix(binding);
        releaseMagBinding(binding);
        return;
    }
    if (lifetime == 0) {
        if (!binding.deregistering) {
            // the mobile node came back before the anchor answered, so this
            // acknowledgement is about a session the gateway is serving again
            EV_INFO << "MAG: late deregistration acknowledgement for MN '" << mnId
                    << "', which has since re-attached; keeping its binding" << endl;
            return;
        }
        EV_INFO << "MAG: deregistration acknowledged for MN '" << mnId << "'" << endl;
        releaseMagBinding(binding);
        return;
    }

    EV_INFO << "MAG: binding accepted for MN '" << mnId << "' prefix " << binding.homeNetworkPrefix
            << "/" << binding.homeNetworkPrefixLength << " for " << lifetime << "s" << endl;

    // RFC 6275 Section 11.7.3: extend the binding well before it runs out, so that
    // network delay does not cost the mobile node its session.
    simtime_t refreshAfter = lifetime * bindingRefreshRatio;
    if (refreshAfter > 0) {
        if (binding.refreshTimer == nullptr) {
            binding.refreshTimer = new Pmipv6Timer("bindingRefresh", MAG_BINDING_REFRESH);
            binding.refreshTimer->session.mnIdentifier = binding.mnIdentifier;
            binding.refreshTimer->session.accessTechnologyType = binding.accessTechnologyType;
            binding.refreshTimer->session.mnLinkLayerIdentifier = binding.mnLinkLayerIdentifier;
        }
        rescheduleAfter(refreshAfter, binding.refreshTimer);
    }

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

//
// A gateway or anchor that is shut down or crashes stops serving every mobility
// session it held: its timers go, and so do the routes and the prefix advertisements
// it installed. Nothing is restored on a restart, because a mobility session is
// re-established by the mobile node attaching again, which is the only event that
// tells the network where the node is.
//
// The tunnel interfaces are the exception, and deliberately so: they are submodules
// of the node, and deleting one while the lifecycle operation is walking that node's
// submodules aborts the simulation. They are released on the ordinary deregistration
// paths, which do not run inside a lifecycle operation.
//
void Pmipv6::releaseAllState(bool deleteTunnels)
{
    if (presenceCheckTimer != nullptr)
        cancelEvent(presenceCheckTimer);
    for (auto it = bindingCache.begin(); it != bindingCache.end(); ) {
        cancelAndDelete(it->second.deleteTimer);
        cancelAndDelete(it->second.expiryTimer);
        if (it->second.downlinkRoute)
            rt6->deleteRoute(it->second.downlinkRoute);
        it = bindingCache.erase(it);
    }
    if (deleteTunnels) {
        for (const auto& element : lmaTunnelByMag) {
            if (NetworkInterface *tunnel = ift->getInterfaceById(element.second))
                rt6->deleteTunnelNetworkInterface(tunnel);
        }
        lmaTunnelByMag.clear();
    }
    if (isLma)
        emit(bindingCacheSizeSignal, (intval_t)bindingCache.size());

    for (auto it = magBindings.begin(); it != magBindings.end(); ) {
        cancelAndDelete(it->second.retransmitTimer);
        cancelAndDelete(it->second.refreshTimer);
        withdrawHomeNetworkPrefix(it->second);
        it = magBindings.erase(it);
    }
    if (magUplinkRoute) {
        rt6->deleteRoute(magUplinkRoute);
        magUplinkRoute = nullptr;
    }
    if (deleteTunnels && magTunnelId != -1) {
        if (NetworkInterface *tunnel = ift->getInterfaceById(magTunnelId))
            rt6->deleteTunnelNetworkInterface(tunnel);
        magTunnelId = -1;
    }
}

void Pmipv6::handleStopOperation(LifecycleOperation *operation)
{
    releaseAllState(false);
}

void Pmipv6::handleCrashOperation(LifecycleOperation *operation)
{
    releaseAllState(false);
}

} // namespace inet

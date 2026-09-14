//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mib/Ieee80211Mib.h"

#include <algorithm>
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Band.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"

namespace inet {

namespace ieee80211 {

Define_Module(Ieee80211Mib);

simsignal_t Ieee80211Mib::bssStateChangedSignal = cComponent::registerSignal("bssStateChanged");

void Ieee80211Mib::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        WATCH(address);
        WATCH(mode);
        WATCH(qos);
        WATCH(localHtCapabilitiesValid);
        WATCH(localCapabilitiesPrepared);
        WATCH(primaryChannelAvailable);
        WATCH(bssActive);
        WATCH(htOperationPresent);
        WATCH(bssData.bssid);
        WATCH(bssStationData.stationType);
        WATCH(bssStationData.isAssociated);
        WATCH(bssAccessPointData.stations);
        WATCH(bssAccessPointData.associationIds);
        WATCH(associationIdReservations);
        WATCH_EXPR("modeStr", getModeStr(mode));
        WATCH_EXPR("stationTypeStr", getStationTypeStr(bssStationData.stationType));
        WATCH_EXPR("qosStr", qos ? ", QoS" : ", Non-QoS");
        WATCH_EXPR("ssidStr", getSsidStr());
        WATCH_EXPR("ssid", bssData.ssid.empty() ? std::string("-") : bssData.ssid); // associated SSID ("-" if none), for node display strings
        WATCH_EXPR("associatedStr", bssStationData.stationType == STATION ? (bssStationData.isAssociated ? "\nAssociated" : "\nNot associated") : "");
        WATCH_EXPR("primaryChannel", primaryChannelAvailable ? std::to_string(htOperation.primaryChannel) : "unavailable");
    }
}

void Ieee80211Mib::checkStateMutation() const
{
    if (publishingStateChange)
        throw cRuntimeError("Cannot mutate IEEE 802.11 state during bssStateChanged notification");
}

void Ieee80211Mib::commitBss(const std::string& ssid, const MacAddress& bssid, const physicallayer::IIeee80211Band *band,
        int channel, const Ieee80211HtOperation *operation)
{
    checkStateMutation();
    if (channel < -1 || channel > 255 || (operation != nullptr && (channel < 0 || operation->primaryChannel != channel)))
        throw cRuntimeError("Inconsistent IEEE 802.11 BSS channel snapshot");
    if (operation != nullptr && band != nullptr)
        band->getStandardChannelNumber(channel);
    bool changed = !bssActive || bssData.ssid != ssid || bssData.bssid != bssid || operationBand != band ||
            primaryChannelAvailable != (channel >= 0) || (channel >= 0 && htOperation.primaryChannel != channel) ||
            htOperationPresent != (operation != nullptr) || (operation != nullptr && !(htOperation == *operation));
    bssData.ssid = ssid;
    bssData.bssid = bssid;
    bssActive = true;
    operationBand = band;
    primaryChannelAvailable = channel >= 0;
    htOperationPresent = operation != nullptr;
    htOperation = operation != nullptr ? *operation : Ieee80211HtOperation();
    htOperation.primaryChannel = channel;
    stateChangePending |= changed;
}

void Ieee80211Mib::clearBss()
{
    checkStateMutation();
    stateChangePending |= bssActive || !peerHtStates.empty() || !bssAccessPointData.stations.empty() ||
            !bssAccessPointData.associationIds.empty();
    bssActive = false;
    htOperationPresent = false;
    primaryChannelAvailable = false;
    operationBand = nullptr;
    bssStationData.isAssociated = false;
    bssAccessPointData.stations.clear();
    bssAccessPointData.associationIds.clear();
    associationIdReservations.clear();
    peerHtStates.clear();
}

void Ieee80211Mib::publishStateChange()
{
    Enter_Method("publishStateChange");
    checkStateMutation();
    if (!stateChangePending)
        return;
    stateChangePending = false;
    publishingStateChange = true;
    try {
        // No borrowed payload: observers query the committed MIB during this callback.
        emit(bssStateChangedSignal, bssActive);
    }
    catch (...) {
        publishingStateChange = false;
        throw;
    }
    publishingStateChange = false;
}

void Ieee80211Mib::configureBssRole(BssStationType stationType, const std::string& ssid)
{
    checkStateMutation();
    if (bssActive)
        throw cRuntimeError("Cannot configure an active BSS role");
    bssStationData.stationType = stationType;
    bssData.ssid = ssid;
}

void Ieee80211Mib::setAssociated(bool associated)
{
    checkStateMutation();
    stateChangePending |= bssStationData.isAssociated != associated;
    bssStationData.isAssociated = associated;
}

Ieee80211Mib::BssMemberStatus Ieee80211Mib::getPeerAssociationStatus(const MacAddress& address) const
{
    auto it = bssAccessPointData.stations.find(address);
    return it == bssAccessPointData.stations.end() ? NOT_AUTHENTICATED : it->second;
}

void Ieee80211Mib::setPeerAssociationStatus(const MacAddress& address, BssMemberStatus status)
{
    checkStateMutation();
    auto it = bssAccessPointData.stations.find(address);
    stateChangePending |= it == bssAccessPointData.stations.end() || it->second != status;
    bssAccessPointData.stations[address] = status;
}

void Ieee80211Mib::removePeerAssociation(const MacAddress& address)
{
    checkStateMutation();
    stateChangePending |= bssAccessPointData.stations.erase(address) != 0;
    releaseAssociationId(address);
}

int Ieee80211Mib::requirePrimaryChannel() const
{
    if (!primaryChannelAvailable)
        throw cRuntimeError("IEEE 802.11 primary channel is unavailable");
    return htOperation.primaryChannel;
}

const Ieee80211HtOperation& Ieee80211Mib::getHtOperation() const
{
    if (!hasHtOperation())
        throw cRuntimeError("No committed IEEE 802.11 HT operation is available");
    return htOperation;
}

void Ieee80211Mib::installLocalHtCapabilities(const Ieee80211HtCapabilities& capabilities, bool htSupported)
{
    checkStateMutation();
    if (localCapabilitiesPrepared && localHtCapabilities == capabilities && localHtCapabilitiesValid == htSupported)
        return;
    if ((localCapabilitiesPrepared && bssActive) || !peerHtStates.empty())
        throw cRuntimeError("Cannot replace local HT capabilities with active BSS or peer relationships");
    localHtCapabilities = capabilities;
    localHtCapabilitiesValid = htSupported;
    localCapabilitiesPrepared = true;
}

const Ieee80211Mib::PeerHtState *Ieee80211Mib::findPeerCapabilities(const MacAddress& address) const
{
    auto it = peerHtStates.find(address);
    return it == peerHtStates.end() || !it->second.valid ? nullptr : &it->second;
}

bool Ieee80211Mib::relationshipAllowsHt(const MacAddress& address) const
{
    const auto *peer = findPeerCapabilities(address);
    if (!isLocalHtCapable() || !hasHtOperation() || peer == nullptr || !peer->negotiatedCapabilities)
        return false;
    const auto& capabilities = *peer->negotiatedCapabilities;
    return capabilities.localTxPeerRx.valid && capabilities.localRxPeerTx.valid &&
            supportsBasicHtMcsSet(bssStationData.stationType == ACCESS_POINT ? peer->advertisedCapabilities : localHtCapabilities, htOperation);
}

const Ieee80211Mib::PeerHtState *Ieee80211Mib::findPeerHtState(const MacAddress& address) const
{
    return relationshipAllowsHt(address) ? findPeerCapabilities(address) : nullptr;
}

void Ieee80211Mib::setPeerHtCapabilities(const MacAddress& address, const Ieee80211HtCapabilities& capabilities)
{
    checkStateMutation();
    if (!localHtCapabilitiesValid)
        throw cRuntimeError("Cannot install peer HT capabilities when local HT is disabled");
    auto& state = peerHtStates[address];
    if (state.valid && state.advertisedCapabilities == capabilities && state.negotiatedCapabilities &&
            state.negotiatedCapabilities->localAdvertisement == localHtCapabilities)
        return;
    auto derived = std::make_shared<const Ieee80211NegotiatedHtCapabilities>(negotiateHtCapabilities(localHtCapabilities, capabilities));
    state.advertisedCapabilities = capabilities;
    state.negotiatedCapabilities = derived;
    state.valid = true;
    stateChangePending = true;
    EV_INFO << "Installed peer HT state, peer = " << address
            << ", txValid = " << derived->localTxPeerRx.valid
            << ", rxValid = " << derived->localRxPeerTx.valid << endl;
}

void Ieee80211Mib::removePeerHtCapabilities(const MacAddress& address)
{
    checkStateMutation();
    stateChangePending |= peerHtStates.erase(address) != 0;
}

void Ieee80211Mib::clearPeerHtCapabilities()
{
    checkStateMutation();
    stateChangePending |= !peerHtStates.empty();
    peerHtStates.clear();
}

std::string Ieee80211Mib::getSsidStr() const
{
    if (mode == INFRASTRUCTURE)
        return "\nSSID: " + bssData.ssid + ", " + bssData.bssid.str();
    return "";
}

const char *Ieee80211Mib::getModeStr(Ieee80211Mib::Mode mode)
{
    switch (mode) {
        case INFRASTRUCTURE: return "Infrastructure";
        case INDEPENDENT: return "Ad-hoc";
        case MESH: return "Mesh";
        default: return "?";
    }
}

const char *Ieee80211Mib::getStationTypeStr(Ieee80211Mib::BssStationType stationType)
{
    switch (stationType) {
        case ACCESS_POINT: return ", AP";
        case STATION: return ", STA";
        default: return "";
    }
}

short Ieee80211Mib::reserveAssociationId(const MacAddress& address)
{
    checkStateMutation();
    // IEEE Std 802.11-2024, 9.4.1.8: an AP assigns AID values in the range 1 through 2007.
    auto committed = bssAccessPointData.associationIds.find(address);
    if (committed != bssAccessPointData.associationIds.end())
        return committed->second;
    auto reserved = associationIdReservations.find(address);
    if (reserved != associationIdReservations.end())
        return reserved->second;

    std::array<bool, 2008> used = {};
    for (const auto& entry : bssAccessPointData.associationIds)
        if (entry.second >= 1 && entry.second <= 2007)
            used[entry.second] = true;
    for (const auto& entry : associationIdReservations)
        if (entry.second >= 1 && entry.second <= 2007)
            used[entry.second] = true;
    for (short aid = 1; aid <= 2007; aid++) {
        if (!used[aid]) {
            associationIdReservations[address] = aid;
            return aid;
        }
    }
    throw cRuntimeError("No IEEE 802.11 association ID is available");
}

short Ieee80211Mib::commitAssociationId(const MacAddress& address)
{
    checkStateMutation();
    auto committed = bssAccessPointData.associationIds.find(address);
    if (committed != bssAccessPointData.associationIds.end()) {
        associationIdReservations.erase(address);
        return committed->second;
    }
    auto reserved = associationIdReservations.find(address);
    if (reserved == associationIdReservations.end())
        throw cRuntimeError("No IEEE 802.11 association ID is reserved for %s", address.str().c_str());
    short aid = reserved->second;
    for (const auto& entry : bssAccessPointData.associationIds)
        if (entry.second == aid)
            throw cRuntimeError("Reserved IEEE 802.11 association ID %d is already committed", aid);
    bssAccessPointData.associationIds[address] = aid;
    stateChangePending = true;
    associationIdReservations.erase(reserved);
    return aid;
}

void Ieee80211Mib::cancelAssociationIdReservation(const MacAddress& address)
{
    checkStateMutation();
    associationIdReservations.erase(address);
}

short Ieee80211Mib::allocateAssociationId(const MacAddress& address)
{
    reserveAssociationId(address);
    return commitAssociationId(address);
}

void Ieee80211Mib::releaseAssociationId(const MacAddress& address)
{
    checkStateMutation();
    associationIdReservations.erase(address);
    stateChangePending |= bssAccessPointData.associationIds.erase(address) != 0;
    removePeerHtCapabilities(address);
}

void Ieee80211Mib::clearAssociationIds()
{
    checkStateMutation();
    stateChangePending |= !bssAccessPointData.stations.empty() || !bssAccessPointData.associationIds.empty();
    bssAccessPointData.stations.clear();
    associationIdReservations.clear();
    bssAccessPointData.associationIds.clear();
    clearPeerHtCapabilities();
}

} // namespace ieee80211

} // namespace inet

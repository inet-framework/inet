//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mib/Ieee80211Mib.h"

#include <algorithm>
#include <limits>

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Band.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"

namespace inet {

namespace ieee80211 {

Define_Module(Ieee80211Mib);

simsignal_t Ieee80211Mib::rateStateChangedSignal = cComponent::registerSignal("rateStateChanged");

void Ieee80211Mib::rateStateChanged()
{
    if (rateGeneration == std::numeric_limits<uint64_t>::max())
        throw cRuntimeError("Rate context generation exhausted");
    ++rateGeneration;
    rateUpdatePending = true;
    if (rateUpdateDepth == 0)
        flushRateStateChanged();
}

void Ieee80211Mib::flushRateStateChanged()
{
    Enter_Method("flushRateStateChanged");
    if (rateUpdatePending) {
        rateUpdatePending = false;
        emit(rateStateChangedSignal, true);
    }
}

void Ieee80211Mib::setPeerRateSet(const MacAddress& peer, const Ieee80211RateSetState& state)
{
    Enter_Method("setPeerRateSet");
    if (peer.isUnspecified() || peer.isMulticast())
        throw cRuntimeError("Peer rate state requires a unicast address");
    validateIeee80211RateSetState(state, "peer rate state");
    auto it = peerRateSets.find(peer);
    if (it != peerRateSets.end() && it->second == state)
        return;
    peerRateSets[peer] = state;
    rateStateChanged();
}

void Ieee80211Mib::installTargetRateContext(const BssRateContextRef& ref,
        const Ieee80211RateSetState& bssRates, const MacAddress& peer, const Ieee80211RateSetState& peerRates)
{
    Enter_Method("installTargetRateContext");
    validateIeee80211RateSetState(bssRates, "target BSS rate state");
    validateIeee80211RateSetState(peerRates, "target peer rate state");
    if (ref.kind != BssRateContextRef::TARGET || ref.transactionId == 0 || ref.generation != 0 ||
            ref.bssid.isUnspecified() || ref.bssid.isMulticast() || peer.isUnspecified() || peer.isMulticast())
        throw cRuntimeError("Invalid new target rate reference");
    if (targetRateContexts.count(ref.transactionId))
        throw cRuntimeError("Target rate transaction already exists");
    auto committedRef = ref;
    committedRef.generation = rateGeneration + 1;
    targetRateContexts.emplace(ref.transactionId, TargetRateContext{committedRef, peer, bssRates, peerRates});
    rateStateChanged();
}

void Ieee80211Mib::bindIncomingRateContext(const MacAddress& peer, int requestSubtype, const BssRateContextRef& ref)
{
    Enter_Method("bindIncomingRateContext");
    auto it = targetRateContexts.find(ref.transactionId);
    if (it == targetRateContexts.end() || it->second.peer != peer || ref.kind != BssRateContextRef::TARGET ||
            it->second.ref.bssid != ref.bssid || (ref.generation != 0 && it->second.ref.generation != ref.generation))
        throw cRuntimeError("Incoming rate binding has no matching target");
    auto& refs = incomingRateContexts[{peer, requestSubtype}];
    if (std::find(refs.begin(), refs.end(), it->second.ref) != refs.end())
        return;
    refs.push_back(it->second.ref);
    rateStateChanged();
}

void Ieee80211Mib::removeTargetRateContext(const BssRateContextRef& ref)
{
    Enter_Method("removeTargetRateContext");
    auto it = targetRateContexts.find(ref.transactionId);
    if (it == targetRateContexts.end() || it->second.ref != ref)
        return;
    targetRateContexts.erase(it);
    for (auto binding = incomingRateContexts.begin(); binding != incomingRateContexts.end(); ) {
        auto& refs = binding->second;
        refs.erase(std::remove(refs.begin(), refs.end(), ref), refs.end());
        if (refs.empty())
            binding = incomingRateContexts.erase(binding);
        else
            ++binding;
    }
    rateStateChanged();
}

void Ieee80211Mib::clearTargetRateContexts()
{
    Enter_Method("clearTargetRateContexts");
    if (targetRateContexts.empty())
        return;
    targetRateContexts.clear();
    incomingRateContexts.clear();
    rateStateChanged();
}

RateContextSnapshot Ieee80211Mib::snapshotRateContext(const MacAddress& peer, int frameSubtype,
        const std::optional<MacAddress>& bssid, const std::optional<BssRateContextRef>& explicitContext) const
{
    RateContextSnapshot result;
    result.localAddress = address;
    result.peerAddress = peer;
    result.localRates = localRateSet;
    result.generation = rateGeneration;
    int pairedSubtype = frameSubtype;
    if (frameSubtype == ST_ASSOCIATIONREQUEST)
        pairedSubtype = ST_ASSOCIATIONRESPONSE;
    else if (frameSubtype == ST_ASSOCIATIONRESPONSE)
        pairedSubtype = ST_ASSOCIATIONREQUEST;
    else if (frameSubtype == ST_REASSOCIATIONREQUEST)
        pairedSubtype = ST_REASSOCIATIONRESPONSE;
    else if (frameSubtype == ST_REASSOCIATIONRESPONSE)
        pairedSubtype = ST_REASSOCIATIONREQUEST;
    // Both directions belong to the same management exchange.
    std::vector<BssRateContextRef> bindings;
    for (int subtype : {frameSubtype, pairedSubtype}) {
        auto binding = incomingRateContexts.find({peer, subtype});
        if (binding != incomingRateContexts.end())
            for (const auto& ref : binding->second)
                if (std::find(bindings.begin(), bindings.end(), ref) == bindings.end())
                    bindings.push_back(ref);
    }
    auto useTarget = [&](const BssRateContextRef& ref) {
        auto it = targetRateContexts.find(ref.transactionId);
        if (it == targetRateContexts.end() || it->second.ref != ref || it->second.peer != peer ||
                (bssid && *bssid != ref.bssid))
            return;
        bool applicable = false;
        for (const auto& entry : incomingRateContexts) {
            if (entry.first.first == peer && (frameSubtype == ST_RTS ||
                    entry.first.second == frameSubtype || entry.first.second == pairedSubtype))
                applicable |= std::find(entry.second.begin(), entry.second.end(), ref) != entry.second.end();
        }
        if (!applicable)
            return;
        result.known = true;
        result.context = ref;
        result.bssRates = it->second.bssRates;
        result.peerRates = it->second.peerRates;
    };
    if (!explicitContext && frameSubtype == ST_RTS) {
        std::vector<BssRateContextRef> refs;
        for (const auto& entry : incomingRateContexts)
            if (entry.first.first == peer)
                for (const auto& ref : entry.second)
                    if ((!bssid || *bssid == ref.bssid) && std::find(refs.begin(), refs.end(), ref) == refs.end())
                        refs.push_back(ref);
        if (!refs.empty()) {
            if (refs.size() == 1)
                useTarget(refs.front());
            return result;
        }
    }
    if (explicitContext) {
        const auto& ref = *explicitContext;
        if (bssid && *bssid != ref.bssid)
            return result;
        if (ref.kind == BssRateContextRef::NONE) {
            // Active wildcard probing has no selected BSS.
            result.known = frameSubtype == ST_PROBEREQUEST && peer.isBroadcast() && !bssid && ref == BssRateContextRef();
            return result;
        }
        if (!bindings.empty() && (bindings.size() != 1 || bindings.front() != ref))
            return result;
        if (ref.kind == BssRateContextRef::TARGET) {
            useTarget(ref);
            return result;
        }
        if (ref != activeRateContext)
            return result;
    }
    else if (!bindings.empty()) {
        if (bindings.size() == 1)
            useTarget(bindings.front());
        return result;
    }
    // A tuned channel or a discovered AP is not an active relationship.
    bool activePeer = mode == INDEPENDENT ||
            bssStationData.stationType == ACCESS_POINT ||
            (bssStationData.isAssociated && peer == bssData.bssid);
    if (activeRateContext.kind != BssRateContextRef::ACTIVE || !activePeer ||
            (bssid && *bssid != activeRateContext.bssid))
        return result;
    result.known = true;
    result.context = activeRateContext;
    result.bssRates = bssRateSet;
    auto peerIt = peerRateSets.find(peer);
    if (peerIt != peerRateSets.end())
        result.peerRates = peerIt->second;
    return result;
}

void Ieee80211Mib::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        configuredSecondaryChannelOffset = par("htSecondaryChannelOffset");
        WATCH(address);
        WATCH(mode);
        WATCH(qos);
        WATCH(localHtCapabilitiesValid);
        WATCH(configuredSecondaryChannelOffset);
        WATCH(primaryChannelAvailable);
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

int Ieee80211Mib::requirePrimaryChannel() const
{
    if (!primaryChannelAvailable)
        throw cRuntimeError("IEEE 802.11 primary channel is unavailable");
    return htOperation.primaryChannel;
}

void Ieee80211Mib::setPrimaryChannel(int primaryChannel)
{
    setPrimaryChannel(primaryChannel, nullptr);
}

void Ieee80211Mib::setPrimaryChannel(int primaryChannel, const physicallayer::IIeee80211Band *band)
{
    Enter_Method("setPrimaryChannel");
    const auto previousOperation = htOperation;
    bool wasPrimaryChannelAvailable = primaryChannelAvailable;
    if (primaryChannel < 0 || primaryChannel > 255)
        throw cRuntimeError("IEEE 802.11 primary channel must be in the range 0..255, not %d", primaryChannel);

    if (band != nullptr) {
        try {
            band->getStandardChannelNumber(primaryChannel);
        }
        catch (const cRuntimeError&) {
            throw cRuntimeError("Invalid primary channel %d for band '%s'", primaryChannel, band->getName());
        }

        if (localHtCapabilitiesValid) {
            if (configuredSecondaryChannelOffset != 0) {
                if (band->isHt40OperationSupported(primaryChannel, configuredSecondaryChannelOffset)) {
                    htOperation.secondaryChannelOffset = configuredSecondaryChannelOffset;
                    htOperation.operatingChannelWidth = MHz(40);
                }
                else {
                    // IEEE Std 802.11-2024, 11.15.2 and 11.15.3.1: fallback to 20 MHz BSS operation
                    EV_WARN << "Configured 40 MHz HT operation (offset " << configuredSecondaryChannelOffset
                            << ") is unsupported on primary channel " << primaryChannel
                            << " in band '" << band->getName() << "'; falling back to 20 MHz BSS operation.\n";
                    htOperation.secondaryChannelOffset = 0;
                    htOperation.operatingChannelWidth = MHz(20);
                }
            }
            else {
                htOperation.secondaryChannelOffset = 0;
                htOperation.operatingChannelWidth = MHz(20);
            }
        }
    }

    htOperation.primaryChannel = primaryChannel;
    primaryChannelAvailable = true;

    bool peerStateChanged = false;
    if (localHtCapabilitiesValid) {
        for (auto& entry : peerHtStates) {
            if (entry.second.valid) {
                peerStateChanged |= !(entry.second.negotiatedCapabilities.operation == htOperation);
                entry.second.negotiatedCapabilities = negotiateHtCapabilities(localHtCapabilities,
                        entry.second.advertisedCapabilities, htOperation);
                if (++entry.second.generation == 0)
                    entry.second.generation = 1;
            }
        }
    }
    if (!wasPrimaryChannelAvailable || !(previousOperation == htOperation) || peerStateChanged)
        rateStateChanged();
}

const Ieee80211HtOperation& Ieee80211Mib::getHtOperation() const
{
    requirePrimaryChannel();
    return htOperation;
}

void Ieee80211Mib::updateLocalHtCapabilities(const physicallayer::Ieee80211ModeSet *modeSet,
        const std::set<Hz>& operationalChannelWidths, int operationalHtSpatialStreamLimit)
{
    Enter_Method("updateLocalHtCapabilities");
    bool wasLocalHtCapabilitiesValid = localHtCapabilitiesValid;
    const auto previousCapabilities = localHtCapabilities;
    const auto previousOperation = htOperation;
    // The radio publishes its initial channel at PHYSICAL_LAYER before the MAC
    // publishes its mode set at LINK_LAYER. Preserve that independent BSS
    // operation input when rebuilding the mode-derived capability subset.
    bool wasPrimaryChannelAvailable = primaryChannelAvailable;
    int primaryChannel = htOperation.primaryChannel;
    localHtCapabilities = Ieee80211HtCapabilities();
    htOperation = Ieee80211HtOperation();
    htOperation.primaryChannel = primaryChannel;
    primaryChannelAvailable = wasPrimaryChannelAvailable;
    localHtCapabilitiesValid = modeSet != nullptr && modeSet->isHtOperationSupported();
    if (!localHtCapabilitiesValid) {
        bool hadPeers = !peerHtStates.empty();
        peerHtStates.clear();
        if (wasLocalHtCapabilitiesValid || hadPeers || !(previousCapabilities == localHtCapabilities) ||
                !(previousOperation == htOperation))
            rateStateChanged();
        return;
    }
    if (operationalHtSpatialStreamLimit <= 0)
        throw cRuntimeError("HT operation requires a positive operational spatial-stream limit");

    // IEEE Std 802.11-2024, 9.4.2.54.4 and 9.4.2.55: advertise exactly the
    // HT modes come from the authoritative mode set, while advertised channel
    // widths are restricted to those the configured transmitter and receiver
    // can actually operate. In particular, do not infer dense MCS blocks or HT
    // widths from legacy/VHT modes that happen to share the set.
    const auto& mandatoryMcs = modeSet->getHtMcsMandatory();
    for (auto channelWidth : modeSet->getHtSupportedChannelWidths())
        if (operationalChannelWidths.count(channelWidth) != 0)
            localHtCapabilities.supportedChannelWidths.insert(channelWidth);
    localHtCapabilities.shortGi20 = localHtCapabilities.supportedChannelWidths.count(MHz(20)) != 0 &&
            modeSet->isHtShortGuardIntervalSupported(MHz(20));
    localHtCapabilities.shortGi40 = localHtCapabilities.supportedChannelWidths.count(MHz(40)) != 0 &&
            modeSet->isHtShortGuardIntervalSupported(MHz(40));
    for (int index = 0; index < modeSet->getNumModes(); index++) {
        const auto *mode = modeSet->getMode(index);
        int mcs = mode->getHtMcsIndex();
        if (mcs >= 0 && mcs < 77 && operationalChannelWidths.count(mode->getDataMode()->getBandwidth()) != 0 &&
                mode->getDataMode()->getNumberOfSpatialStreams() <= operationalHtSpatialStreamLimit)
            localHtCapabilities.rxMcsSupported[mcs] = true;
    }
    for (int mcs = 0; mcs < 77; mcs++)
        htOperation.basicMcsSupported[mcs] = mandatoryMcs[mcs] && localHtCapabilities.rxMcsSupported[mcs];
    // The equal-case Tx MCS set is represented by the maximum MCS index per
    // spatial-stream group. Rebuild it from the filtered Rx bitmap; MCS 32 is
    // not part of this map's MCS 0..31 NSS encoding.
    localHtCapabilities.txMcsNss = Ieee80211HtMcsNssMap();
    for (int mcs = 0; mcs < 32; mcs++) {
        if (localHtCapabilities.rxMcsSupported[mcs]) {
            int nss = mcs / 8;
            localHtCapabilities.txMcsNss.maxMcsPerNss[nss] = std::max(localHtCapabilities.txMcsNss.maxMcsPerNss[nss], mcs % 8);
        }
    }
    if (localHtCapabilities.supportedChannelWidths.empty())
        throw cRuntimeError("HT operation mode set '%s' does not provide an HT channel width", modeSet->getName());
    localHtCapabilities.maxAmpduLengthExponent = par("htMaxAmpduLengthExponent");
    if (localHtCapabilities.maxAmpduLengthExponent < 0 || localHtCapabilities.maxAmpduLengthExponent > 3)
        throw cRuntimeError("htMaxAmpduLengthExponent must be between 0 and 3");

    configuredSecondaryChannelOffset = par("htSecondaryChannelOffset");
    if (configuredSecondaryChannelOffset != 0 && configuredSecondaryChannelOffset != 1 && configuredSecondaryChannelOffset != 3)
        throw cRuntimeError("htSecondaryChannelOffset must be 0, 1, or 3");
    htOperation.secondaryChannelOffset = configuredSecondaryChannelOffset;
    bool use40Mhz = htOperation.secondaryChannelOffset != 0;
    if (use40Mhz && localHtCapabilities.supportedChannelWidths.count(MHz(40)) == 0)
        throw cRuntimeError("40 MHz HT operation requires a configured PHY that can operate a 40 MHz channel width");
    htOperation.operatingChannelWidth = use40Mhz ? MHz(40) : MHz(20);
    int protectionMode = par("htProtectionMode");
    if (protectionMode < 0 || protectionMode > 3)
        throw cRuntimeError("htProtectionMode must be between 0 and 3");
    htOperation.protectionMode = static_cast<Ieee80211HtProtectionMode>(protectionMode);
    bool peerStateChanged = false;
    for (auto& entry : peerHtStates) {
        if (entry.second.valid) {
            peerStateChanged |= !(entry.second.negotiatedCapabilities.localAdvertisement == localHtCapabilities) ||
                    !(entry.second.negotiatedCapabilities.operation == htOperation);
            entry.second.negotiatedCapabilities = negotiateHtCapabilities(localHtCapabilities,
                    entry.second.advertisedCapabilities, htOperation);
            if (++entry.second.generation == 0)
                entry.second.generation = 1;
        }
    }
    if (!wasLocalHtCapabilitiesValid || !(previousCapabilities == localHtCapabilities) ||
            !(previousOperation == htOperation) || peerStateChanged)
        rateStateChanged();
}

const Ieee80211Mib::PeerHtState *Ieee80211Mib::findPeerHtState(const MacAddress& address) const
{
    auto it = peerHtStates.find(address);
    return it == peerHtStates.end() || !it->second.valid ? nullptr : &it->second;
}

const Ieee80211RateSetState *Ieee80211Mib::findPeerRateSet(const MacAddress& address) const
{
    auto it = peerRateSets.find(address);
    return it == peerRateSets.end() ? nullptr : &it->second;
}

void Ieee80211Mib::setLocalRateSet(const Ieee80211RateSetState& rateSet)
{
    Enter_Method("setLocalRateSet");
    validateIeee80211RateSetState(rateSet, "local rate state");
    if (localRateSet == rateSet)
        return;
    localRateSet = rateSet;
    rateStateChanged();
}

void Ieee80211Mib::setBssRateSet(const Ieee80211RateSetState& rateSet)
{
    Enter_Method("setBssRateSet");
    validateIeee80211RateSetState(rateSet, "BSS rate state");
    if (bssRateSet == rateSet && activeRateContext.kind == BssRateContextRef::ACTIVE && activeRateContext.bssid == bssData.bssid)
        return;
    bssRateSet = rateSet;
    activeRateContext = {BssRateContextRef::ACTIVE, bssData.bssid, 0, rateGeneration + 1};
    rateStateChanged();
}

void Ieee80211Mib::installBssAndPeerRateSets(const Ieee80211RateSetState& newBssRateSet,
        const MacAddress& peerAddress, const Ieee80211RateSetState& newPeerRateSet)
{
    Enter_Method("installBssAndPeerRateSets");
    validateIeee80211RateSetState(newBssRateSet, "BSS rate state");
    validateIeee80211RateSetState(newPeerRateSet, "peer rate state");
    bool bssChanged = bssRateSet != newBssRateSet || activeRateContext.kind != BssRateContextRef::ACTIVE ||
            activeRateContext.bssid != bssData.bssid;
    auto peerIt = peerRateSets.find(peerAddress);
    bool peerChanged = peerIt == peerRateSets.end() || peerIt->second != newPeerRateSet;
    if (!bssChanged && !peerChanged)
        return;
    bssRateSet = newBssRateSet;
    if (bssChanged)
        activeRateContext = {BssRateContextRef::ACTIVE, bssData.bssid, 0, rateGeneration + 1};
    peerRateSets[peerAddress] = newPeerRateSet;
    rateStateChanged();
}

void Ieee80211Mib::clearBssRateSet()
{
    Enter_Method("clearBssRateSet");
    if (activeRateContext.kind != BssRateContextRef::ACTIVE && !bssRateSet.supported.known && !bssRateSet.basic.known && !bssRateSet.operational.known)
        return;
    bssRateSet = Ieee80211RateSetState();
    activeRateContext = BssRateContextRef();
    rateStateChanged();
}

void Ieee80211Mib::removePeerRateSet(const MacAddress& address)
{
    Enter_Method("removePeerRateSet");
    if (peerRateSets.erase(address) == 0)
        return;
    rateStateChanged();
}

void Ieee80211Mib::clearPeerRateSets()
{
    Enter_Method("clearPeerRateSets");
    if (peerRateSets.empty())
        return;
    peerRateSets.clear();
    rateStateChanged();
}

void Ieee80211Mib::setPeerHtCapabilities(const MacAddress& address, const Ieee80211HtCapabilities& capabilities,
        const Ieee80211HtOperation& operation)
{
    Enter_Method("setPeerHtCapabilities");
    if (!localHtCapabilitiesValid)
        throw cRuntimeError("Cannot install peer HT capabilities when local HT operation is disabled");
    auto& state = peerHtStates[address];
    bool changed = !state.valid || !(state.advertisedCapabilities == capabilities) ||
            !(state.negotiatedCapabilities.localAdvertisement == localHtCapabilities) ||
            !(state.negotiatedCapabilities.operation == operation);
    state.valid = true;
    state.advertisedCapabilities = capabilities;
    state.negotiatedCapabilities = negotiateHtCapabilities(localHtCapabilities, capabilities, operation);
    if (++state.generation == 0)
        state.generation = 1;
    EV_INFO << "Installed peer HT state, peer = " << address
            << ", txValid = " << state.negotiatedCapabilities.localTxPeerRx.valid
            << ", rxValid = " << state.negotiatedCapabilities.localRxPeerTx.valid << endl;
    if (changed)
        rateStateChanged();
}

void Ieee80211Mib::removePeerHtCapabilities(const MacAddress& address)
{
    Enter_Method("removePeerHtCapabilities");
    if (peerHtStates.erase(address) != 0)
        rateStateChanged();
}

void Ieee80211Mib::clearPeerHtCapabilities()
{
    Enter_Method("clearPeerHtCapabilities");
    if (!peerHtStates.empty()) {
        peerHtStates.clear();
        rateStateChanged();
    }
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
    associationIdReservations.erase(reserved);
    return aid;
}

void Ieee80211Mib::cancelAssociationIdReservation(const MacAddress& address)
{
    associationIdReservations.erase(address);
}

short Ieee80211Mib::allocateAssociationId(const MacAddress& address)
{
    reserveAssociationId(address);
    return commitAssociationId(address);
}

void Ieee80211Mib::releaseAssociationId(const MacAddress& address)
{
    Enter_Method("releaseAssociationId");
    associationIdReservations.erase(address);
    bssAccessPointData.associationIds.erase(address);
    bool changed = peerHtStates.erase(address) != 0;
    changed |= peerRateSets.erase(address) != 0;
    if (changed)
        rateStateChanged();
}

void Ieee80211Mib::clearAssociationIds()
{
    Enter_Method("clearAssociationIds");
    bssAccessPointData.stations.clear();
    associationIdReservations.clear();
    bssAccessPointData.associationIds.clear();
    bool changed = !peerHtStates.empty() || !peerRateSets.empty();
    peerHtStates.clear();
    peerRateSets.clear();
    if (changed)
        rateStateChanged();
}

} // namespace ieee80211

} // namespace inet

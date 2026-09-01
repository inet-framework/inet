//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mib/Ieee80211Mib.h"

#include <algorithm>

#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"

namespace inet {

namespace ieee80211 {

Define_Module(Ieee80211Mib);

void Ieee80211Mib::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        WATCH(address);
        WATCH(mode);
        WATCH(qos);
        WATCH(localHtCapabilitiesValid);
        WATCH(bssData.bssid);
        WATCH(bssStationData.stationType);
        WATCH(bssStationData.isAssociated);
        WATCH(bssAccessPointData.stations);
        WATCH(bssAccessPointData.associationIds);
        WATCH_EXPR("modeStr", getModeStr(mode));
        WATCH_EXPR("stationTypeStr", getStationTypeStr(bssStationData.stationType));
        WATCH_EXPR("qosStr", qos ? ", QoS" : ", Non-QoS");
        WATCH_EXPR("ssidStr", getSsidStr());
        WATCH_EXPR("ssid", bssData.ssid.empty() ? std::string("-") : bssData.ssid); // associated SSID ("-" if none), for node display strings
        WATCH_EXPR("associatedStr", bssStationData.stationType == STATION ? (bssStationData.isAssociated ? "\nAssociated" : "\nNot associated") : "");
    }
}

void Ieee80211Mib::updateLocalHtCapabilities(const physicallayer::Ieee80211ModeSet *modeSet)
{
    // The radio publishes its initial channel at PHYSICAL_LAYER before the MAC
    // publishes its mode set at LINK_LAYER. Preserve that independent BSS
    // operation input when rebuilding the mode-derived capability subset.
    int primaryChannel = htOperation.primaryChannel;
    localHtCapabilities = Ieee80211HtCapabilities();
    htOperation = Ieee80211HtOperation();
    htOperation.primaryChannel = primaryChannel;
    localHtCapabilitiesValid = modeSet != nullptr && modeSet->isHtOperationSupported();
    if (!localHtCapabilitiesValid) {
        clearPeerHtCapabilities();
        return;
    }

    // IEEE Std 802.11-2024, 9.4.2.54.4 and 9.4.2.55: advertise exactly the
    // HT modes and HT-only channel widths present in the authoritative mode
    // set. In particular, do not infer dense MCS blocks or HT widths from
    // legacy/VHT modes that happen to share the set.
    const auto& supportedMcs = modeSet->getHtMcsSupported();
    const auto& mandatoryMcs = modeSet->getHtMcsMandatory();
    localHtCapabilities.supportedChannelWidths = modeSet->getHtSupportedChannelWidths();
    localHtCapabilities.shortGi20 = modeSet->isHtShortGuardIntervalSupported(MHz(20));
    localHtCapabilities.shortGi40 = modeSet->isHtShortGuardIntervalSupported(MHz(40));
    for (int mcs = 0; mcs < 77; mcs++) {
        localHtCapabilities.rxMcsSupported[mcs] = supportedMcs[mcs];
        if (mcs < 32 && supportedMcs[mcs]) {
            int nss = mcs / 8;
            localHtCapabilities.txMcsNss.maxMcsPerNss[nss] = std::max(localHtCapabilities.txMcsNss.maxMcsPerNss[nss], mcs % 8);
        }
        htOperation.basicMcsSupported[mcs] = mandatoryMcs[mcs];
    }
    if (localHtCapabilities.supportedChannelWidths.empty())
        throw cRuntimeError("HT operation mode set '%s' does not provide an HT channel width", modeSet->getName());
    localHtCapabilities.maxAmpduLengthExponent = par("htMaxAmpduLengthExponent");
    if (localHtCapabilities.maxAmpduLengthExponent < 0 || localHtCapabilities.maxAmpduLengthExponent > 3)
        throw cRuntimeError("htMaxAmpduLengthExponent must be between 0 and 3");

    htOperation.secondaryChannelOffset = par("htSecondaryChannelOffset");
    if (htOperation.secondaryChannelOffset != 0 && htOperation.secondaryChannelOffset != 1 && htOperation.secondaryChannelOffset != 3)
        throw cRuntimeError("htSecondaryChannelOffset must be 0, 1, or 3");
    bool use40Mhz = htOperation.secondaryChannelOffset != 0;
    if (use40Mhz && localHtCapabilities.supportedChannelWidths.count(MHz(40)) == 0)
        throw cRuntimeError("40 MHz HT operation requires a 40 MHz-capable mode set");
    htOperation.operatingChannelWidth = use40Mhz ? MHz(40) : MHz(20);
    int protectionMode = par("htProtectionMode");
    if (protectionMode < 0 || protectionMode > 3)
        throw cRuntimeError("htProtectionMode must be between 0 and 3");
    htOperation.protectionMode = static_cast<Ieee80211HtProtectionMode>(protectionMode);
    for (auto& entry : peerHtStates)
        if (entry.second.valid)
            entry.second.negotiatedCapabilities = negotiateHtCapabilities(localHtCapabilities,
                    entry.second.advertisedCapabilities, entry.second.negotiatedCapabilities.operation);
}

const Ieee80211Mib::PeerHtState *Ieee80211Mib::findPeerHtState(const MacAddress& address) const
{
    auto it = peerHtStates.find(address);
    return it == peerHtStates.end() || !it->second.valid ? nullptr : &it->second;
}

void Ieee80211Mib::setPeerHtCapabilities(const MacAddress& address, const Ieee80211HtCapabilities& capabilities,
        const Ieee80211HtOperation& operation)
{
    if (!localHtCapabilitiesValid)
        throw cRuntimeError("Cannot install peer HT capabilities when local HT operation is disabled");
    auto& state = peerHtStates[address];
    state.valid = true;
    state.advertisedCapabilities = capabilities;
    state.negotiatedCapabilities = negotiateHtCapabilities(localHtCapabilities, capabilities, operation);
    if (++state.generation == 0)
        state.generation = 1;
    EV_INFO << "Installed peer HT state, peer = " << address
            << ", txValid = " << state.negotiatedCapabilities.localTxPeerRx.valid
            << ", rxValid = " << state.negotiatedCapabilities.localRxPeerTx.valid << endl;
}

void Ieee80211Mib::removePeerHtCapabilities(const MacAddress& address)
{
    peerHtStates.erase(address);
}

void Ieee80211Mib::clearPeerHtCapabilities()
{
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

short Ieee80211Mib::allocateAssociationId(const MacAddress& address)
{
    auto existing = bssAccessPointData.associationIds.find(address);
    if (existing != bssAccessPointData.associationIds.end())
        return existing->second;
    for (short aid = 1; aid <= 2007; aid++) {
        bool used = false;
        for (const auto& entry : bssAccessPointData.associationIds)
            if (entry.second == aid) {
                used = true;
                break;
            }
        if (!used) {
            bssAccessPointData.associationIds[address] = aid;
            return aid;
        }
    }
    throw cRuntimeError("No IEEE 802.11 association ID is available");
}

void Ieee80211Mib::releaseAssociationId(const MacAddress& address)
{
    bssAccessPointData.associationIds.erase(address);
    removePeerHtCapabilities(address);
}

} // namespace ieee80211

} // namespace inet

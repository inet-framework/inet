//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mib/Ieee80211Mib.h"

#include <algorithm>
#include <sstream>

#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Band.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"

namespace inet {

namespace ieee80211 {

Define_Module(Ieee80211Mib);

simsignal_t Ieee80211Mib::rateStateChangedSignal = cComponent::registerSignal("rateStateChanged");
simsignal_t Ieee80211Mib::meshPeerChangedSignal = cComponent::registerSignal("meshPeerChanged");
simsignal_t Ieee80211Mib::meshPowerModeChangedSignal = cComponent::registerSignal("meshPowerModeChanged");

const MeshPeerPowerState *Ieee80211Mib::findMeshPeerPowerState(MacAddress peer) const
{
    auto it = meshPeerPower.find(peer);
    return it == meshPeerPower.end() ? nullptr : &it->second;
}

MeshPowerMode Ieee80211Mib::getMeshNonpeerPowerMode() const
{
    // IEEE Std 802.11-2024, 14.15.3.1: even local light sleep makes
    // nonpeer operation deep sleep; the group PSL has a different predicate.
    for (const auto& entry : meshPeerPower)
        if (entry.second.local != MeshPowerMode::ACTIVE)
            return MeshPowerMode::DEEP_SLEEP;
    return MeshPowerMode::ACTIVE;
}

bool Ieee80211Mib::getMeshGroupPowerSaveLevel() const
{
    for (const auto& entry : meshPeerPower)
        if (entry.second.local == MeshPowerMode::DEEP_SLEEP)
            return true;
    return false;
}

void Ieee80211Mib::commitMeshLocalPowerMode(MacAddress peer, MeshPowerMode mode)
{
    Enter_Method("commitMeshLocalPowerMode");
    auto it = meshPeerPower.find(peer);
    if (it == meshPeerPower.end() || it->second.local == mode)
        return;
    it->second.local = mode;
    meshConfiguration.capability = (meshConfiguration.capability & ~64) | (getMeshGroupPowerSaveLevel() ? 64 : 0);
    MeshPeerChange details(peer);
    emit(meshPowerModeChangedSignal, long(mode), &details);
}

void Ieee80211Mib::commitMeshRemotePowerMode(MacAddress peer, MeshPowerMode mode)
{
    Enter_Method("commitMeshRemotePowerMode");
    auto it = meshPeerPower.find(peer);
    if (it == meshPeerPower.end() || it->second.remote == mode)
        return;
    it->second.remote = mode;
    MeshPeerChange details(peer);
    emit(meshPowerModeChangedSignal, long(mode), &details);
}

void Ieee80211Mib::commitMeshLocalActiveForAllPeers()
{
    Enter_Method("commitMeshLocalActiveForAllPeers");
    std::vector<MacAddress> changed;
    for (auto& entry : meshPeerPower)
        if (entry.second.local != MeshPowerMode::ACTIVE) {
            entry.second.local = MeshPowerMode::ACTIVE;
            changed.push_back(entry.first);
        }
    meshConfiguration.capability &= ~64;
    // Every observer sees the complete batch. Nested commands may replace
    // individual state, so do not perform more batch writes after publication.
    for (auto peer : changed) {
        auto current = meshPeerPower.find(peer);
        if (current != meshPeerPower.end() && current->second.local == MeshPowerMode::ACTIVE) {
            MeshPeerChange details(peer);
            emit(meshPowerModeChangedSignal, long(MeshPowerMode::ACTIVE), &details);
        }
    }
}

void Ieee80211Mib::configureMeshProfile(const std::string& id, const MeshConfiguration& configuration)
{
    Enter_Method("configureMeshProfile");
    if (id.empty() || id.size() > 32 || configuration.pathSelectionProtocolId != 1
            || (configuration.pathSelectionMetricId != 1 && configuration.pathSelectionMetricId != 2)
            || configuration.congestionControlId != 0 || configuration.synchronizationId != 1
            || configuration.authenticationId != 0)
        throw cRuntimeError("Unsupported open mesh profile");
    meshId = id;
    meshConfiguration = configuration;
}

void Ieee80211Mib::setMeshForwarding(bool enabled)
{
    Enter_Method("setMeshForwarding");
    // IEEE Std 802.11-2024, 9.4.2.96.8: advertise the configured dot11MeshForwarding.
    meshConfiguration.capability = (meshConfiguration.capability & ~8) | (enabled ? 8 : 0);
}

bool Ieee80211Mib::isMeshPeerEstablished(MacAddress peer) const
{
    return mode == MESH && peerRateSets.count(peer) != 0;
}

void Ieee80211Mib::establishMeshPeer(MacAddress peer, const Ieee80211RateSetState& rates)
{
    establishMeshPeer(peer, rates, {});
}

void Ieee80211Mib::establishMeshPeer(MacAddress peer, const Ieee80211RateSetState& rates, const Ieee80211PeerPhyCapabilities& phy)
{
    Enter_Method("establishMeshPeer");
    validateIeee80211RateSetState(rates, "mesh peer rates");
    auto previous = peerRateSets.find(peer);
    auto previousPhy = meshPeerPhy.find(peer);
    if (previous != peerRateSets.end() && previous->second == rates
            && previousPhy != meshPeerPhy.end() && previousPhy->second == phy)
        return;
    peerRateSets[peer] = rates;
    meshPeerPower.try_emplace(peer);
    meshPeerPhy[peer] = phy;
    if (phy.htPresent)
        updatePeerHtCapabilities(peer, phy.ht, phy.htOperation);
    else
        peerHtStates.erase(peer);
    MeshPeerChange details(peer);
    emit(rateStateChangedSignal, true);
    emit(meshPeerChangedSignal, true, &details);
}

bool Ieee80211Mib::withdrawMeshPeer(MacAddress peer)
{
    Enter_Method("withdrawMeshPeer");
    meshPeerPhy.erase(peer);
    meshPeerPower.erase(peer);
    meshConfiguration.capability = (meshConfiguration.capability & ~64) | (getMeshGroupPowerSaveLevel() ? 64 : 0);
    peerHtStates.erase(peer);
    return peerRateSets.erase(peer) != 0;
}

const Ieee80211PeerPhyCapabilities *Ieee80211Mib::findMeshPeerPhyCapabilities(MacAddress peer) const
{
    auto it = meshPeerPhy.find(peer);
    return it == meshPeerPhy.end() ? nullptr : &it->second;
}

void Ieee80211Mib::configureMeshPhyCapabilities(const std::string& profile, const physicallayer::Ieee80211ModeSet *modes,
        const physicallayer::Ieee80211Channel *channel, int spatialStreams)
{
    Enter_Method("configureMeshPhyCapabilities");
    meshShortGuardIntervalTransmissionEnabled = par("meshShortGuardIntervalTransmissionEnabled");
    if (profile != "ofdm" && profile != "ht" && profile != "vht")
        throw cRuntimeError("mesh phyProfile must be ofdm, ht or vht");
    meshPhyProfile = profile;
    localMeshPhy = {};
    if (profile == "ofdm")
        return;
    if (!channel || !channel->hasAllocation() || !modes || !modes->isHtOperationSupported())
        throw cRuntimeError("HT/VHT mesh requires a configured contiguous5GHz allocation and capable catalog");
    int rx = par("meshRxSpatialStreams"), tx = par("meshTxSpatialStreams");
    rx = rx == -1 ? spatialStreams : rx;
    tx = tx == -1 ? spatialStreams : tx;
    if (rx < 1 || tx < 1 || rx > spatialStreams || tx > spatialStreams)
        throw cRuntimeError("Mesh stream ceilings exceed the antenna/catalog capability");
    int rxMcs = par("meshHtRxMaxMcs"), txMcs = par("meshHtTxMaxMcs");
    if (rxMcs < 0 || rxMcs > 7 || txMcs < 0 || txMcs > 7)
        throw cRuntimeError("HT per-stream MCS ceilings must be0..7");
    auto& ht = localMeshPhy.ht;
    ht = localHtCapabilities;
    // Device capability and the current operating allocation are separate.
    // In particular, a VHT device operating at20MHz still supports HT40 and
    // the mandatory long-GI VHT20/40/80 tuples (2024,21.1.1).
    ht.supportedChannelWidths = modes->getHtSupportedChannelWidths();
    for (int mcs = 0; mcs < 77; mcs++)
        ht.rxMcsSupported[mcs] = ht.rxMcsSupported[mcs] && mcs < rx * 8 && mcs % 8 <= rxMcs;
    ht.txMcsNss.maxMcsPerNss.fill(-1);
    for (int nss = 0; nss < std::min(tx, 4); nss++)
        ht.txMcsNss.maxMcsPerNss[nss] = txMcs;
    ht.txMcsSetDefined = true;
    ht.txRxMcsSetNotEqual = rx != tx || rxMcs != txMcs;
    ht.txMaxNss = ht.txRxMcsSetNotEqual ? std::min(tx, 4) : 0;
    ht.shortGi20 = modes->isHtShortGuardIntervalSupported(MHz(20)) && par("meshShortGuardInterval").boolValue();
    ht.shortGi40 = modes->isHtShortGuardIntervalSupported(MHz(40)) && par("meshShortGuardInterval").boolValue();
    ht.maxAmsduLength = par("meshMaxAmsduLength");
    ht.minimumMpduStartSpacing = par("meshMinimumMpduStartSpacing");
    if ((ht.maxAmsduLength != 3839 && ht.maxAmsduLength != 7935) || ht.minimumMpduStartSpacing < 0 || ht.minimumMpduStartSpacing > 7)
        throw cRuntimeError("Invalid mesh HT aggregation capability limits");
    auto& operation = localMeshPhy.htOperation;
    operation = htOperation;
    // HT operation stores a band index; the existing IE adapter maps it to
    // the IEEE channel number exactly once at the wire boundary.
    operation.primaryChannel = channel->getChannelNumber();
    operation.operatingChannelWidth = channel->getAllocationWidth() >= MHz(40) ? MHz(40) : MHz(20);
    operation.secondaryChannelOffset = operation.operatingChannelWidth == MHz(20) ? 0 :
        channel->getCenterChannel(MHz(40)) > channel->getPrimaryChannel() ? 1 : 3;
    operation.rifsMode = false;
    if (operation.protectionMode != Ieee80211HtProtectionMode::NO_PROTECTION)
        throw cRuntimeError("Protected HT/VHT mesh operation is outside the selected profile");
    for (int mcs = 0; mcs < 77; mcs++)
        if (operation.basicMcsSupported[mcs] && (!ht.rxMcsSupported[mcs] || ht.txMcsNss.maxMcsPerNss[mcs / 8] < mcs % 8))
            throw cRuntimeError("Local mesh PHY cannot support its HT basic MCS set");
    localMeshPhy.htPresent = true;
    localHtCapabilities = ht;
    htOperation = operation;
    if (profile == "vht") {
        auto& v = localMeshPhy.vht;
        v.width160 = par("meshVht160MhzCapable");
        if (channel->getAllocationWidth() == MHz(160) && !v.width160)
            throw cRuntimeError("Operating160MHz requires the optional VHT160MHz device capability");
        Hz deviceWidth = v.width160 ? Hz(MHz(160)) : Hz(MHz(80));
        for (auto width : {MHz(20),MHz(40),MHz(80),MHz(160)}) {
            if (width > deviceWidth) continue;
            for (int mcs=0;mcs<8;mcs++) {
                bool found = false;
                for (int i=0;i<modes->getNumModes();i++) {
                    auto mode = modes->getMode(i);
                    found |= mode->getVhtMcsIndex() == mcs && mode->getDataMode()->getNumberOfSpatialStreams() == 1
                        && mode->getDataMode()->getBandwidth() == width && !mode->isVhtShortGuardInterval();
                }
                if (!found) throw cRuntimeError("VHT device capability lacks mandatory long-GI NSS1/MCS0..7 at%gMHz",width.get());
            }
        }
        auto readMap = [&](const char *name, std::array<int, 8>& map, int streams) {
            auto configured = check_and_cast<cValueArray *>(par(name).objectValue());
            if (configured->size() != 0 && configured->size() != 8)
                throw cRuntimeError("%s must be empty or contain8 MCS ceilings", name);
            for (int i = 0; i < modes->getNumModes(); i++) {
                auto mode = modes->getMode(i);
                int mcs = mode->getVhtMcsIndex(), nss = mode->getDataMode()->getNumberOfSpatialStreams();
                if (mcs >= 0 && nss <= streams && mode->getDataMode()->getBandwidth() <= deviceWidth)
                    map[nss - 1] = std::max(map[nss - 1], mcs);
            }
            if (configured->size())
                for (int nss = 0; nss < 8; nss++) {
                    int ceiling = configured->get(nss).intValue();
                    if (ceiling != -1 && ceiling != 7 && ceiling != 8 && ceiling != 9)
                        throw cRuntimeError("VHT map ceilings must be-1,7,8,9");
                    if (ceiling > map[nss])
                        throw cRuntimeError("VHT configured ceiling exceeds actual catalog/antenna/width support");
                    map[nss] = ceiling;
                }
        };
        readMap("meshVhtRxMcsMap", v.rxMcs, rx);
        readMap("meshVhtTxMcsMap", v.txMcs, tx);
        if (v.rxMcs[0] < 7 || v.txMcs[0] < 7)
            throw cRuntimeError("VHT device MCS maps must support the mandatory first spatial stream");
        for (int i = 0; i < modes->getNumModes(); i++) {
            auto mode = modes->getMode(i);
            if (mode->getVhtMcsIndex() >= 0 && mode->isVhtShortGuardInterval() && par("meshShortGuardInterval").boolValue()) {
                v.shortGi80 |= mode->getDataMode()->getBandwidth() == MHz(80);
                v.shortGi160 |= v.width160 && mode->getDataMode()->getBandwidth() == MHz(160);
            }
        }
        v.maxMpduLength = par("meshVhtMaxMpduLength");
        v.maxAmpduLengthExponent = par("meshVhtMaxAmpduLengthExponent");
        if ((v.maxMpduLength != 3895 && v.maxMpduLength != 7991 && v.maxMpduLength != 11454)
                || v.maxAmpduLengthExponent < 0 || v.maxAmpduLengthExponent > 7)
            throw cRuntimeError("Invalid VHT receive size limits");
        auto& op = localMeshPhy.vhtOperation;
        op.width = channel->getAllocationWidth();
        op.center0 = channel->getCenterChannel(op.width == MHz(160) ? Hz(MHz(80)) : op.width);
        op.center1 = op.width == MHz(160) ? channel->getCenterChannel(MHz(160)) : 0;
        int basic = par("meshVhtBasicMcsMap");
        if (basic < 0 || basic > 65535 || !supportsBasicVhtMcs(v, basic))
            throw cRuntimeError("Local mesh PHY cannot support its VHT basic MCS/NSS set");
        op.basicMcsMap = basic;
        localMeshPhy.vhtPresent = true;
    }
    emit(rateStateChangedSignal, true);
}

void Ieee80211Mib::publishMeshPeerRemoval(MacAddress peer)
{
    Enter_Method("publishMeshPeerRemoval");
    ASSERT(!isMeshPeerEstablished(peer));
    MeshPeerChange details(peer);
    emit(rateStateChangedSignal, true);
    emit(meshPeerChangedSignal, false, &details);
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
        WATCH(meshId);
        WATCH_MAP(meshPeerPower);
        WATCH(meshPhyProfile);
        WATCH(meshShortGuardIntervalTransmissionEnabled);
        WATCH_EXPR("meshPhyCapabilities", describeMeshPhy());
        WATCH_EXPR("meshPeerCount", mode == MESH ? peerRateSets.size() : 0);
        WATCH_EXPR("meshMetricId", int(meshConfiguration.pathSelectionMetricId));
        WATCH_EXPR("modeStr", getModeStr(mode));
        WATCH_EXPR("stationTypeStr", getStationTypeStr(bssStationData.stationType));
        WATCH_EXPR("qosStr", qos ? ", QoS" : ", Non-QoS");
        WATCH_EXPR("ssidStr", getSsidStr());
        WATCH_EXPR("ssid", bssData.ssid.empty() ? std::string("-") : bssData.ssid); // associated SSID ("-" if none), for node display strings
        WATCH_EXPR("associatedStr", bssStationData.stationType == STATION ? (bssStationData.isAssociated ? "\nAssociated" : "\nNot associated") : "");
        WATCH_EXPR("primaryChannel", primaryChannelAvailable ? std::to_string(htOperation.primaryChannel) : "unavailable");
    }
}

std::string Ieee80211Mib::describeMeshPhy() const
{
    std::ostringstream stream;
    auto describe = [&](const Ieee80211PeerPhyCapabilities& phy) {
        stream << " HT=" << phy.htPresent << " VHT=" << phy.vhtPresent;
        if (phy.htPresent) {
            stream << " HT-RxMCS=";
            for (int mcs = 0; mcs < 77; mcs++) if (phy.ht.rxMcsSupported[mcs]) stream << mcs << ',';
            stream << " HT-TxCeilings=";
            for (int ceiling : phy.ht.txMcsNss.maxMcsPerNss) stream << ceiling << ',';
            stream << " HT-width=" << phy.htOperation.operatingChannelWidth << " A-MSDU=" << phy.ht.maxAmsduLength
                << " A-MPDU-exponent=" << phy.ht.maxAmpduLengthExponent << " spacing=" << phy.ht.minimumMpduStartSpacing;
        }
        if (phy.vhtPresent) {
            stream << " VHT-Rx=";
            for (int ceiling : phy.vht.rxMcs) stream << ceiling << ',';
            stream << " VHT-Tx=";
            for (int ceiling : phy.vht.txMcs) stream << ceiling << ',';
            stream << " width=" << phy.vhtOperation.width << " CCFS=" << int(phy.vhtOperation.center0) << ',' << int(phy.vhtOperation.center1)
                << " maxMPDU=" << phy.vht.maxMpduLength << " A-MPDU-exponent=" << phy.vht.maxAmpduLengthExponent;
        }
        stream << '\n';
    };
    stream << "local"; describe(localMeshPhy);
    for (const auto& [peer, phy] : meshPeerPhy) { stream << peer; describe(phy); }
    return stream.str();
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
        emit(rateStateChangedSignal, true);
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
            emit(rateStateChangedSignal, true);
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
    bool vhtSupported = false;
    for (int index = 0; index < modeSet->getNumModes(); index++) {
        const auto *mode = modeSet->getMode(index);
        vhtSupported |= mode->getModulationClass() == physicallayer::IIeee80211Mode::ModulationClass::VHT;
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
    // IEEE Std 802.11-2024, 10.27.3.3: the protected modes permit the conservative zero policy.
    // IEEE Std 802.11-2024, 11.38.1: a VHT AP advertises zero.
    htOperation.rifsMode = !vhtSupported && (htOperation.protectionMode == Ieee80211HtProtectionMode::NO_PROTECTION ||
        htOperation.protectionMode == Ieee80211HtProtectionMode::TWENTY_MHZ_PROTECTION);
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
        emit(rateStateChangedSignal, true);
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
    emit(rateStateChangedSignal, true);
}

void Ieee80211Mib::setBssRateSet(const Ieee80211RateSetState& rateSet)
{
    Enter_Method("setBssRateSet");
    validateIeee80211RateSetState(rateSet, "BSS rate state");
    if (bssRateSet == rateSet)
        return;
    bssRateSet = rateSet;
    emit(rateStateChangedSignal, true);
}

void Ieee80211Mib::installBssAndPeerRateSets(const Ieee80211RateSetState& newBssRateSet,
        const MacAddress& peerAddress, const Ieee80211RateSetState& newPeerRateSet)
{
    Enter_Method("installBssAndPeerRateSets");
    validateIeee80211RateSetState(newBssRateSet, "BSS rate state");
    validateIeee80211RateSetState(newPeerRateSet, "peer rate state");
    if (updateBssAndPeerRateSets(newBssRateSet, peerAddress, newPeerRateSet))
        emit(rateStateChangedSignal, true);
}

bool Ieee80211Mib::updateBssAndPeerRateSets(const Ieee80211RateSetState& newBssRateSet,
        const MacAddress& peerAddress, const Ieee80211RateSetState& newPeerRateSet)
{
    bool bssChanged = bssRateSet != newBssRateSet;
    auto peerIt = peerRateSets.find(peerAddress);
    bool peerChanged = peerIt == peerRateSets.end() || peerIt->second != newPeerRateSet;
    if (!bssChanged && !peerChanged)
        return false;
    bssRateSet = newBssRateSet;
    peerRateSets[peerAddress] = newPeerRateSet;
    return true;
}

void Ieee80211Mib::installBssAndPeerState(const Ieee80211RateSetState& newBssRateSet,
        const MacAddress& peerAddress, const Ieee80211RateSetState& newPeerRateSet,
        const Ieee80211HtCapabilities *capabilities, const Ieee80211HtOperation& operation)
{
    Enter_Method("installBssAndPeerState");
    validateIeee80211RateSetState(newBssRateSet, "BSS rate state");
    validateIeee80211RateSetState(newPeerRateSet, "peer rate state");
    bool changed = capabilities != nullptr ? updatePeerHtCapabilities(peerAddress, *capabilities, operation) :
            peerHtStates.erase(peerAddress) != 0;
    changed |= updateBssAndPeerRateSets(newBssRateSet, peerAddress, newPeerRateSet);
    if (changed)
        emit(rateStateChangedSignal, true);
}

void Ieee80211Mib::clearBssAndPeerState(const MacAddress& peerAddress)
{
    Enter_Method("clearBssAndPeerState");
    bool changed = bssRateSet != Ieee80211RateSetState();
    changed |= peerHtStates.erase(peerAddress) != 0;
    changed |= peerRateSets.erase(peerAddress) != 0;
    bssRateSet = Ieee80211RateSetState();
    if (changed)
        emit(rateStateChangedSignal, true);
}

void Ieee80211Mib::clearBssAndAllPeerStates()
{
    Enter_Method("clearBssAndAllPeerStates");
    bool changed = bssRateSet != Ieee80211RateSetState() || !peerHtStates.empty() || !peerRateSets.empty();
    bssRateSet = Ieee80211RateSetState();
    peerHtStates.clear();
    peerRateSets.clear();
    if (changed)
        emit(rateStateChangedSignal, true);
}

void Ieee80211Mib::clearBssRateSet()
{
    Enter_Method("clearBssRateSet");
    if (!bssRateSet.supported.known && !bssRateSet.basic.known && !bssRateSet.operational.known)
        return;
    bssRateSet = Ieee80211RateSetState();
    emit(rateStateChangedSignal, true);
}

void Ieee80211Mib::clearAccessPointState()
{
    Enter_Method("clearAccessPointState");
    bssAccessPointData.stations.clear();
    associationIdReservations.clear();
    bssAccessPointData.associationIds.clear();
    clearBssAndAllPeerStates();
}

void Ieee80211Mib::removePeerRateSet(const MacAddress& address)
{
    Enter_Method("removePeerRateSet");
    if (peerRateSets.erase(address) == 0)
        return;
    emit(rateStateChangedSignal, true);
}

void Ieee80211Mib::clearPeerRateSets()
{
    Enter_Method("clearPeerRateSets");
    if (peerRateSets.empty())
        return;
    peerRateSets.clear();
    emit(rateStateChangedSignal, true);
}

void Ieee80211Mib::setPeerHtCapabilities(const MacAddress& address, const Ieee80211HtCapabilities& capabilities,
        const Ieee80211HtOperation& operation)
{
    Enter_Method("setPeerHtCapabilities");
    if (updatePeerHtCapabilities(address, capabilities, operation))
        emit(rateStateChangedSignal, true);
}

bool Ieee80211Mib::updatePeerHtCapabilities(const MacAddress& address, const Ieee80211HtCapabilities& capabilities,
        const Ieee80211HtOperation& operation)
{
    if (!localHtCapabilitiesValid)
        throw cRuntimeError("Cannot install peer HT capabilities when local HT operation is disabled");
    auto negotiated = negotiateHtCapabilities(localHtCapabilities, capabilities, operation);
    auto& state = peerHtStates[address];
    bool changed = !state.valid || !(state.advertisedCapabilities == capabilities) ||
            !(state.negotiatedCapabilities.localAdvertisement == localHtCapabilities) ||
            !(state.negotiatedCapabilities.operation == operation);
    state.valid = true;
    state.advertisedCapabilities = capabilities;
    state.negotiatedCapabilities = negotiated;
    if (++state.generation == 0)
        state.generation = 1;
    EV_INFO << "Installed peer HT state, peer = " << address
            << ", txValid = " << state.negotiatedCapabilities.localTxPeerRx.valid
            << ", rxValid = " << state.negotiatedCapabilities.localRxPeerTx.valid << endl;
    return changed;
}

void Ieee80211Mib::removePeerHtCapabilities(const MacAddress& address)
{
    Enter_Method("removePeerHtCapabilities");
    if (peerHtStates.erase(address) != 0)
        emit(rateStateChangedSignal, true);
}

void Ieee80211Mib::clearPeerHtCapabilities()
{
    Enter_Method("clearPeerHtCapabilities");
    if (!peerHtStates.empty()) {
        peerHtStates.clear();
        emit(rateStateChangedSignal, true);
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
        emit(rateStateChangedSignal, true);
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
        emit(rateStateChangedSignal, true);
}

} // namespace ieee80211

} // namespace inet

//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/rateselection/RateSelectionBase.h"

#include <cmath>

#include "inet/common/Simsignals.h"
#include "inet/linklayer/ieee80211/mac/rateselection/Ieee80211PeerModeSelection.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

void RateSelectionBase::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        mib.reference(this, "mibModule", true);
    else if (stage == INITSTAGE_LINK_LAYER && modeSet != nullptr)
        fastestMandatoryMode = modeSet->getFastestMandatoryMode();
}

void RateSelectionBase::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    ModeSetListener::receiveSignal(source, signalID, obj, details);
    if (signalID == modesetChangedSignal && modeSet != nullptr)
        fastestMandatoryMode = modeSet->getFastestMandatoryMode();
}

const Ieee80211RateSetState& RateSelectionBase::getBssRateSetForReceiver(const MacAddress& receiver) const
{
    // A selected foreign AP has a separate BSS policy until association commits it as the current BSS.
    if (mib->mode == Ieee80211Mib::INFRASTRUCTURE && mib->bssStationData.stationType == Ieee80211Mib::STATION &&
            !receiver.isMulticast() && (!mib->bssStationData.isAssociated || receiver != mib->bssData.bssid)) {
        if (const auto *target = mib->findPeerRateSet(receiver))
            return *target;
    }
    return mib->getBssRateSet();
}

const MacAddress& RateSelectionBase::getResponsePeer(const Ptr<const Ieee80211TwoAddressHeader>& header) const
{
    // Outgoing management frames can still have an unspecified transmitter during Duration calculation.
    // The receiver identifies incoming unicast frames independently of that later header update.
    return header->getReceiverAddress() == mib->address ? header->getTransmitterAddress() : header->getReceiverAddress();
}

bool RateSelectionBase::isAllowedByRateState(const IIeee80211Mode *mode,
        const MacAddress& receiver, bool groupAddressed) const
{
    if (mode == nullptr || modeSet == nullptr || !modeSet->containsMode(mode))
        return false;
    // VHT negotiation remains outside this model. Preserve its static mode-set policy.
    if (mode->getModulationClass() == IIeee80211Mode::ModulationClass::VHT)
        return true;
    const auto& bss = getBssRateSetForReceiver(receiver);
    const auto& local = mib->getLocalRateSet();
    const auto *peer = groupAddressed ? nullptr : mib->findPeerRateSet(receiver);
    if (mode->getHtMcsIndex() >= 0) {
        if (groupAddressed) {
            // IEEE Std 802.11-2024, 10.6.5.4: use a locally supported HT basic MCS.
            auto width = mode->getDataMode()->getBandwidth();
            if (!mib->isHtOperationSupported() ||
                    mib->localHtCapabilities.supportedChannelWidths.count(width) == 0 ||
                    width > mib->getHtOperation().operatingChannelWidth || mode->isHtShortGuardInterval())
                return false;
        }
        if (!groupAddressed && selectPeerCompatibleMode(modeSet, mib->findPeerHtState(receiver), mode, receiver) != mode)
            return false;
        if (local.operational.known && local.operational.htMcs.count(mode->getHtMcsIndex()) == 0)
            return false;
        if (bss.operational.known &&
                bss.operational.htMcs.count(mode->getHtMcsIndex()) == 0)
            return false;
        if (peer != nullptr && peer->supported.known &&
                peer->supported.htMcs.count(mode->getHtMcsIndex()) == 0)
            return false;
    }
    else {
        auto rate = mode->getDataMode()->getNetBitrate();
        if (local.operational.known && local.operational.legacyRates.count(rate) == 0)
            return false;
        if (bss.operational.known &&
                bss.operational.legacyRates.count(rate) == 0)
            return false;
        if (peer != nullptr && peer->supported.known &&
                peer->supported.legacyRates.count(rate) == 0)
            return false;
    }
    return true;
}

const IIeee80211Mode *RateSelectionBase::getPeerCompatibleMode(const MacAddress& peerAddress,
        const IIeee80211Mode *mode) const
{
    if (mode == nullptr || peerAddress.isMulticast() || mode->getHtMcsIndex() < 0)
        return mode;
    return selectPeerCompatibleMode(modeSet, mib->findPeerHtState(peerAddress), mode, peerAddress);
}

const IIeee80211Mode *RateSelectionBase::selectAllowedMode(const MacAddress& peerAddress,
        const IIeee80211Mode *mode, bool groupAddressed) const
{
    if (mode == nullptr || modeSet == nullptr)
        throw cRuntimeError("Cannot select an allowed IEEE 802.11 mode without a mode set");
    if (isAllowedByRateState(mode, peerAddress, groupAddressed))
        return mode;
    const IIeee80211Mode *best = nullptr;
    auto upperBound = mode->getDataMode()->getNetBitrate();
    for (int i = 0; i < modeSet->getNumModes(); i++) {
        const auto *candidate = modeSet->getMode(i);
        if (candidate->getDataMode()->getNetBitrate() > upperBound)
            continue;
        if (!isAllowedByRateState(candidate, peerAddress, groupAddressed))
            continue;
        if (best == nullptr || candidate->getDataMode()->getNetBitrate() > best->getDataMode()->getNetBitrate())
            best = candidate;
    }
    if (best == nullptr)
        throw cRuntimeError("No allowed IEEE 802.11 mode at or below '%s' exists for peer %s",
                mode->getName(), peerAddress.str().c_str());
    return best;
}

const IIeee80211Mode *RateSelectionBase::selectBasicMode(const IIeee80211Mode *upperBoundMode,
        const MacAddress& receiver, bool allowHtBasicMcs) const
{
    // IEEE Std 802.11-2024, 10.6.5.4 and 10.6.6.2/4: basic-set choice and bounded fallback.
    const auto& bss = getBssRateSetForReceiver(receiver);
    bps upperBound = upperBoundMode == nullptr ? bps(INFINITY) : upperBoundMode->getNonHtReferenceRate();
    const IIeee80211Mode *best = nullptr;
    if (bss.basic.known) {
        for (const auto *candidate : modeSet->getLegacyOperationalModes()) {
            auto rate = candidate->getDataMode()->getNetBitrate();
            if (bss.basic.legacyRates.count(rate) == 0 || rate > upperBound ||
                    !isAllowedByRateState(candidate, receiver, receiver.isMulticast()))
                continue;
            if (best == nullptr || rate > best->getDataMode()->getNetBitrate())
                best = candidate;
        }
    }
    if (best != nullptr)
        return best;
    if (allowHtBasicMcs && bss.basic.known && bss.basic.legacyRates.empty() && !bss.basic.htMcs.empty()) {
        for (int i = 0; i < modeSet->getNumModes(); i++) {
            const auto *candidate = modeSet->getMode(i);
            if (candidate->getHtMcsIndex() < 0 || bss.basic.htMcs.count(candidate->getHtMcsIndex()) == 0 ||
                    !isAllowedByRateState(candidate, receiver, receiver.isMulticast()))
                continue;
            if (best == nullptr || candidate->getDataMode()->getNetBitrate() > best->getDataMode()->getNetBitrate())
                best = candidate;
        }
    }
    if (best != nullptr)
        return best;
    if (receiver.isMulticast() && bss.basic.known &&
            (!bss.basic.legacyRates.empty() || !bss.basic.htMcs.empty()))
        throw cRuntimeError("No operational group mode belongs to the nonempty BSS basic rate set");
    if (upperBoundMode == nullptr && bss.basic.known && !bss.basic.legacyRates.empty())
        throw cRuntimeError("No eligible mode belongs to the nonempty BSS basic rate set");
    // A bound can exclude every basic rate. The fallback remains non-HT and bounded.
    for (const auto *candidate : modeSet->getLegacyOperationalModes()) {
        auto rate = candidate->getDataMode()->getNetBitrate();
        if (!modeSet->getIsMandatory(candidate) || rate > upperBound ||
                !isAllowedByRateState(candidate, receiver, receiver.isMulticast()))
            continue;
        if (best == nullptr || rate > best->getDataMode()->getNetBitrate())
            best = candidate;
    }
    if (best == nullptr)
        throw cRuntimeError("No eligible basic or mandatory legacy rate for peer %s", receiver.str().c_str());
    return best;
}

const IIeee80211Mode *RateSelectionBase::validateConfiguredMode(const IIeee80211Mode *mode,
        const MacAddress& receiver, const char *frameClass) const
{
    // Preserve negotiated HT width, MCS and GI adaptation for a configured target rate.
    auto compatible = getPeerCompatibleMode(receiver, mode);
    if (compatible != mode && mode->getHtMcsIndex() >= 0)
        return selectAllowedMode(receiver, compatible);
    if (!isAllowedByRateState(compatible, receiver, receiver.isMulticast()))
        throw cRuntimeError("Configured %s mode '%s' for peer %s violates local, BSS, or peer rate restrictions",
                frameClass, mode->getName(), receiver.str().c_str());
    return compatible;
}

const IIeee80211Mode *RateSelectionBase::selectGroupMode(const IIeee80211Mode *configuredMode,
        const IIeee80211Mode *preferredMode) const
{
    const auto receiver = MacAddress::BROADCAST_ADDRESS;
    const auto *candidate = configuredMode != nullptr ? configuredMode : preferredMode;
    if (candidate == nullptr)
        return selectBasicMode(nullptr, receiver, true);
    const auto& basic = mib->getBssRateSet().basic;
    auto rate = candidate->getDataMode()->getNetBitrate();
    bool eligible;
    if (basic.known && !basic.legacyRates.empty())
        eligible = candidate->getHtMcsIndex() < 0 &&
                candidate->getModulationClass() != IIeee80211Mode::ModulationClass::VHT &&
                basic.legacyRates.count(rate) != 0;
    else if (basic.known && !basic.htMcs.empty())
        eligible = basic.htMcs.count(candidate->getHtMcsIndex()) != 0;
    else
        eligible = candidate->getHtMcsIndex() < 0 &&
                candidate->getModulationClass() != IIeee80211Mode::ModulationClass::VHT &&
                modeSet->getIsMandatory(candidate);
    if (eligible && isAllowedByRateState(candidate, receiver, true))
        return candidate;
    if (configuredMode != nullptr)
        throw cRuntimeError("Configured group mode '%s' violates the BSS basic or operational rate set", configuredMode->getName());
    // A data or management rate is only a preference for group frames; an explicit group rate is strict.
    return selectBasicMode(nullptr, receiver, true);
}

const IIeee80211Mode *RateSelectionBase::computePrimaryResponseMode(const IIeee80211Mode *elicitingMode,
        Ieee80211ResponseFrameKind responseKind, const MacAddress& receiver) const
{
    // IEEE Std 802.11-2024, 10.6.6.5.2: primary responses and the Basic BA exception.
    if (elicitingMode == nullptr)
        throw cRuntimeError("Cannot select a response mode without an eliciting mode");
    bool htOrVht = elicitingMode->getModulationClass() == IIeee80211Mode::ModulationClass::HT ||
            elicitingMode->getModulationClass() == IIeee80211Mode::ModulationClass::VHT;
    if (responseKind == Ieee80211ResponseFrameKind::BASIC_BLOCK_ACK && !htOrVht) {
        for (const auto *candidate : modeSet->getLegacyOperationalModes())
            if (candidate->getDataMode()->getNetBitrate() == elicitingMode->getDataMode()->getNetBitrate() &&
                    candidate->getModulationClass() == elicitingMode->getModulationClass() &&
                    candidate->getLegacyPreambleType() == elicitingMode->getLegacyPreambleType())
                return candidate;
        throw cRuntimeError("Basic BA cannot match the received BAR mode '%s'", elicitingMode->getName());
    }
    auto referenceRate = elicitingMode->getNonHtReferenceRate();
    if (std::isnan(referenceRate.get()))
        throw cRuntimeError("Eliciting mode '%s' has no non-HT reference rate", elicitingMode->getName());
    auto eligible = [elicitingMode, htOrVht, referenceRate](const IIeee80211Mode *candidate) {
        auto family = candidate->getModulationClass();
        bool compatible = htOrVht ?
                family == IIeee80211Mode::ModulationClass::OFDM || family == IIeee80211Mode::ModulationClass::ERP_OFDM :
                family == elicitingMode->getModulationClass() &&
                candidate->getLegacyPreambleType() == elicitingMode->getLegacyPreambleType();
        return compatible && candidate->getDataMode()->getNetBitrate() <= referenceRate;
    };
    const auto& bss = getBssRateSetForReceiver(receiver);
    const IIeee80211Mode *best = nullptr;
    if (bss.basic.known) {
        for (const auto *candidate : modeSet->getLegacyOperationalModes()) {
            auto rate = candidate->getDataMode()->getNetBitrate();
            if (bss.basic.legacyRates.count(rate) == 0 || !eligible(candidate))
                continue;
            if (best == nullptr || rate > best->getDataMode()->getNetBitrate())
                best = candidate;
        }
    }
    if (best == nullptr) {
        for (const auto *candidate : modeSet->getLegacyOperationalModes()) {
            auto rate = candidate->getDataMode()->getNetBitrate();
            if (!modeSet->getIsMandatory(candidate) || !eligible(candidate))
                continue;
            if (best == nullptr || rate > best->getDataMode()->getNetBitrate())
                best = candidate;
        }
    }
    if (best == nullptr)
        throw cRuntimeError("No primary response mode for eliciting mode '%s' and peer %s", elicitingMode->getName(), receiver.str().c_str());
    return best;
}

const IIeee80211Mode *RateSelectionBase::validateResponseOverride(const IIeee80211Mode *computedMode,
        const IIeee80211Mode *configuredMode, Ieee80211ResponseFrameKind responseKind, const MacAddress& receiver) const
{
    if (configuredMode != nullptr && configuredMode != computedMode)
        throw cRuntimeError("Response override for kind %d and peer %s must equal computed primary mode '%s'",
                static_cast<int>(responseKind), receiver.str().c_str(), computedMode->getName());
    return computedMode;
}

} // namespace ieee80211
} // namespace inet

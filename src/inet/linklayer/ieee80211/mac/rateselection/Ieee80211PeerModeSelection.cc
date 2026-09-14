//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include <cmath>
#include <cstring>

#include "inet/linklayer/ieee80211/mac/rateselection/Ieee80211PeerModeSelection.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

const IIeee80211Mode *selectGroupAddressedMode(const Ieee80211ModeSet *modeSet, const IIeee80211Mode *requestedMode)
{
    // IEEE Std 802.11-2024, 10.6.5.1 and 10.6.5.4. The model advertises
    // mandatory legacy operational modes as its BSS basic legacy rate set.
    const auto *resolvedMode = modeSet->containsMode(requestedMode) ? requestedMode : modeSet->findCompatibleMode(requestedMode);
    const IIeee80211Mode *legacyMode = nullptr;
    for (const auto *candidate : modeSet->getLegacyOperationalModes()) {
        if (!modeSet->getIsMandatory(candidate))
            continue;
        if (candidate == resolvedMode)
            return candidate;
        if (legacyMode == nullptr || candidate->getDataMode()->getNetBitrate() > legacyMode->getDataMode()->getNetBitrate())
            legacyMode = candidate;
    }
    return legacyMode != nullptr ? legacyMode : requestedMode;
}

namespace {

static const IIeee80211Mode *getLegacyFallback(const Ieee80211ModeSet *modeSet,
        const IIeee80211Mode *upperBoundMode, const MacAddress& peerAddress)
{
    auto upperBoundBitrate = upperBoundMode->getDataMode()->getNetBitrate();
    const IIeee80211Mode *legacyMode = nullptr;
    for (const auto *candidate : modeSet->getLegacyOperationalModes()) {
        if (!modeSet->getIsMandatory(candidate) || candidate->getDataMode()->getNetBitrate() > upperBoundBitrate)
            continue;
        if (legacyMode == nullptr ||
                candidate->getDataMode()->getNetBitrate() > legacyMode->getDataMode()->getNetBitrate())
            legacyMode = candidate;
    }
    // Preserve mode sets whose legacy operational set contains no mandatory
    // entry, while keeping the caller's rate as an upper bound.
    if (legacyMode == nullptr) {
        for (const auto *candidate : modeSet->getLegacyOperationalModes()) {
            if (candidate->getDataMode()->getNetBitrate() > upperBoundBitrate)
                continue;
            if (legacyMode == nullptr ||
                    candidate->getDataMode()->getNetBitrate() > legacyMode->getDataMode()->getNetBitrate())
                legacyMode = candidate;
        }
    }
    if (legacyMode == nullptr)
        throw cRuntimeError("No legacy operational mode at or below '%s' is available for peer %s",
                upperBoundMode->getName(), peerAddress.str().c_str());
    return legacyMode;
}

// IEEE Std 802.11-2024, 10.6.5.8: an individually addressed frame shall use
// a receiver-supported MCS/rate and CH_BANDWIDTH permitted by the BSS HT
// Operation. The directional negotiated state is the model's source of the
// receiver's capability advertisement.
static bool isCompatibleHtMode(const IIeee80211Mode *mode, const Ieee80211Mib::PeerHtState *peerHtState)
{
    if (mode == nullptr || peerHtState == nullptr || !peerHtState->valid)
        return false;

    const auto& negotiated = peerHtState->negotiatedCapabilities;
    const auto& receiverCapabilities = negotiated.localTxPeerRx;
    if (!receiverCapabilities.valid)
        return false;

    int mcsIndex = mode->getHtMcsIndex();
    if (mcsIndex < 0 || mcsIndex >= 77 || !receiverCapabilities.supportedMcs[mcsIndex])
        return false;

    auto bandwidth = mode->getDataMode()->getBandwidth();
    if (receiverCapabilities.supportedChannelWidths.count(bandwidth) == 0 ||
            bandwidth > negotiated.operation.operatingChannelWidth)
        return false;

    // IEEE Std 802.11-2024, 10.17 and Table 9-224: a short guard interval
    // is usable only when the receiver advertised it for this channel width.
    if (mode->isHtShortGuardInterval()) {
        if (bandwidth == MHz(20))
            return receiverCapabilities.receiverShortGi20;
        if (bandwidth == MHz(40))
            return receiverCapabilities.receiverShortGi40;
        return false;
    }
    return true;
}

static bool isBetterHtMode(const IIeee80211Mode *candidate, const IIeee80211Mode *current)
{
    auto candidateBitrate = candidate->getDataMode()->getNetBitrate();
    auto currentBitrate = current->getDataMode()->getNetBitrate();
    if (candidateBitrate != currentBitrate)
        return candidateBitrate > currentBitrate;

    // The remaining tie-breaks are deliberately based only on the mode
    // contract, never on pointer identity or allocation order.
    if (candidate->isHtShortGuardInterval() != current->isHtShortGuardInterval())
        return !candidate->isHtShortGuardInterval();
    auto candidateBandwidth = candidate->getDataMode()->getBandwidth();
    auto currentBandwidth = current->getDataMode()->getBandwidth();
    if (candidateBandwidth != currentBandwidth)
        return candidateBandwidth < currentBandwidth;
    if (candidate->getHtMcsIndex() != current->getHtMcsIndex())
        return candidate->getHtMcsIndex() < current->getHtMcsIndex();
    // A mode set normally contains unique MCS/width/GI entries, but keeping a
    // final value-based key makes the choice total even for custom mode sets.
    return std::strcmp(candidate->getName(), current->getName()) < 0;
}

} // namespace

const IIeee80211Mode *selectPeerCompatibleMode(const Ieee80211ModeSet *modeSet,
        const Ieee80211Mib::PeerHtState *peerHtState, const IIeee80211Mode *mode, const MacAddress& peerAddress)
{
    if (mode == nullptr || mode->getHtMcsIndex() < 0)
        return mode;
    if (modeSet == nullptr)
        throw cRuntimeError("Cannot select a peer-compatible HT mode without an IEEE 802.11 mode set");
    if (!modeSet->containsMode(mode))
        throw cRuntimeError("HT mode '%s' is not contained in IEEE 802.11 mode set '%s'",
                mode->getName(), modeSet->getName());

    if (peerHtState == nullptr || !peerHtState->valid || !peerHtState->negotiatedCapabilities.localTxPeerRx.valid)
        return getLegacyFallback(modeSet, mode, peerAddress);
    if (isCompatibleHtMode(mode, peerHtState))
        return mode;

    auto candidateBitrate = mode->getDataMode()->getNetBitrate();
    const IIeee80211Mode *bestMode = nullptr;
    for (int i = 0; i < modeSet->getNumModes(); i++) {
        const auto *candidate = modeSet->getMode(i);
        if (candidate->getHtMcsIndex() < 0 ||
                candidate->getDataMode()->getNetBitrate() > candidateBitrate ||
                !isCompatibleHtMode(candidate, peerHtState))
            continue;
        if (bestMode == nullptr || isBetterHtMode(candidate, bestMode))
            bestMode = candidate;
    }
    return bestMode != nullptr ? bestMode : getLegacyFallback(modeSet, mode, peerAddress);
}

const IIeee80211Mode *selectPeerCompatibleVhtMode(const Ieee80211ModeSet *modeSet,
        const Ieee80211VhtCapabilities& local, const Ieee80211VhtOperation& localOperation,
        const Ieee80211Mib::PeerVhtState *peer, const IIeee80211Mode *requested)
{
    if (requested == nullptr || requested->getVhtMcsIndex() < 0)
        return requested;
    if (modeSet == nullptr)
        throw cRuntimeError("Cannot select a VHT mode without a mode set");
    const IIeee80211Mode *best = nullptr;
    auto ceiling = requested->getDataMode()->getNetBitrate();
    if (peer != nullptr) {
        const auto& remote = peer->advertisedCapabilities;
        auto compatible = [&](const IIeee80211Mode *mode) {
            int mcs = mode->getVhtMcsIndex();
            auto data = mode->getDataMode();
            int nss = data->getNumberOfSpatialStreams();
            auto width = data->getBandwidth();
            if (mcs < 0 || nss < 1 || nss > 8 || local.txMaxMcs[nss - 1] < mcs || remote.rxMaxMcs[nss - 1] < mcs ||
                    width > localOperation.channelWidth || width > peer->operation.channelWidth ||
                    (width > MHz(80) && (!local.supported160Mhz || !remote.supported160Mhz)))
                return false;
            bool shortGi = data->getGuardInterval() == SimTime(400, SIMTIME_NS);
            bool giSupported = width == MHz(20) ? local.shortGi20 && remote.shortGi20 :
                    width == MHz(40) ? local.shortGi40 && remote.shortGi40 :
                    width == MHz(80) ? local.shortGi80 && remote.shortGi80 : local.shortGi160 && remote.shortGi160;
            if (shortGi && !giSupported)
                return false;
            // Highest Supported Long GI Data Rate limits refer to long-GI rate,
            // including when selecting a corresponding short-GI transmission.
            auto longGiRate = data->getNetBitrate();
            if (shortGi) {
                bool found = false;
                for (int i = 0; i < modeSet->getNumModes(); i++) {
                    auto counterpart = modeSet->getMode(i);
                    auto counterpartData = counterpart->getDataMode();
                    if (counterpart->getVhtMcsIndex() == mcs && counterpartData->getBandwidth() == width &&
                            counterpartData->getNumberOfSpatialStreams() == nss &&
                            counterpartData->getGuardInterval() == SimTime(800, SIMTIME_NS)) {
                        longGiRate = counterpartData->getNetBitrate();
                        found = true;
                        break;
                    }
                }
                if (!found)
                    return false;
            }
            // The wire field is floor(rate in Mb/s), per Table 9-315.
            int encodedRate = std::floor(longGiRate.get<Mbps>());
            return (local.txHighestLongGiRateMbps == 0 || encodedRate <= local.txHighestLongGiRateMbps) &&
                    (remote.rxHighestLongGiRateMbps == 0 || encodedRate <= remote.rxHighestLongGiRateMbps);
        };
        if (modeSet->containsMode(requested) && compatible(requested))
            return requested;
        for (int i = 0; i < modeSet->getNumModes(); i++) {
            auto mode = modeSet->getMode(i);
            if (mode->getDataMode()->getNetBitrate() <= ceiling && compatible(mode) &&
                    (best == nullptr || mode->getDataMode()->getNetBitrate() > best->getDataMode()->getNetBitrate()))
                best = mode;
        }
    }
    if (best != nullptr)
        return best;
    for (auto mode : modeSet->getLegacyOperationalModes())
        if (mode->getDataMode()->getNetBitrate() <= ceiling &&
                (best == nullptr || mode->getDataMode()->getNetBitrate() > best->getDataMode()->getNetBitrate()))
            best = mode;
    if (best == nullptr)
        throw cRuntimeError("No legacy fallback for unnegotiated VHT mode");
    return best;
}

} // namespace ieee80211
} // namespace inet

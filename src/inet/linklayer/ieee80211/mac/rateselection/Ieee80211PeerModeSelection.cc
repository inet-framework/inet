//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include <cstring>

#include "inet/linklayer/ieee80211/mac/rateselection/Ieee80211PeerModeSelection.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

namespace {

static const IIeee80211Mode *getLegacyFallback(const Ieee80211ModeSet *modeSet, const MacAddress& peerAddress)
{
    const auto *legacyMode = modeSet->getFastestLegacyOperationalMode();
    if (legacyMode == nullptr)
        throw cRuntimeError("No legacy operational mode is available for peer %s", peerAddress.str().c_str());
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
        return getLegacyFallback(modeSet, peerAddress);
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
    return bestMode != nullptr ? bestMode : getLegacyFallback(modeSet, peerAddress);
}

} // namespace ieee80211
} // namespace inet

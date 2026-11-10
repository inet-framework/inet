//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include <cstring>
#include <tuple>

#include "inet/linklayer/ieee80211/mac/rateselection/Ieee80211PeerModeSelection.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

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
static bool isCompatibleHtMode(const IIeee80211Mode *mode,
        const Ieee80211HtDirectionalCapabilities& receiverCapabilities, const Ieee80211HtOperation& operation)
{
    if (!receiverCapabilities.valid)
        return false;

    int mcsIndex = mode->getHtMcsIndex();
    if (mcsIndex < 0 || mcsIndex >= 77 || !receiverCapabilities.supportedMcs[mcsIndex])
        return false;

    auto bandwidth = mode->getDataMode()->getBandwidth();
    if (receiverCapabilities.supportedChannelWidths.count(bandwidth) == 0 ||
            bandwidth > operation.operatingChannelWidth)
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

static const IIeee80211Mode *selectCompatibleMode(const Ieee80211ModeSet *modeSet,
        const IIeee80211Mode *mode, const MacAddress& peerAddress,
        const Ieee80211HtDirectionalCapabilities *receiverCapabilities, const Ieee80211HtOperation *operation)
{
    if (mode == nullptr || mode->getHtMcsIndex() < 0)
        return mode;
    if (modeSet == nullptr)
        throw cRuntimeError("Cannot select a peer-compatible HT mode without an IEEE 802.11 mode set");
    if (!modeSet->containsMode(mode))
        throw cRuntimeError("HT mode '%s' is not contained in IEEE 802.11 mode set '%s'",
                mode->getName(), modeSet->getName());

    if (receiverCapabilities == nullptr || !receiverCapabilities->valid)
        return getLegacyFallback(modeSet, mode, peerAddress);
    if (isCompatibleHtMode(mode, *receiverCapabilities, *operation))
        return mode;

    auto candidateBitrate = mode->getDataMode()->getNetBitrate();
    const IIeee80211Mode *bestMode = nullptr;
    for (int i = 0; i < modeSet->getNumModes(); i++) {
        const auto *candidate = modeSet->getMode(i);
        if (candidate->getHtMcsIndex() < 0 ||
                candidate->getDataMode()->getNetBitrate() > candidateBitrate ||
                !isCompatibleHtMode(candidate, *receiverCapabilities, *operation))
            continue;
        if (bestMode == nullptr || isBetterHtMode(candidate, bestMode))
            bestMode = candidate;
    }
    return bestMode != nullptr ? bestMode : getLegacyFallback(modeSet, mode, peerAddress);
}

} // namespace

bool isPeerCompatibleMode(const Ieee80211ModeSet *modeSet,
        const Ieee80211Mib::PeerHtState *peerHtState, const IIeee80211Mode *mode)
{
    if (!modeSet || !mode || !modeSet->containsMode(mode))
        return false;
    if (mode->getHtMcsIndex() < 0)
        return true;
    return peerHtState && peerHtState->valid &&
        isCompatibleHtMode(mode, peerHtState->negotiatedCapabilities.localTxPeerRx, peerHtState->negotiatedCapabilities.operation);
}

bool isMeshPeerCompatibleMode(const Ieee80211ModeSet *modes, const Ieee80211PeerPhyCapabilities& local,
        const Ieee80211PeerPhyCapabilities *peer, const IIeee80211Mode *mode, bool shortGiTransmissionEnabled)
{
    if (!modes || !mode || !modes->containsMode(mode))
        return false;
    int vhtMcs = mode->getVhtMcsIndex();
    if (vhtMcs < 0 && mode->getHtMcsIndex() < 0)
        return true;
    if (!peer || !local.htPresent || !peer->htPresent)
        return false;
    if ((mode->isHtShortGuardInterval() || mode->isVhtShortGuardInterval()) && !shortGiTransmissionEnabled)
        return false;
    if (vhtMcs < 0) {
        // The negotiated Tx direction already uses the peer's Rx GI bits;
        // our Rx advertisement is independent of this local Tx activation.
        auto negotiated = negotiateHtCapabilities(local.ht, peer->ht, local.htOperation);
        return isCompatibleHtMode(mode, negotiated.localTxPeerRx, local.htOperation);
    }
    // 2024 10.6.5.8: catalog legality plus directional Tx/Rx MCS/NSS/width/GI.
    int nss = mode->getDataMode()->getNumberOfSpatialStreams();
    auto width = mode->getDataMode()->getBandwidth();
    if (!local.vhtPresent || !peer->vhtPresent || nss < 1 || nss > 8
            || local.vht.txMcs[nss - 1] < vhtMcs || peer->vht.rxMcs[nss - 1] < vhtMcs
            || width > local.vhtOperation.width || width > peer->vhtOperation.width
            || (width == MHz(160) && (!local.vht.width160 || !peer->vht.width160))
            || (peer->vht.rxHighestRate && mode->getDataMode()->getNetBitrate() > Mbps(peer->vht.rxHighestRate)))
        return false;
    if (mode->isVhtShortGuardInterval()) {
        if (width == MHz(20)) return peer->ht.shortGi20;
        if (width == MHz(40)) return peer->ht.shortGi40;
        if (width == MHz(80)) return peer->vht.shortGi80;
        if (width == MHz(160)) return peer->vht.shortGi160;
        return false;
    }
    return true;
}

const IIeee80211Mode *selectMeshPeerCompatibleMode(const Ieee80211ModeSet *modes, const Ieee80211PeerPhyCapabilities& local,
        const Ieee80211PeerPhyCapabilities *peer, const IIeee80211Mode *requested, bool fixed, const MacAddress& address,
        bool shortGiTransmissionEnabled)
{
    if (isMeshPeerCompatibleMode(modes, local, peer, requested, shortGiTransmissionEnabled))
        return requested;
    if (fixed)
        throw cRuntimeError("Configured fixed PHY tuple MCS=%d NSS=%d width=%gMHz GI=%s is incompatible with mesh peer %s",
            requested->getVhtMcsIndex() >= 0 ? requested->getVhtMcsIndex() : requested->getHtMcsIndex(),
            requested->getDataMode()->getNumberOfSpatialStreams(), MHz(requested->getDataMode()->getBandwidth()).get(),
            requested->isHtShortGuardInterval() || requested->isVhtShortGuardInterval() ? "short" : "long", address.str().c_str());
    const IIeee80211Mode *best = nullptr;
    auto key = [](const IIeee80211Mode *mode) {
        return std::make_tuple(mode->getDataMode()->getBandwidth().get(), mode->getDataMode()->getNumberOfSpatialStreams(),
            static_cast<int>(mode->getModulationClass()), mode->isHtShortGuardInterval() || mode->isVhtShortGuardInterval(),
            mode->getVhtMcsIndex(), mode->getHtMcsIndex());
    };
    for (int i = 0; i < modes->getNumModes(); i++) {
        auto candidate = modes->getMode(i);
        if (!isMeshPeerCompatibleMode(modes, local, peer, candidate, shortGiTransmissionEnabled)
                || candidate->getDataMode()->getNetBitrate() > requested->getDataMode()->getNetBitrate())
            continue;
        if (!best || candidate->getDataMode()->getNetBitrate() > best->getDataMode()->getNetBitrate()
                || (candidate->getDataMode()->getNetBitrate() == best->getDataMode()->getNetBitrate()
                    && key(candidate) < key(best)))
            best = candidate;
    }
    if (!best)
        throw cRuntimeError("No legal PHY tuple for mesh peer %s", address.str().c_str());
    return best;
}

const IIeee80211Mode *selectPeerCompatibleMode(const Ieee80211ModeSet *modeSet,
        const Ieee80211Mib::PeerHtState *peerHtState, const IIeee80211Mode *mode, const MacAddress& peerAddress)
{
    const auto *negotiated = peerHtState != nullptr && peerHtState->valid ? &peerHtState->negotiatedCapabilities : nullptr;
    return selectCompatibleMode(modeSet, mode, peerAddress,
            negotiated != nullptr ? &negotiated->localTxPeerRx : nullptr,
            negotiated != nullptr ? &negotiated->operation : nullptr);
}

const IIeee80211Mode *selectPeerCompatibleResponseMode(const Ieee80211ModeSet *modeSet,
        const Ieee80211Mib::PeerHtState *peerHtState, const IIeee80211Mode *mode,
        const MacAddress& peerAddress, ResponseQueryRole role)
{
    const auto *negotiated = peerHtState != nullptr && peerHtState->valid ? &peerHtState->negotiatedCapabilities : nullptr;
    const auto *direction = negotiated == nullptr ? nullptr :
            role == ResponseQueryRole::PREDICT ? &negotiated->localRxPeerTx : &negotiated->localTxPeerRx;
    return selectCompatibleMode(modeSet, mode, peerAddress, direction,
            negotiated != nullptr ? &negotiated->operation : nullptr);
}

} // namespace ieee80211
} // namespace inet

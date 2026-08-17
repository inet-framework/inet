//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE80211HTCAPABILITIES_H
#define __INET_IEEE80211HTCAPABILITIES_H

#include <algorithm>
#include <array>
#include <set>

#include "inet/common/Units.h"

namespace inet {
namespace ieee80211 {

using namespace units::values;

// IEEE Std 802.11-2024, Table 9-230.
enum class Ieee80211HtProtectionMode : uint8_t {
    NO_PROTECTION = 0,
    NONMEMBER_PROTECTION = 1,
    TWENTY_MHZ_PROTECTION = 2,
    NON_HT_MIXED = 3,
};

struct Ieee80211HtMcsNssMap
{
    std::array<int, 4> maxMcsPerNss;

    Ieee80211HtMcsNssMap() { maxMcsPerNss.fill(-1); }
};

/**
 * Model-backed subset of the HT Capabilities element (IEEE Std 802.11-2024, 9.4.2.54).
 * Unrepresented optional capabilities are encoded as unsupported or reserved.
 */
struct Ieee80211HtCapabilities
{
    std::set<Hz> supportedChannelWidths;
    std::array<bool, 77> rxMcsSupported = {};
    bool txMcsSetDefined = true;
    bool txRxMcsSetNotEqual = false;
    int txMaxNss = 0;
    bool txUnequalModulation = false;
    // Modeled per-NSS local Tx ceiling. It is deliberately left unknown when a
    // received unequal Tx/Rx advertisement supplies only Table 9-226's summary fields.
    Ieee80211HtMcsNssMap txMcsNss;
    bool ldpc = false;
    bool greenfield = false;
    bool shortGi20 = false;
    bool shortGi40 = false;
    int maxAmpduLengthExponent = 0;
};

/** Model-backed subset of the HT Operation element (IEEE Std 802.11-2024, 9.4.2.55). */
struct Ieee80211HtOperation
{
    Hz operatingChannelWidth = MHz(20);
    int primaryChannel = 0;
    int secondaryChannelOffset = 0;
    Ieee80211HtProtectionMode protectionMode = Ieee80211HtProtectionMode::NO_PROTECTION;
    std::array<bool, 77> basicMcsSupported = {};
};

struct Ieee80211HtDirectionalCapabilities
{
    bool valid = false;
    std::set<Hz> supportedChannelWidths;
    Ieee80211HtMcsNssMap mcsNss;
    std::array<bool, 77> supportedMcs = {};
    bool receiverLdpc = false;
    bool receiverShortGi20 = false;
    bool receiverShortGi40 = false;
    int receiverMaxAmpduLengthExponent = 0;
};

struct Ieee80211NegotiatedHtCapabilities
{
    Ieee80211HtCapabilities localAdvertisement;
    Ieee80211HtCapabilities peerAdvertisement;
    Ieee80211HtDirectionalCapabilities localTxPeerRx;
    Ieee80211HtDirectionalCapabilities localRxPeerTx;
    Ieee80211HtOperation operation;
};

inline Ieee80211NegotiatedHtCapabilities negotiateHtCapabilities(const Ieee80211HtCapabilities& local,
        const Ieee80211HtCapabilities& peer, const Ieee80211HtOperation& operation)
{
    Ieee80211NegotiatedHtCapabilities negotiated;
    negotiated.localAdvertisement = local;
    negotiated.peerAdvertisement = peer;
    negotiated.operation = operation;
    for (const auto& width : local.supportedChannelWidths)
        if (peer.supportedChannelWidths.count(width)) {
            negotiated.localTxPeerRx.supportedChannelWidths.insert(width);
            negotiated.localRxPeerTx.supportedChannelWidths.insert(width);
    }
    for (int mcs = 0; mcs < 77; mcs++) {
        int nss = mcs / 8;
        bool localTx = local.txMcsSetDefined && !local.txRxMcsSetNotEqual ? local.rxMcsSupported[mcs] :
                nss < 4 && local.txMcsNss.maxMcsPerNss[nss] >= mcs % 8;
        // Table 9-226 defines the equal case as the exact Rx MCS bitmap, not
        // a contiguous range ending at the highest advertised MCS.
        bool peerTx = peer.txMcsSetDefined && !peer.txRxMcsSetNotEqual && peer.rxMcsSupported[mcs];
        negotiated.localTxPeerRx.supportedMcs[mcs] = localTx && peer.rxMcsSupported[mcs];
        negotiated.localRxPeerTx.supportedMcs[mcs] = local.rxMcsSupported[mcs] && peerTx;
    }
    for (int nss = 0; nss < 4; nss++) {
        for (int mcs = 0; mcs < 8; mcs++) {
            if (negotiated.localTxPeerRx.supportedMcs[nss * 8 + mcs])
                negotiated.localTxPeerRx.mcsNss.maxMcsPerNss[nss] = mcs;
            if (negotiated.localRxPeerTx.supportedMcs[nss * 8 + mcs])
                negotiated.localRxPeerTx.mcsNss.maxMcsPerNss[nss] = mcs;
        }
    }
    // LDPC and short-GI bits advertise receiver capability, so they are directional.
    negotiated.localTxPeerRx.receiverLdpc = peer.ldpc;
    negotiated.localRxPeerTx.receiverLdpc = local.ldpc;
    negotiated.localTxPeerRx.receiverShortGi20 = peer.shortGi20;
    negotiated.localRxPeerTx.receiverShortGi20 = local.shortGi20;
    negotiated.localTxPeerRx.receiverShortGi40 = peer.shortGi40;
    negotiated.localRxPeerTx.receiverShortGi40 = local.shortGi40;
    negotiated.localTxPeerRx.receiverMaxAmpduLengthExponent = peer.maxAmpduLengthExponent;
    negotiated.localRxPeerTx.receiverMaxAmpduLengthExponent = local.maxAmpduLengthExponent;
    // The HT Operation width is the BSS maximum. A 20 MHz-only STA may join a
    // 20/40 MHz BSS, so validity requires a common usable width, not equality
    // with the advertised BSS width (IEEE Std 802.11-2024, 11.15.2).
    negotiated.localTxPeerRx.valid = !negotiated.localTxPeerRx.supportedChannelWidths.empty() &&
            negotiated.localTxPeerRx.supportedMcs[0];
    // An undefined peer Tx MCS set is permitted by Table 9-226. It is unknown,
    // rather than an advertisement that the peer cannot transmit.
    bool peerTxMcsUnknown = !peer.txMcsSetDefined || peer.txRxMcsSetNotEqual;
    negotiated.localRxPeerTx.valid = !negotiated.localRxPeerTx.supportedChannelWidths.empty() &&
            (peerTxMcsUnknown || negotiated.localRxPeerTx.supportedMcs[0]);
    return negotiated;
}

inline bool supportsBasicHtMcsSet(const Ieee80211HtCapabilities& capabilities, const Ieee80211HtOperation& operation)
{
    for (int mcs = 0; mcs < 77; mcs++)
        if (operation.basicMcsSupported[mcs] && !capabilities.rxMcsSupported[mcs])
            return false;
    return true;
}

} // namespace ieee80211
} // namespace inet

#endif

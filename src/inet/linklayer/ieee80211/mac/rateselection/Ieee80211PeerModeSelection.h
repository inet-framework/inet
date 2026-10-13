//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211PEERMODESELECTION_H
#define __INET_IEEE80211PEERMODESELECTION_H

#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211Mib.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"

namespace inet {
namespace ieee80211 {

/**
 * Selects a mode compatible with local transmission to the peer.
 * Non-HT modes remain unchanged. Absent peer state selects a legacy mode
 * whose bitrate does not exceed the requested HT mode.
 */
INET_API const physicallayer::IIeee80211Mode *selectPeerCompatibleMode(
        const physicallayer::Ieee80211ModeSet *modeSet,
        const Ieee80211Mib::PeerHtState *peerHtState,
        const physicallayer::IIeee80211Mode *mode,
        const MacAddress& peerAddress);

/**
 * Selects a compatible response mode from the directional negotiated state.
 * PREDICT uses local reception from the peer. RESPOND uses local transmission
 * to the peer. Unknown peer Tx MCS support retains the legacy prediction fallback.
 */
INET_API const physicallayer::IIeee80211Mode *selectPeerCompatibleResponseMode(
        const physicallayer::Ieee80211ModeSet *modeSet,
        const Ieee80211Mib::PeerHtState *peerHtState,
        const physicallayer::IIeee80211Mode *mode,
        const MacAddress& peerAddress,
        ResponseQueryRole role);

} // namespace ieee80211
} // namespace inet

#endif

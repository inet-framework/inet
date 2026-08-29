//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211PEERMODESELECTION_H
#define __INET_IEEE80211PEERMODESELECTION_H

#include "inet/linklayer/ieee80211/mib/Ieee80211Mib.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"

namespace inet {
namespace ieee80211 {

/**
 * Selects a mode that is compatible with the negotiated receive capabilities
 * of a peer. Non-HT modes are returned unchanged. A null peer state denotes
 * that HT negotiation is unavailable and selects the mode-set's legacy
 * operational fallback.
 */
INET_API const physicallayer::IIeee80211Mode *selectPeerCompatibleMode(
        const physicallayer::Ieee80211ModeSet *modeSet,
        const Ieee80211Mib::PeerHtState *peerHtState,
        const physicallayer::IIeee80211Mode *mode,
        const MacAddress& peerAddress);

} // namespace ieee80211
} // namespace inet

#endif

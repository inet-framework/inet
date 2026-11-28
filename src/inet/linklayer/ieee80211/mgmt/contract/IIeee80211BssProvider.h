//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef INET_IIEEE80211BSSPROVIDER_H
#define INET_IIEEE80211BSSPROVIDER_H
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211HtCapabilities.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211RateSet.h"
namespace inet::ieee80211 {
/** AP-owned preparation and relationship transitions for the simplified no-air model. */
class INET_API IIeee80211BssProvider
{
  public:
    // Boolean availability after AP startup has prepared the BSS or shutdown has cleared it.
    static const simsignal_t simplifiedAssociationAvailabilityChangedSignal;

    virtual ~IIeee80211BssProvider() = default;
    virtual bool isAcceptingSimplifiedPeers() const = 0;
    virtual void prepareBss() = 0;
    virtual void installSimplifiedPeer(const MacAddress& address, const Ieee80211HtCapabilities *capabilities, const Ieee80211RateSetState& rates) = 0;
    virtual void removeSimplifiedPeer(const MacAddress& address) = 0;
};
} // namespace inet::ieee80211
#endif

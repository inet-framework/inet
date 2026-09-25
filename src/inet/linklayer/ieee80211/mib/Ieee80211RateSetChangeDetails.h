//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE80211RATESETCHANGEDETAILS_H
#define __INET_IEEE80211RATESETCHANGEDETAILS_H

#include "inet/linklayer/common/MacAddress.h"

namespace inet {
namespace ieee80211 {

// The publisher owns this value for the duration of the signal callback.
class INET_API Ieee80211RateSetChangeDetails : public cObject
{
  public:
    enum class Change { UNCHANGED = 0, INSTALLED = 1, REPLACED = 2, CLEARED = 3 };

  private:
    const MacAddress peerAddress;
    const Change bssChange;
    const Change peerChange;

  public:
    Ieee80211RateSetChangeDetails() : bssChange(Change::UNCHANGED), peerChange(Change::UNCHANGED) {}
    Ieee80211RateSetChangeDetails(const MacAddress& peerAddress, Change bssChange, Change peerChange) :
        peerAddress(peerAddress), bssChange(bssChange), peerChange(peerChange) {}
    const MacAddress& getPeerAddress() const { return peerAddress; }
    Change getBssChange() const { return bssChange; }
    Change getPeerChange() const { return peerChange; }
};

} // namespace ieee80211
} // namespace inet

#endif

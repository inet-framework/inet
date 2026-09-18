//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef INET_IIEEE80211MACCONFIGURATION_H
#define INET_IIEEE80211MACCONFIGURATION_H

#include "inet/linklayer/ieee80211/mac/contract/IIeee80211ModeSetProvider.h"

namespace inet::ieee80211 {
/** MAC configuration extends catalog access with preparation after PHY readiness.
 * Repeated preparation preserves protocol and algorithm state.
 */
class INET_API IIeee80211MacConfiguration : public IIeee80211ModeSetProvider
{
  public:
    virtual ~IIeee80211MacConfiguration() = default;
    virtual void prepareLocalCapabilities() = 0;
};
} // namespace inet::ieee80211
#endif

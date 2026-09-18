//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef INET_IIEEE80211MACCONFIGURATION_H
#define INET_IIEEE80211MACCONFIGURATION_H

#include "inet/common/INETDefs.h"

namespace inet::physicallayer { class Ieee80211ModeSet; }

namespace inet::ieee80211 {
/** Configured catalog is ready after LOCAL; explicit preparation requires PHY readiness.
 * Repeated preparation preserves protocol and algorithm state.
 */
class INET_API IIeee80211MacConfiguration
{
  public:
    virtual ~IIeee80211MacConfiguration() = default;
    [[nodiscard]] virtual const physicallayer::Ieee80211ModeSet *getConfiguredModeSet() const = 0;
    virtual void prepareLocalCapabilities() = 0;
};
} // namespace inet::ieee80211
#endif

//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef INET_IIEEE80211MODESETPROVIDER_H
#define INET_IIEEE80211MODESETPROVIDER_H

#include "inet/common/INETDefs.h"

namespace inet::physicallayer { class Ieee80211ModeSet; }

namespace inet::ieee80211 {

/** Read-only configured catalog, available after LOCAL and stable across stop/restart. */
class INET_API IIeee80211ModeSetProvider
{
  public:
    virtual ~IIeee80211ModeSetProvider() = default;
    [[nodiscard]] virtual const physicallayer::Ieee80211ModeSet *getConfiguredModeSet() const = 0;
};

} // namespace inet::ieee80211

#endif

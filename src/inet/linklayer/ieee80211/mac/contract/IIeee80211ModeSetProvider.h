// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_IIEEE80211MODESETPROVIDER_H
#define __INET_IIEEE80211MODESETPROVIDER_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace physicallayer {
class Ieee80211ModeSet;
}
namespace ieee80211 {

// Read-only access to the configured MAC mode catalog.
class INET_API IIeee80211ModeSetProvider
{
  public:
    virtual ~IIeee80211ModeSetProvider() {}

    // Available after link-layer initialization. The catalog owns the returned value.
    virtual const physicallayer::Ieee80211ModeSet *getModeSet() const = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IIEEE80211MODESETLISTENER_H
#define __INET_IIEEE80211MODESETLISTENER_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace physicallayer {

class Ieee80211ModeSet;

/**
 * Explicitly registered consumer of an interface's mode-set changes. The
 * coordinator applies consumers before publishing modesetChanged. Implementing
 * this contract or subscribing to that signal does not register a consumer.
 * Applying a change must not notify observers. Failures are fatal simulation
 * errors; partially applied changes are not rolled back.
 */
class INET_API IIeee80211ModeSetListener
{
  public:
    virtual ~IIeee80211ModeSetListener() = default;
    virtual const Ieee80211ModeSet *getModeSet() const = 0;
    virtual void applyModeSet(const Ieee80211ModeSet *modeSet) = 0;
};

} // namespace physicallayer
} // namespace inet

#endif

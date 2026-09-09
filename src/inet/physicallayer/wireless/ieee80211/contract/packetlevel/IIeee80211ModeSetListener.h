//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IIEEE80211MODESETLISTENER_H
#define __INET_IIEEE80211MODESETLISTENER_H

#include <functional>

#include "inet/common/INETDefs.h"

namespace inet {
namespace physicallayer {

class Ieee80211ModeSet;

/**
 * Transaction participant for a radio's mode-set changes. Stateful subscribers
 * to modesetChanged must implement this role to support recoverable changes.
 * The radio captures every participant before applying any change, applies all
 * participants before publishing the signal, and restores them if a call fails.
 * Snapshot callbacks are one-shot, must not throw or emit signals, and restore
 * all state affected by applyModeSet. Applying a change must not notify observers.
 */
class INET_API IIeee80211ModeSetListener
{
  public:
    virtual ~IIeee80211ModeSetListener() = default;
    virtual const Ieee80211ModeSet *getModeSet() const = 0;
    virtual std::function<void()> saveModeSetState() = 0;
    virtual void applyModeSet(const Ieee80211ModeSet *modeSet) = 0;
};

} // namespace physicallayer
} // namespace inet

#endif

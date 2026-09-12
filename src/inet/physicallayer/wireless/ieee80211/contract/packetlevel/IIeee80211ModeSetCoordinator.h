//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IIEEE80211MODESETCOORDINATOR_H
#define __INET_IIEEE80211MODESETCOORDINATOR_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace physicallayer {

class Ieee80211ModeSet;

/**
 * Same-interface mode-set transaction owner. Registration is explicit and unrelated
 * to signal subscriptions. Consumers are modules implementing IIeee80211ModeSetListener.
 * MAC_STATE precedes DERIVED_STATE; consumers within DERIVED_STATE are independent.
 * Begin validates membership before PHY mutation; complete applies consumers and
 * publishes the completed fact. Any failure after begin is fatal, without rollback.
 */
class INET_API IIeee80211ModeSetCoordinator
{
  public:
    enum Phase { MAC_STATE, DERIVED_STATE };
    virtual ~IIeee80211ModeSetCoordinator() = default;
    // Register during initialization; repeated registration in the same phase is idempotent.
    virtual void registerModeSetConsumer(cModule *consumer, Phase phase) = 0;
    // Explicitly detach before deleting a consumer. Membership cannot change during a transaction.
    virtual void unregisterModeSetConsumer(cModule *consumer) = 0;
    virtual void beginModeSetChange(const Ieee80211ModeSet *modeSet) = 0;
    virtual void completeModeSetChange(const Ieee80211ModeSet *modeSet) = 0;
};

} // namespace physicallayer
} // namespace inet

#endif

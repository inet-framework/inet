//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE80211INTERFACE_H
#define __INET_IEEE80211INTERFACE_H

#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/physicallayer/wireless/ieee80211/contract/packetlevel/IIeee80211ModeSetCoordinator.h"

namespace inet {
namespace ieee80211 {

class INET_API Ieee80211Interface : public NetworkInterface, public physicallayer::IIeee80211ModeSetCoordinator
{
  protected:
    std::map<int, Phase> modeSetConsumers;
    const physicallayer::Ieee80211ModeSet *pendingModeSet = nullptr;
    const physicallayer::Ieee80211ModeSet *modeSet = nullptr;
    bool changingModeSet = false;
    bool modeSetInitialized = false;
    bool modeSetPublished = false;

    virtual void initialize(int stage) override;

  public:
    virtual void registerModeSetConsumer(cModule *consumer, Phase phase) override;
    virtual void unregisterModeSetConsumer(cModule *consumer) override;
    virtual void beginModeSetChange(const physicallayer::Ieee80211ModeSet *modeSet) override;
    virtual void completeModeSetChange(const physicallayer::Ieee80211ModeSet *modeSet) override;
};

} // namespace ieee80211
} // namespace inet

#endif

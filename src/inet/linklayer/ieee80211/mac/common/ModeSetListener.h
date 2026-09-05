//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MODESETLISTENER_H
#define __INET_MODESETLISTENER_H

#include "inet/common/SimpleModule.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/physicallayer/wireless/ieee80211/contract/packetlevel/IIeee80211ModeSetListener.h"

namespace inet {
namespace ieee80211 {

class INET_API ModeSetListener : public SimpleModule, public cListener, public physicallayer::IIeee80211ModeSetListener
{
  public:
    virtual const physicallayer::Ieee80211ModeSet *getModeSet() const override { return modeSet; }
    virtual std::function<void()> saveModeSetState() override;
    virtual void applyModeSet(const physicallayer::Ieee80211ModeSet *modeSet) override;

  protected:
    physicallayer::Ieee80211ModeSet *modeSet = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif


//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MODESETLISTENER_H
#define __INET_MODESETLISTENER_H

#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.h"

namespace inet {
namespace ieee80211 {

class INET_API ModeSetListener : public cSimpleModule, public cListener
{
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


//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211OFDMMODULATORMODULE_H
#define __INET_IEEE80211OFDMMODULATORMODULE_H

#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmModulator.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211OfdmModulatorModule : public IModulator, public cSimpleModule
{
  protected:
    const Ieee80211OfdmModulator *ofdmModulator;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("This module doesn't handle self messages"); }

  public:
    virtual ~Ieee80211OfdmModulatorModule();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return ofdmModulator->printToStream(stream, level); }
    const Ieee80211OfdmModulation *getModulation() const override { return ofdmModulator->getModulation(); }
    const ITransmissionSymbolModel *modulate(const ITransmissionBitModel *bitModel) const override;
};

} /* namespace physicallayer */
} /* namespace inet */

#endif


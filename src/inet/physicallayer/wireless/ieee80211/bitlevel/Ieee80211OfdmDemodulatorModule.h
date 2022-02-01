//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211OFDMDEMODULATORMODULE_H
#define __INET_IEEE80211OFDMDEMODULATORMODULE_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/IDemodulator.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmDemodulator.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211OfdmDemodulatorModule : public IDemodulator, public cSimpleModule
{
  protected:
    const Ieee80211OfdmDemodulator *ofdmDemodulator = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("This module doesn't handle self messages"); }

  public:
    virtual ~Ieee80211OfdmDemodulatorModule();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return ofdmDemodulator->printToStream(stream, level); }
    const Ieee80211OfdmModulation *getModulation() const { return ofdmDemodulator->getModulation(); }
    const IReceptionBitModel *demodulate(const IReceptionSymbolModel *symbolModel) const override;
};
} /* namespace physicallayer */
} /* namespace inet */

#endif


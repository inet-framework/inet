//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211OFDMINTERLEAVERMODULE_H
#define __INET_IEEE80211OFDMINTERLEAVERMODULE_H

#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmInterleaver.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211OfdmInterleaverModule : public cSimpleModule, public IInterleaver
{
  protected:
    const Ieee80211OfdmInterleaver *interleaver = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("This module doesn't handle self messages."); }

  public:
    virtual ~Ieee80211OfdmInterleaverModule();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual BitVector interleave(const BitVector& bits) const override { return interleaver->interleave(bits); }
    virtual BitVector deinterleave(const BitVector& bits) const override { return interleaver->deinterleave(bits); }
    virtual const Ieee80211OfdmInterleaving *getInterleaving() const override { return interleaver->getInterleaving(); }
};
} /* namespace physicallayer */
} /* namespace inet */

#endif


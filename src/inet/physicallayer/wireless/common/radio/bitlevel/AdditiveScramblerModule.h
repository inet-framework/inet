//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ADDITIVESCRAMBLERMODULE_H
#define __INET_ADDITIVESCRAMBLERMODULE_H

#include "inet/physicallayer/wireless/common/radio/bitlevel/AdditiveScrambler.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/AdditiveScrambling.h"

namespace inet {
namespace physicallayer {

class INET_API AdditiveScramblerModule : public cSimpleModule, public IScrambler
{
  protected:
    const AdditiveScrambler *scrambler;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("This module doesn't handle self messages"); }

  public:
    virtual ~AdditiveScramblerModule();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual BitVector scramble(const BitVector& bits) const override { return scrambler->scramble(bits); }
    virtual BitVector descramble(const BitVector& bits) const override { return scrambler->descramble(bits); }
    virtual const AdditiveScrambling *getScrambling() const override { return scrambler->getScrambling(); }
};

} /* namespace physicallayer */
} /* namespace inet */

#endif


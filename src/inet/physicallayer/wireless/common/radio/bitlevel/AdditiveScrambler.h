//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ADDITIVESCRAMBLER_H
#define __INET_ADDITIVESCRAMBLER_H

#include "inet/common/BitVector.h"
#include "inet/common/ShortBitVector.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/AdditiveScrambler.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/AdditiveScrambling.h"

namespace inet {
namespace physicallayer {

/*
 * It is an additive scrambler implementation, it can be used with arbitrary
 * generator polynomials.
 * http://en.wikipedia.org/wiki/Scrambler#Additive_.28synchronous.29_scramblers
 */
class INET_API AdditiveScrambler : public IScrambler
{
  protected:
    BitVector scramblingSequence;
    const AdditiveScrambling *scrambling;

  protected:
    inline bool eXOR(bool alpha, bool beta) const { return alpha != beta; }
    BitVector generateScramblingSequence(const ShortBitVector& generatorPolynomial, const ShortBitVector& seed) const;

  public:
    AdditiveScrambler(const AdditiveScrambling *scrambling);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    BitVector scramble(const BitVector& bits) const override;
    BitVector descramble(const BitVector& bits) const override { return scramble(bits); }
    const AdditiveScrambling *getScrambling() const override { return scrambling; }
    const BitVector& getScramblingSequcene() const { return scramblingSequence; }
};

} /* namespace physicallayer */
} /* namespace inet */

#endif


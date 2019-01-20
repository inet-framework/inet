//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_ADDITIVESCRAMBLER_H
#define __INET_ADDITIVESCRAMBLER_H

#include "inet/common/BitVector.h"
#include "inet/common/INETDefs.h"
#include "inet/common/ShortBitVector.h"
#include "inet/physicallayer/common/bitlevel/AdditiveScrambler.h"
#include "inet/physicallayer/common/bitlevel/AdditiveScrambling.h"

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

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
    BitVector scramble(const BitVector& bits) const override;
    BitVector descramble(const BitVector& bits) const override { return scramble(bits); }
    const AdditiveScrambling *getScrambling() const override { return scrambling; }
    const BitVector& getScramblingSequcene() const { return scramblingSequence; }
};

} /* namespace physicallayer */
} /* namespace inet */

#endif // __INET_ADDITIVESCRAMBLER_H


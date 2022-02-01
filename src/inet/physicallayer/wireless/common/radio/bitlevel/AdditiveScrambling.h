//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ADDITIVESCRAMBLING_H
#define __INET_ADDITIVESCRAMBLING_H

#include "inet/common/ShortBitVector.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IScrambler.h"

namespace inet {
namespace physicallayer {

class INET_API AdditiveScrambling : public IScrambling
{
  protected:
    ShortBitVector seed;
    ShortBitVector generatorPolynomial;

  public:
    AdditiveScrambling(const ShortBitVector& seed, const ShortBitVector& generatorPolynomial);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    const ShortBitVector& getGeneratorPolynomial() const { return generatorPolynomial; }
    const ShortBitVector& getSeed() const { return seed; }
};

} /* namespace physicallayer */
} /* namespace inet */

#endif


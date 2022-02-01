//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/radio/bitlevel/AdditiveScrambling.h"

namespace inet {
namespace physicallayer {

AdditiveScrambling::AdditiveScrambling(const ShortBitVector& seed, const ShortBitVector& generatorPolynomial) :
    seed(seed),
    generatorPolynomial(generatorPolynomial)
{
}

std::ostream& AdditiveScrambling::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "AdditiveScrambling";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(seed)
               << EV_FIELD(generatorPolynomial);
    return stream;
}

} /* namespace physicallayer */
} /* namespace inet */


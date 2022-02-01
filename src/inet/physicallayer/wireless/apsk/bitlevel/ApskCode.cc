//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/apsk/bitlevel/ApskCode.h"

namespace inet {

namespace physicallayer {

#define SYMBOL_SIZE    48

ApskCode::ApskCode(const ConvolutionalCode *convCode, const IInterleaving *interleaving, const IScrambling *scrambling) :
    convolutionalCode(convCode),
    interleaving(interleaving),
    scrambling(scrambling)
{
}

ApskCode::~ApskCode()
{
    delete convolutionalCode;
    delete interleaving;
    delete scrambling;
}

std::ostream& ApskCode::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ApskCode";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(convolutionalCode, printFieldToString(convolutionalCode, level + 1, evFlags))
               << EV_FIELD(interleaving, printFieldToString(interleaving, level + 1, evFlags))
               << EV_FIELD(scrambling, printFieldToString(scrambling, level + 1, evFlags));
    return stream;
}

} /* namespace physicallayer */

} /* namespace inet */


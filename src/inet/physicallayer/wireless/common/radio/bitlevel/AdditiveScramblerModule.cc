//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/radio/bitlevel/AdditiveScramblerModule.h"

namespace inet {
namespace physicallayer {

Define_Module(AdditiveScramblerModule);

void AdditiveScramblerModule::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        ShortBitVector seed(par("seed").stringValue());
        ShortBitVector generatorPolynomial(par("generatorPolynomial").stringValue());
        const AdditiveScrambling *scrambling = new AdditiveScrambling(seed, generatorPolynomial);
        scrambler = new AdditiveScrambler(scrambling);
    }
}

std::ostream& AdditiveScramblerModule::printToStream(std::ostream& stream, int level, int evFlags) const
{
    return scrambler->printToStream(stream, level);
}

AdditiveScramblerModule::~AdditiveScramblerModule()
{
    delete scrambler->getScrambling();
    delete scrambler;
}

} /* namespace physicallayer */
} /* namespace inet */


//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/radio/bitlevel/AdditiveScrambler.h"

#include "inet/common/ShortBitVector.h"

namespace inet {
namespace physicallayer {

std::ostream& AdditiveScrambler::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "AdditiveScrambler";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(scrambling, printFieldToString(scrambling, level + 1, evFlags));
    return stream;
}

BitVector AdditiveScrambler::scramble(const BitVector& bits) const
{
    EV_DEBUG << "Scrambling the following bits: " << bits << endl;
    BitVector scrambledBits;
    int sequenceLength = scramblingSequence.getSize();
    for (unsigned int i = 0; i < bits.getSize(); i++) {
        int scramblingIndex = i % sequenceLength;
        bool scrambledBit = eXOR(bits.getBit(i), scramblingSequence.getBit(scramblingIndex));
        scrambledBits.appendBit(scrambledBit);
    }
    EV_DEBUG << "The scrambled bits are: " << scrambledBits << endl;
    return scrambledBits;
}

BitVector AdditiveScrambler::generateScramblingSequence(const ShortBitVector& generatorPolynomial, const ShortBitVector& seed) const
{
    BitVector scramblingSequence;
    int sequenceLength = (int)pow(2, seed.getSize()) - 1;
    ShortBitVector shiftRegisters = seed;
    for (int i = 0; i < sequenceLength; i++) {
        bool registerSum = false;
        for (unsigned int j = 0; j < generatorPolynomial.getSize(); j++) {
            if (generatorPolynomial.getBit(j))
                registerSum = eXOR(shiftRegisters.getBit(j), registerSum);
        }
        shiftRegisters.leftShift(1);
        shiftRegisters.setBit(0, registerSum);
        scramblingSequence.appendBit(registerSum);
    }
    return scramblingSequence;
}

AdditiveScrambler::AdditiveScrambler(const AdditiveScrambling *scrambling) : scrambling(scrambling)
{
    ShortBitVector generatorPolynomial = scrambling->getGeneratorPolynomial();
    ShortBitVector seed = scrambling->getSeed();
    scramblingSequence = generateScramblingSequence(generatorPolynomial, seed);
}

} /* namespace physicallayer */
} /* namespace inet */


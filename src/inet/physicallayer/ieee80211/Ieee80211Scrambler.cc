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

#include "inet/physicallayer/ieee80211/Ieee80211Scrambler.h"
#include "inet/common/ShortBitVector.h"

namespace inet {
namespace physicallayer {

Define_Module(Ieee80211Scrambler);

void Ieee80211Scrambler::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        ShortBitVector seed(par("seed").stringValue());
        ShortBitVector generatorPolynomial(par("generatorPolynomial").stringValue());
        scramblingSequence = generateScramblingSequence(generatorPolynomial, seed);
    }
}

BitVector Ieee80211Scrambler::scrambling(const BitVector& bits) const
{
    EV_DETAIL << "Scrambling the following bits: " << bits << endl;
    BitVector scrambledBits;
    int sequenceLength = scramblingSequence.getSize();
    for (unsigned int i = 0; i < bits.getSize(); i++)
    {
        int scramblingIndex = i % sequenceLength;
        bool scrambledBit = eXOR(bits.getBit(i), scramblingSequence.getBit(scramblingIndex));
        scrambledBits.appendBit(scrambledBit);
    }
    EV_DETAIL << "The scrambled bits are: " << scrambledBits << endl;
    return scrambledBits;
}

BitVector Ieee80211Scrambler::generateScramblingSequence(const ShortBitVector& generatorPolynomial, const ShortBitVector& seed) const
{
    BitVector scramblingSequence;
    int sequenceLength = (int)pow(2, seed.getSize()) - 1;
    ShortBitVector shiftRegisters = seed;
    for (int i = 0; i < sequenceLength; i++)
    {
        bool registerSum = false;
        for (unsigned int j = 0; j < generatorPolynomial.getSize(); j++)
        {
            if (generatorPolynomial.getBit(j))
                registerSum = eXOR(shiftRegisters.getBit(j), registerSum);
        }
        shiftRegisters.leftShift(1);
        shiftRegisters.setBit(0, registerSum);
        scramblingSequence.appendBit(registerSum);
    }
    return scramblingSequence;
}

} /* namespace physicallayer */
} /* namespace inet */

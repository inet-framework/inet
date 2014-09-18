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

#include "inet/physicallayer/ieee80211/Ieee80211Interleaver.h"

namespace inet {
namespace physicallayer {

Define_Module(Ieee80211Interleaver);

void Ieee80211Interleaver::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        numberOfCodedBitsPerSymbol = par("numberOfCodedBitsPerSymbol");
        numberOfCodedBitsPerSubcarrier = par("numberOfCodedBitsPerSubcarrier");
        s = std::max(numberOfCodedBitsPerSubcarrier / 2, 1);
    }
}

BitVector Ieee80211Interleaver::interleaving(const BitVector deinterleavedBits) const
{
    if (deinterleavedBits.getSize() % numberOfCodedBitsPerSymbol)
        throw cRuntimeError("deinterleavedBits length must be a multiple of numberOfCodedBitsPerSymbol = %d", numberOfCodedBitsPerSymbol);
    int numberOfSymbols = deinterleavedBits.getSize() / numberOfCodedBitsPerSymbol;
    EV_DETAIL << "Interleaving the following bits: " << deinterleavedBits << endl;
    BitVector interleavedBits;
    for (int i = 0; i < numberOfSymbols; i++)
    {
        for (int j = 0; j < numberOfCodedBitsPerSymbol; j++)
        {
            // First permutation: (N_CBPS /16) (k mod 16) + Floor(k/16), k = 0,1,...,N_CBPS - 1.
            int firstPerm = (numberOfCodedBitsPerSymbol / 16) * (j % 16) + floor(j / 16);
            // Second permutation: j = s × Floor(i/s) + (i + N_CBPS - Floor(16 × i/N_CBPS )) mod s, i = 0,1,... N_CBPS – 1,
            // where s = max(N_BPSC / 2, 1).
            int secondPerm = s * floor(firstPerm / s) +
                    (firstPerm + numberOfCodedBitsPerSymbol - (int)floor(16 * firstPerm / numberOfCodedBitsPerSymbol)) % s;
            int shiftedSecondPerm = secondPerm + i * numberOfCodedBitsPerSymbol;
            interleavedBits.setBit(shiftedSecondPerm, deinterleavedBits.getBit(i * numberOfCodedBitsPerSymbol + j));
        }
    }
    EV_DETAIL << "The interleaved bits are: " << interleavedBits << endl;
    return interleavedBits;
}

BitVector Ieee80211Interleaver::deinterleaving(const BitVector interleavedBits) const
{
    if (interleavedBits.getSize() % numberOfCodedBitsPerSymbol)
        throw cRuntimeError("interleavedBits length must be a multiple of numberOfCodedBitsPerSymbol = %d", numberOfCodedBitsPerSymbol);
    EV_DETAIL << "Deinterleaving the following bits: " << interleavedBits << endl;
    int numberOfSymbols = interleavedBits.getSize() / numberOfCodedBitsPerSymbol;
    BitVector deinterleavedBits;
    for (int i = 0; i < numberOfSymbols; i++)
    {
        for (int j = 0; j < numberOfCodedBitsPerSymbol; j++)
        {
            // i = s × Floor(j/s) + (j + Floor(16 × j/N_CBPS )) mod s, j = 0,1,... N CBPS - 1,
            // where s = max(N_BPSC / 2, 1).
            int firstPerm = s * floor(j / s) + (j + (int)floor(16 * j/numberOfCodedBitsPerSymbol)) % s;
            // k = 16 × i - (N_CBPS - 1)Floor(16 × i/N_CBPS ), i = 0,1,... N_CBPS - 1
            int secondPerm = 16 * firstPerm - (numberOfCodedBitsPerSymbol - 1) * floor(16 * firstPerm / numberOfCodedBitsPerSymbol);
            int shiftedSecondPerm = secondPerm + i * numberOfCodedBitsPerSymbol;
            deinterleavedBits.setBit(shiftedSecondPerm, interleavedBits.getBit(i * numberOfCodedBitsPerSymbol + j));
        }
    }
    EV_DETAIL << "The deinterleaved bits are: " << deinterleavedBits << endl;
    return deinterleavedBits;
}

} /* namespace physicallayer */
} /* namespace inet */

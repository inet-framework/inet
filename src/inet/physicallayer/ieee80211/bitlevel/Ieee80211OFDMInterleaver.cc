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

#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OfdmInterleaver.h"

namespace inet {

namespace physicallayer {

std::ostream& Ieee80211OfdmInterleaver::printToStream(std::ostream& stream, int level) const
{
    stream << "Ieee80211Interleaver";
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", interleaving = " << printObjectToString(interleaving, level + 1);
    return stream;
}

BitVector Ieee80211OfdmInterleaver::interleave(const BitVector& deinterleavedBits) const
{
    if (deinterleavedBits.getSize() % numberOfCodedBitsPerSymbol)
        throw cRuntimeError("deinterleavedBits length = %d must be a multiple of numberOfCodedBitsPerSymbol = %d", deinterleavedBits.getSize(), numberOfCodedBitsPerSymbol);
    int numberOfSymbols = deinterleavedBits.getSize() / numberOfCodedBitsPerSymbol;
    EV_DEBUG << "Interleaving the following bits: " << deinterleavedBits << endl;
    BitVector interleavedBits;
    for (int i = 0; i < numberOfSymbols; i++) {
        for (int j = 0; j < numberOfCodedBitsPerSymbol; j++) {
            // First permutation: (N_CBPS /16) (k mod 16) + Floor(k/16), k = 0,1,...,N_CBPS - 1.
            int firstPerm = (numberOfCodedBitsPerSymbol / 16) * (j % 16) + floor(j / 16);
            // Second permutation: j = s × Floor(i/s) + (i + N_CBPS - Floor(16 × i/N_CBPS )) mod s, i = 0,1,... N_CBPS – 1,
            // where s = max(N_BPSC / 2, 1).
            int secondPerm = s * floor(firstPerm / s)
                + (firstPerm + numberOfCodedBitsPerSymbol - (int)floor(16 * firstPerm / numberOfCodedBitsPerSymbol)) % s;
            int shiftedSecondPerm = secondPerm + i * numberOfCodedBitsPerSymbol;
            interleavedBits.setBit(shiftedSecondPerm, deinterleavedBits.getBit(i * numberOfCodedBitsPerSymbol + j));
        }
    }
    EV_DEBUG << "The interleaved bits are: " << interleavedBits << endl;
    return interleavedBits;
}

BitVector Ieee80211OfdmInterleaver::deinterleave(const BitVector& interleavedBits) const
{
    if (interleavedBits.getSize() % numberOfCodedBitsPerSymbol)
        throw cRuntimeError("interleavedBits length must be a multiple of numberOfCodedBitsPerSymbol = %d", numberOfCodedBitsPerSymbol);
    EV_DEBUG << "Deinterleaving the following bits: " << interleavedBits << endl;
    int numberOfSymbols = interleavedBits.getSize() / numberOfCodedBitsPerSymbol;
    BitVector deinterleavedBits;
    for (int i = 0; i < numberOfSymbols; i++) {
        for (int j = 0; j < numberOfCodedBitsPerSymbol; j++) {
            // i = s × Floor(j/s) + (j + Floor(16 × j/N_CBPS )) mod s, j = 0,1,... N CBPS - 1,
            // where s = max(N_BPSC / 2, 1).
            int firstPerm = s * floor(j / s) + (j + (int)floor(16 * j / numberOfCodedBitsPerSymbol)) % s;
            // k = 16 × i - (N_CBPS - 1)Floor(16 × i/N_CBPS ), i = 0,1,... N_CBPS - 1
            int secondPerm = 16 * firstPerm - (numberOfCodedBitsPerSymbol - 1) * floor(16 * firstPerm / numberOfCodedBitsPerSymbol);
            int shiftedSecondPerm = secondPerm + i * numberOfCodedBitsPerSymbol;
            deinterleavedBits.setBit(shiftedSecondPerm, interleavedBits.getBit(i * numberOfCodedBitsPerSymbol + j));
        }
    }
    EV_DEBUG << "The deinterleaved bits are: " << deinterleavedBits << endl;
    return deinterleavedBits;
}

Ieee80211OfdmInterleaver::Ieee80211OfdmInterleaver(const Ieee80211OfdmInterleaving *interleaving) : interleaving(interleaving)
{
    numberOfCodedBitsPerSubcarrier = interleaving->getNumberOfCodedBitsPerSubcarrier();
    numberOfCodedBitsPerSymbol = interleaving->getNumberOfCodedBitsPerSymbol();
    s = std::max(numberOfCodedBitsPerSubcarrier / 2, 1);
}
} /* namespace physicallayer */
} /* namespace inet */


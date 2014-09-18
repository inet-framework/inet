//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "Demodulator.h"

namespace inet {

namespace physicallayer {

Demodulator::Demodulator() :
    preambleSymbolLength(-1),
    modulation(NULL)
{}

const IReceptionBitModel *Demodulator::demodulate(const IReceptionSymbolModel *symbolModel) const
{
//    const int codeWordLength = modulation->getCodeWordLength();
//    const int bitLength = (symbolModel->getSymbolLength() - preambleSymbolLength) * codeWordLength; // TODO: -
//    const double bitRate = symbolModel->getSymbolRate() * codeWordLength;
//    return new ReceptionBitModel(bitLength, bitRate, BitVector::UNDEF, NULL, NULL, NULL, 0, 0);
}

} // namespace physicallayer

} // namespace inet

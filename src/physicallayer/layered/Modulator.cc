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

#include "Modulator.h"

namespace inet {

namespace physicallayer {

Modulator::Modulator() :
    preambleSymbolLength(-1),
    modulation(NULL)
{}

const ITransmissionSymbolModel *Modulator::modulate(const ITransmissionBitModel *bitModel) const
{
    const int codeWordLength = modulation->getCodeWordLength();
    const int symbolLength = preambleSymbolLength + (bitModel->getBitLength() + codeWordLength - 1) / codeWordLength;
    const double symbolRate = bitModel->getBitRate() / codeWordLength;
    return new TransmissionSymbolModel(symbolLength, symbolRate, NULL, modulation);
}

} // namespace physicallayer

} // namespace inet

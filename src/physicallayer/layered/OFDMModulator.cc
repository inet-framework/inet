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

#include "OFDMModulator.h"

namespace inet {

namespace physicallayer {

OFDMModulator::OFDMModulator() :
    preambleSymbolLength(-1),
    modulationScheme(NULL)
{}

OFDMModulator::OFDMModulator(const char* modulationScheme)
{

}

const ITransmissionSymbolModel *OFDMModulator::modulate(const ITransmissionBitModel *bitModel) const
{
    const int codeWordLength = modulationScheme->getCodeWordLength();
    const int symbolLength = preambleSymbolLength + (bitModel->getBitLength() + codeWordLength - 1) / codeWordLength;
    const double symbolRate = bitModel->getBitRate() / codeWordLength;
    return new TransmissionSymbolModel(symbolLength, symbolRate, NULL, modulationScheme);
}


} // namespace physicallayer

} // namespace inet

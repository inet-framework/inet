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

#include "APSKModulator.h"
#include "QAM16Modulation.h"
#include "QAM64Modulation.h"
#include "BPSKModulation.h"
#include "QPSKModulation.h"
#include "SignalSymbolModel.h"

namespace inet {
namespace physicallayer {

void APSKModulator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        const char *modulationSchemeStr = par("modulationScheme");
        if (!strcmp("QAM-16", modulationSchemeStr))
            modulationScheme = &QAM16Modulation::singleton;
        else if (!strcmp("QAM-64", modulationSchemeStr))
            modulationScheme = &QAM64Modulation::singleton;
        else if (!strcmp("QPSK", modulationSchemeStr))
            modulationScheme = &QPSKModulation::singleton;
        else if (!strcmp("BPSK", modulationSchemeStr))
            modulationScheme = &BPSKModulation::singleton;
        else
            throw cRuntimeError("Unknown modulation scheme = %s", modulationSchemeStr);
    }
}

const ITransmissionSymbolModel *APSKModulator::modulate(const ITransmissionBitModel *bitModel) const
{
    const int codeWordLength = modulationScheme->getCodeWordLength();
    const int symbolLength = preambleSymbolLength + (bitModel->getBitLength() + codeWordLength - 1) / codeWordLength;
    const double symbolRate = bitModel->getBitRate() / codeWordLength;
    const BitVector& bits = bitModel->getBits();
    std::vector<ISymbol> *symbols = new std::vector<ISymbol>(); // FIXME: Sample model should delete it
    ShortBitVector subcarrierBits;
    for (unsigned int i = 0; i < bits.getSize(); i++)
    {
        subcarrierBits.setBit(i % codeWordLength, bits.getBit(i));
        if (i % codeWordLength == codeWordLength - 1)
        {
           const ISymbol *apskSymbol = modulationScheme->mapToConstellationDiagram(subcarrierBits);
           symbols->push_back(*apskSymbol);
        }
    }
    return new TransmissionSymbolModel(symbolLength, symbolRate, symbols, modulationScheme);
}


} // namespace physicallayer

} // namespace inet

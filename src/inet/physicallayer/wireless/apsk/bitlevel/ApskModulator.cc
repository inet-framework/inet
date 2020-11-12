//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/physicallayer/wireless/apsk/bitlevel/ApskModulator.h"

#include "inet/physicallayer/wireless/common/modulation/BpskModulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam16Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam64Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/QpskModulation.h"

namespace inet {

namespace physicallayer {

Define_Module(ApskModulator);

ApskModulator::ApskModulator() :
    modulation(nullptr)
{
}

void ApskModulator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        modulation = ApskModulationBase::findModulation(par("modulation"));
}

std::ostream& ApskModulator::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ApskModulator";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(modulation, printFieldToString(modulation, level + 1, evFlags));
    return stream;
}

const ITransmissionSymbolModel *ApskModulator::modulate(const ITransmissionBitModel *bitModel) const
{
    const BitVector *bits = bitModel->getBits();
    unsigned int codeWordSize = modulation->getCodeWordSize();
    if (bits->getSize() % codeWordSize != 0)
        throw cRuntimeError("Invalid bit length for code word size");
    ShortBitVector symbolBits;
    std::vector<const ISymbol *> *symbols = new std::vector<const ISymbol *>();
    for (unsigned int i = 0; i < bits->getSize(); i++) {
        symbolBits.setBit(i % codeWordSize, bits->getBit(i));
        if (i % codeWordSize == codeWordSize - 1)
            symbols->push_back(modulation->mapToConstellationDiagram(symbolBits));
    }
    // TODO: double symbolRate = bitModel->getBitRate() / modulation->getCodeWordSize();
    return new TransmissionSymbolModel(symbols->size(), NaN, symbols->size(), NaN, symbols, modulation, modulation);
}

} // namespace physicallayer

} // namespace inet


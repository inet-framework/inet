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

#include "inet/physicallayer/common/bitlevel/SignalBitModel.h"
#include "inet/physicallayer/apskradio/bitlevel/APSKSymbol.h"
#include "inet/physicallayer/apskradio/bitlevel/APSKDemodulator.h"
#include "inet/physicallayer/modulation/QAM16Modulation.h"
#include "inet/physicallayer/modulation/QAM64Modulation.h"
#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/modulation/QPSKModulation.h"

namespace inet {

namespace physicallayer {

Define_Module(APSKDemodulator);

APSKDemodulator::APSKDemodulator() :
    modulation(nullptr)
{
}

void APSKDemodulator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        modulation = APSKModulationBase::findModulation(par("modulation"));
}

std::ostream& APSKDemodulator::printToStream(std::ostream& stream, int level) const
{
    stream << "APSKDemodulator";
    if (level >= PRINT_LEVEL_DETAIL)
        stream << ", modulation = " << printObjectToString(modulation, level - 1);
    return stream;
}

const IReceptionBitModel *APSKDemodulator::demodulate(const IReceptionSymbolModel *symbolModel) const
{
    const std::vector<const ISymbol *> *symbols = symbolModel->getSymbols();
    BitVector *bits = new BitVector();
    for (unsigned int i = 0; i < symbols->size(); i++) {
        const APSKSymbol *symbol = dynamic_cast<const APSKSymbol *>(symbols->at(i));
        ShortBitVector symbolBits = modulation->demapToBitRepresentation(symbol);
        for (unsigned int j = 0; j < symbolBits.getSize(); j++)
            bits->appendBit(symbolBits.getBit(j));
    }
    return new ReceptionBitModel(-1, bps(NaN), -1, bps(NaN), bits);
}

} // namespace physicallayer

} // namespace inet


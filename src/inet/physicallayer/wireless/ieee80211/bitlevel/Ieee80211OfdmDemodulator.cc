//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmDemodulator.h"

#include "inet/physicallayer/wireless/apsk/bitlevel/ApskSymbol.h"
#include "inet/physicallayer/wireless/common/modulation/BpskModulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam16Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam64Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/QpskModulation.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalBitModel.h"

namespace inet {
namespace physicallayer {

Ieee80211OfdmDemodulator::Ieee80211OfdmDemodulator(const Ieee80211OfdmModulation *subcarrierModulation) :
    subcarrierModulation(subcarrierModulation)
{
}

std::ostream& Ieee80211OfdmDemodulator::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "Ieee80211OfdmDemodulator";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(subcarrierModulation, printFieldToString(subcarrierModulation, level + 1, evFlags));
    return stream;
}

BitVector Ieee80211OfdmDemodulator::demodulateSymbol(const Ieee80211OfdmSymbol *signalSymbol) const
{
    std::vector<const ApskSymbol *> apskSymbols = signalSymbol->getSubCarrierSymbols();
    const ApskModulationBase *demodulationScheme = subcarrierModulation->getSubcarrierModulation();
    BitVector field;
    for (unsigned int i = 0; i < apskSymbols.size(); i++) {
        if (!isPilotOrDcSubcarrier(i)) {
            const ApskSymbol *apskSymbol = apskSymbols.at(i);
            ShortBitVector bits = demodulationScheme->demapToBitRepresentation(apskSymbol);
            for (unsigned int j = 0; j < bits.getSize(); j++)
                field.appendBit(bits.getBit(j));
        }
    }
    EV_DEBUG << "The field symbols has been demodulated into the following bit stream: " << field << endl;
    return field;
}

const IReceptionBitModel *Ieee80211OfdmDemodulator::createBitModel(const BitVector *bitRepresentation, int signalFieldLength, bps signalFieldBitrate, int dataFieldLength, bps dataFieldBitrate) const
{
    return new ReceptionBitModel(b(signalFieldLength), signalFieldBitrate, b(dataFieldLength), dataFieldBitrate, bitRepresentation, NaN);
}

bool Ieee80211OfdmDemodulator::isPilotOrDcSubcarrier(int i) const
{
    return i == 5 || i == 19 || i == 33 || i == 47 || i == 26; // pilots are: 5,19,33,47, 26 (0+26) is a dc subcarrier
}

const IReceptionBitModel *Ieee80211OfdmDemodulator::demodulate(const IReceptionSymbolModel *symbolModel) const
{
    const std::vector<const ISymbol *> *symbols = symbolModel->getAllSymbols();
    BitVector *bitRepresentation = new BitVector();
    for (auto& symbols_i : *symbols) {
        const Ieee80211OfdmSymbol *symbol = dynamic_cast<const Ieee80211OfdmSymbol *>(symbols_i);
        BitVector bits = demodulateSymbol(symbol);
        for (unsigned int j = 0; j < bits.getSize(); j++)
            bitRepresentation->appendBit(bits.getBit(j));
    }
    return createBitModel(bitRepresentation, -1, bps(NaN), -1, bps(NaN)); // TODO
}

} // namespace physicallayer
} // namespace inet


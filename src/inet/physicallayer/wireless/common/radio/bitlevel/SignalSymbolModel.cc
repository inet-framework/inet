//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalSymbolModel.h"

namespace inet {
namespace physicallayer {

SignalSymbolModel::SignalSymbolModel(int headerSymbolLength, double headerSymbolRate, int dataSymbolLength, double dataSymbolRate, const std::vector<const ISymbol *> *symbols) :
    headerSymbolLength(headerSymbolLength),
    dataSymbolLength(dataSymbolLength),
    headerSymbolRate(headerSymbolRate),
    dataSymbolRate(dataSymbolRate),
    symbols(symbols)
{
}

SignalSymbolModel::~SignalSymbolModel()
{
    if (symbols) {
        for (auto symbol : *symbols)
            delete symbol;
        delete symbols;
    }
}

std::ostream& SignalSymbolModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "SignalSymbolModel";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(headerSymbolLength)
               << EV_FIELD(dataSymbolLength)
               << EV_FIELD(headerSymbolRate)
               << EV_FIELD(dataSymbolRate);
    return stream;
}

TransmissionSymbolModel::TransmissionSymbolModel(int headerSymbolLength, double headerSymbolRate, int dataSymbolLength, double dataSymbolRate, const std::vector<const ISymbol *> *symbols, const IModulation *headerModulation, const IModulation *dataModulation) :
    SignalSymbolModel(headerSymbolLength, headerSymbolRate, dataSymbolLength, dataSymbolRate, symbols),
    headerModulation(headerModulation),
    dataModulation(dataModulation)
{
}

ReceptionSymbolModel::ReceptionSymbolModel(int headerSymbolLength, double headerSymbolRate, int dataSymbolLength, double dataSymbolRate, const std::vector<const ISymbol *> *symbols, double symbolErrorRate) :
    SignalSymbolModel(headerSymbolLength, headerSymbolRate, dataSymbolLength, dataSymbolRate, symbols),
    symbolErrorRate(symbolErrorRate)
{
}

} // namespace physicallayer
} // namespace inet


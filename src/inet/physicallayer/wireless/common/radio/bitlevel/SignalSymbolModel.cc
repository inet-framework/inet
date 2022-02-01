//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalSymbolModel.h"

namespace inet {
namespace physicallayer {

SignalSymbolModel::SignalSymbolModel(int headerSymbolLength, double headerSymbolRate, int payloadSymbolLength, double payloadSymbolRate, const std::vector<const ISymbol *> *symbols) :
    headerSymbolLength(headerSymbolLength),
    payloadSymbolLength(payloadSymbolLength),
    headerSymbolRate(headerSymbolRate),
    payloadSymbolRate(payloadSymbolRate),
    symbols(symbols)
{
}

SignalSymbolModel::~SignalSymbolModel()
{
    delete symbols;
}

std::ostream& SignalSymbolModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "SignalSymbolModel";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(headerSymbolLength)
               << EV_FIELD(payloadSymbolLength)
               << EV_FIELD(headerSymbolRate)
               << EV_FIELD(payloadSymbolRate);
    return stream;
}

TransmissionSymbolModel::TransmissionSymbolModel(int headerSymbolLength, double headerSymbolRate, int payloadSymbolLength, double payloadSymbolRate, const std::vector<const ISymbol *> *symbols, const IModulation *headerModulation, const IModulation *payloadModulation) :
    SignalSymbolModel(headerSymbolLength, headerSymbolRate, payloadSymbolLength, payloadSymbolRate, symbols),
    headerModulation(headerModulation),
    payloadModulation(payloadModulation)
{
}

ReceptionSymbolModel::ReceptionSymbolModel(int headerSymbolLength, double headerSymbolRate, int payloadSymbolLength, double payloadSymbolRate, const std::vector<const ISymbol *> *symbols, double symbolErrorRate) :
    SignalSymbolModel(headerSymbolLength, headerSymbolRate, payloadSymbolLength, payloadSymbolRate, symbols),
    symbolErrorRate(symbolErrorRate)
{
}

} // namespace physicallayer
} // namespace inet


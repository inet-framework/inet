//
// Copyright (C) 2013 OpenSim Ltd.
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


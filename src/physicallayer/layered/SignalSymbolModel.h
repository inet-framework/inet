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

#ifndef __INET_SIGNALSYMBOLMODEL_H
#define __INET_SIGNALSYMBOLMODEL_H

#include "ISignalSymbolModel.h"
#include "IModulation.h"

namespace inet {
namespace physicallayer {

class INET_API SignalSymbolModel : public virtual ISignalSymbolModel
{
  protected:
    const int symbolLength;
    const double symbolRate;
    const std::vector<const ISymbol*> *symbols;
    const IModulation *modulation;

  public:
    SignalSymbolModel(int symbolLength, double symbolRate, const std::vector<const ISymbol*> *symbols, const IModulation *modulation) :
        symbolLength(symbolLength),
        symbolRate(symbolRate),
        symbols(symbols),
        modulation(modulation)
    {}

    virtual void printToStream(std::ostream &stream) const;
    virtual int getSymbolLength() const { return symbolLength; }
    virtual double getSymbolRate() const { return symbolRate; }
    virtual const IModulation *getModulation() const { return modulation; }
    virtual const std::vector<const ISymbol*> *getSymbols() const { return symbols; }
};

class INET_API TransmissionSymbolModel : public SignalSymbolModel, public virtual ITransmissionSymbolModel
{
  public:
    TransmissionSymbolModel(int symbolLength, double symbolRate, const std::vector<const ISymbol*> *symbols, const IModulation *modulation) :
        SignalSymbolModel(symbolLength, symbolRate, symbols, modulation)
    {}
};

class INET_API ReceptionSymbolModel : public SignalSymbolModel, public virtual IReceptionSymbolModel
{
  protected:
    const double ser;
    const double symbolErrorCount;

  public:
    ReceptionSymbolModel(int symbolLength, double symbolRate, const std::vector<const ISymbol*> *symbols, const IModulation *modulation, double ser, double symbolErrorCount) :
        SignalSymbolModel(symbolLength, symbolRate, symbols, modulation),
        ser(ser),
        symbolErrorCount(symbolErrorCount)
    {}

    virtual double getSER() const { return ser; }
    virtual int getSymbolErrorCount() const { return symbolErrorCount; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_SIGNALSYMBOLMODEL_H

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

#ifndef __INET_SIGNALSYMBOLMODEL_H
#define __INET_SIGNALSYMBOLMODEL_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalSymbolModel.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IModulation.h"

namespace inet {
namespace physicallayer {

class INET_API SignalSymbolModel : public virtual ISignalSymbolModel
{
  protected:
    const int headerSymbolLength;
    const int payloadSymbolLength;
    const double headerSymbolRate;
    const double payloadSymbolRate;
    const std::vector<const ISymbol *> *symbols;

  public:
    SignalSymbolModel(int headerSymbolLength, double headerSymbolRate, int payloadSymbolLength, double payloadSymbolRate, const std::vector<const ISymbol *> *symbols);
    virtual ~SignalSymbolModel();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual int getPayloadSymbolLength() const override { return payloadSymbolLength; }
    virtual double getPayloadSymbolRate() const override { return payloadSymbolRate; }
    virtual int getHeaderSymbolLength() const override { return headerSymbolLength; }
    virtual double getHeaderSymbolRate() const override { return headerSymbolRate; }
    virtual const std::vector<const ISymbol *> *getSymbols() const override { return symbols; }
};

class INET_API TransmissionSymbolModel : public SignalSymbolModel, public virtual ITransmissionSymbolModel
{
  protected:
    const IModulation *headerModulation;
    const IModulation *payloadModulation;

  public:
    TransmissionSymbolModel(int headerSymbolLength, double headerSymbolRate, int payloadSymbolLength, double payloadSymbolRate, const std::vector<const ISymbol *> *symbols, const IModulation *headerModulation, const IModulation *payloadModulation);

    virtual const IModulation *getHeaderModulation() const override { return headerModulation; }
    virtual const IModulation *getPayloadModulation() const override { return payloadModulation; }
};

class INET_API ReceptionSymbolModel : public SignalSymbolModel, public virtual IReceptionSymbolModel
{
  protected:
    double symbolErrorRate;

  public:
    ReceptionSymbolModel(int headerSymbolLength, double headerSymbolRate, int payloadSymbolLength, double payloadSymbolRate, const std::vector<const ISymbol *> *symbols, double symbolErrorRate);

    virtual double getSymbolErrorRate() const override { return symbolErrorRate; }
};

} // namespace physicallayer
} // namespace inet

#endif


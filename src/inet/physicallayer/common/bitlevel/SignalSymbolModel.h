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

#include "inet/physicallayer/contract/bitlevel/ISignalSymbolModel.h"
#include "inet/physicallayer/contract/packetlevel/IModulation.h"

namespace inet {
namespace physicallayer {

class INET_API SignalSymbolModel : public virtual ISignalSymbolModel
{
  protected:
    const int headerSymbolLength;
    const int dataSymbolLength;
    const double headerSymbolRate;
    const double dataSymbolRate;
    const std::vector<const ISymbol *> *symbols;

  public:
    SignalSymbolModel(int headerSymbolLength, double headerSymbolRate, int dataSymbolLength, double dataSymbolRate, const std::vector<const ISymbol *> *symbols);
    virtual ~SignalSymbolModel();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
    virtual int getDataSymbolLength() const override { return dataSymbolLength; }
    virtual double getDataSymbolRate() const override { return dataSymbolRate; }
    virtual int getHeaderSymbolLength() const override { return headerSymbolLength; }
    virtual double getHeaderSymbolRate() const override { return headerSymbolRate; }
    virtual const std::vector<const ISymbol *> *getAllSymbols() const override { return symbols; }
};

class INET_API TransmissionSymbolModel : public SignalSymbolModel, public virtual ITransmissionSymbolModel
{
  protected:
    const IModulation *headerModulation;
    const IModulation *dataModulation;

  public:
    TransmissionSymbolModel(int headerSymbolLength, double headerSymbolRate, int dataSymbolLength, double dataSymbolRate, const std::vector<const ISymbol *> *symbols, const IModulation *headerModulation, const IModulation *dataModulation);

    virtual const IModulation *getHeaderModulation() const override { return headerModulation; }
    virtual const IModulation *getDataModulation() const override { return dataModulation; }
};

class INET_API ReceptionSymbolModel : public SignalSymbolModel, public virtual IReceptionSymbolModel
{
  public:
    ReceptionSymbolModel(int headerSymbolLength, double headerSymbolRate, int dataSymbolLength, double dataSymbolRate, const std::vector<const ISymbol *> *symbols);
};

} // namespace physicallayer
} // namespace inet

#endif /* ifndef __INET_SIGNALSYMBOLMODEL_H */


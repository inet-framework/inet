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

#ifndef __INET_ISIGNALSYMBOLMODEL_H
#define __INET_ISIGNALSYMBOLMODEL_H

#include "inet/physicallayer/contract/bitlevel/ISymbol.h"
#include "inet/physicallayer/contract/packetlevel/IPrintableObject.h"
#include "inet/physicallayer/contract/packetlevel/IModulation.h"

namespace inet {

namespace physicallayer {

/**
 * This purely virtual interface provides an abstraction for different radio
 * signal models in the waveform or symbol domain.
 */
class INET_API ISignalSymbolModel : public IPrintableObject
{
  public:
    virtual int getPayloadSymbolLength() const = 0;
    virtual double getPayloadSymbolRate() const = 0;
    virtual int getHeaderSymbolLength() const = 0;
    virtual double getHeaderSymbolRate() const = 0;
    virtual const std::vector<const ISymbol *> *getSymbols() const = 0;
};

class INET_API ITransmissionSymbolModel : public virtual ISignalSymbolModel
{
  public:
    virtual const IModulation *getHeaderModulation() const = 0;
    virtual const IModulation *getPayloadModulation() const = 0;
};

class INET_API IReceptionSymbolModel : public virtual ISignalSymbolModel
{
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_ISIGNALSYMBOLMODEL_H


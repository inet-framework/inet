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

#include "ISymbol.h"
#include "IPrintableObject.h"
#include "IModulation.h"

namespace inet {
namespace physicallayer {

/**
 * This purely virtual interface provides an abstraction for different radio
 * signal models in the waveform or symbol domain.
 */
class INET_API ISignalSymbolModel : public IPrintableObject
{
  public:
    virtual int getSymbolLength() const = 0;
    virtual double getSymbolRate() const = 0;
    virtual const std::vector<const ISymbol*> *getSymbols() const = 0;
    virtual const IModulation *getModulation() const = 0;
};

class INET_API ITransmissionSymbolModel : public virtual ISignalSymbolModel
{
};

class INET_API IReceptionSymbolModel : public virtual ISignalSymbolModel
{
  public:
    /**
     * Returns the symbol error rate (probability).
     */
    virtual double getSER() const = 0;

    /**
     * Returns the actual number of erroneous symbols.
     */
    virtual int getSymbolErrorCount() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_ISIGNALSYMBOLMODEL_H

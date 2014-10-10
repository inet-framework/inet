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

#ifndef __INET_MODULATION_H
#define __INET_MODULATION_H

#include "inet/physicallayer/contract/IModulation.h"
#include "inet/common/Complex.h"
#include "inet/common/ShortBitVector.h"

namespace inet {

namespace physicallayer {

class INET_API Modulation : public IModulation
{
  protected:
    const Complex *encodingTable;
    const int codeWordLength;
    const int constellationSize;
    double normalizationFactor;

  public:
    Modulation(const Complex *encodingTable, int codeWordLength, int constellationSize, double normalizationFactor) :
        encodingTable(encodingTable),
        codeWordLength(codeWordLength),
        constellationSize(constellationSize),
        normalizationFactor(normalizationFactor)
    {}
    virtual void printToStream(std::ostream &stream) const;
    virtual int getCodeWordLength() const { return codeWordLength; }
    virtual int getConstellationSize() const { return constellationSize; }
    virtual double getNormalizationFactor() const { return normalizationFactor; }
    virtual const Complex& map(const ShortBitVector& symbol) const;

};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_MODULATION_H

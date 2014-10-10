//
// Copyright (C) 2014 OpenSim Ltd.
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

#ifndef __INET_APSKMODULATIONBASE_H_
#define __INET_APSKMODULATIONBASE_H_

#include "inet/physicallayer/contract/IAPSKModulation.h"
#include "inet/common/ShortBitVector.h"
#include "inet/physicallayer/modulation/APSKSymbol.h"

namespace inet {
namespace physicallayer {

class INET_API APSKModulationBase : public IAPSKModulation
{
    protected:
        const APSKSymbol *encodingTable;
        const int codeWordLength;
        const int constellationSize;
        double normalizationFactor;

    public:
        APSKModulationBase(const APSKSymbol *encodingTable, int codeWordLength, int constellationSize, double normalizationFactor) :
          encodingTable(encodingTable),
          codeWordLength(codeWordLength),
          constellationSize(constellationSize),
          normalizationFactor(normalizationFactor)
      {}
      virtual void printToStream(std::ostream &stream) const;
      virtual int getCodeWordLength() const { return codeWordLength; }
      virtual int getConstellationSize() const { return constellationSize; }
      virtual double getNormalizationFactor() const { return normalizationFactor; }
      virtual const APSKSymbol *mapToConstellationDiagram(const ShortBitVector& symbol) const;
      virtual ShortBitVector demapToBitRepresentation(const APSKSymbol *symbol) const;
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_APSKMODULATIONBASE_H_ */

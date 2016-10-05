//
// Copyright (C) OpenSim Ltd.
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

#ifndef __INET_IENERGYSINK_H
#define __INET_IENERGYSINK_H

#include "inet/power/contract/IEnergyGenerator.h"

namespace inet {

namespace power {

/**
 * This class is a base interface that must be implemented by energy sink
 * models to integrate with other parts of the power model. Energy generators
 * connect to an energy sink, and they notify the energy sink when their energy
 * generation changes. This interface is extended by various energy sink
 * interfaces. Actual energy sink implementations should implement one of the
 * derived interfaces.
 *
 * See the corresponding NED file for more details.
 *
 * @author Levente Meszaros
 */
class INET_API IEnergySink
{
  public:
    virtual ~IEnergySink() {}

    /**
     * Returns the number of energy generators in the range [0, +infinity).
     */
    virtual int getNumEnergyGenerators() const = 0;

    /**
     * Returns the energy generator for the provided index. This functions throws
     * an exception if the index is out of range, and it never returns nullptr.
     */
    virtual const IEnergyGenerator *getEnergyGenerator(int index) const = 0;

    /**
     * Adds a new energy generator to the energy sink. The energyGenerator
     * parameter must not be nullptr.
     */
    virtual void addEnergyGenerator(const IEnergyGenerator *energyGenerator) = 0;

    /**
     * Removes a previously added energy generator from this energy sink.
     * This functions throws an exception if the generator is not found.
     */
    virtual void removeEnergyGenerator(const IEnergyGenerator *energyGenerator) = 0;
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_IENERGYSINK_H


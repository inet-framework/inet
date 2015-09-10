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

#ifndef __INET_IENERGYSINK_H
#define __INET_IENERGYSINK_H

#include "IEnergyGenerator.h"

namespace inet {

namespace power {

/**
 * This is an interface that should be implemented by energy sink models to
 * integrate with other parts of the power model. Energy generators will connect
 * to an energy sink, and they will notify it when the amount of power they
 * generate changes.
 *
 * See the corresponding NED file for more details.
 *
 * @author Levente Meszaros
 */
class INET_API IEnergySink
{
  public:
    /**
     * The signal that is used to publish power generation changes.
     */
    static simsignal_t powerGenerationChangedSignal;

  public:
    virtual ~IEnergySink() {}

    /**
     * Returns the number of energy generators.
     */
    virtual int getNumEnergyGenerators() const = 0;

    /**
     * Returns the energy generator for the provided id.
     */
    virtual const IEnergyGenerator *getEnergyGenerator(int energyGeneratorId) const = 0;

    /**
     * Adds a new energy generator to the energy sink and returns its id.
     */
    virtual int addEnergyGenerator(const IEnergyGenerator *energyGenerator) = 0;

    /**
     * Removes a previously added energy generator from this energy sink.
     */
    virtual void removeEnergyGenerator(int energyGeneratorId) = 0;

    /**
     * Returns the current total power generation in the range [0, +infinity).
     */
    virtual W getTotalPowerGeneration() const = 0;

    /**
     * Returns the generated power for the provided energy generator in the
     * range [0, +infinity).
     */
    virtual W getPowerGeneration(int energyGeneratorId) const = 0;

    /**
     * Changes the generated power for the provided energy generator in the
     * range [0, +infinity).
     */
    virtual void setPowerGeneration(int energyGeneratorId, W generatedPower) = 0;
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_IENERGYSINK_H


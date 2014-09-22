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

#ifndef __INET_ENERGYSINKBASE_H
#define __INET_ENERGYSINKBASE_H

#include "inet/power/contract/IEnergySink.h"

namespace inet {

namespace power {

/**
 * This is an abstract base class that provides a list of energy generators
 * attached to the energy sink.
 *
 * @author Levente Meszaros
 */
class INET_API EnergySinkBase : public virtual IEnergySink
{
  protected:
    struct EnergyGeneratorEntry
    {
      public:
        /**
         * The owner energy generator.
         */
        const IEnergyGenerator *energyGenerator;

        /**
         * The current power generation.
         */
        W generatedPower;

      public:
        EnergyGeneratorEntry(const IEnergyGenerator *energyGenerator, W generatedPower) :
            energyGenerator(energyGenerator), generatedPower(generatedPower) {}
    };

    /**
     * List of currently known energy generators.
     */
    std::vector<EnergyGeneratorEntry> energyGenerators;

    /**
     * The current total power generation.
     */
    W totalGeneratedPower;

  protected:
    W computeTotalGeneratedPower();

  public:
    EnergySinkBase();

    virtual int getNumEnergyGenerators() const { return energyGenerators.size(); }

    virtual const IEnergyGenerator *getEnergyGenerator(int energyGeneratorId) const;

    virtual int addEnergyGenerator(const IEnergyGenerator *energyGenerator);

    virtual void removeEnergyGenerator(int energyGeneratorId);

    virtual W getTotalPowerGeneration() const { return totalGeneratedPower; }

    virtual W getPowerGeneration(int energyGeneratorId) const;

    virtual void setPowerGeneration(int energyGeneratorId, W generatedPower);
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_ENERGYSINKBASE_H


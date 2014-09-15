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

#ifndef __INET_POWERSINKBASE_H
#define __INET_POWERSINKBASE_H

#include "IPowerSink.h"

namespace inet {

namespace power {

/**
 * This is an abstract base class for different power sinks. It provides a list
 * of power generators attached to the power sink.
 *
 * @author Levente Meszaros
 */
class INET_API PowerSinkBase : public virtual cSimpleModule, public virtual IPowerSink
{
  protected:
    struct PowerGeneratorEntry
    {
      public:
        /**
         * The owner power generator.
         */
        IPowerGenerator *powerGenerator;

        /**
         * Current power generation.
         */
        W generatedPower;

      public:
        PowerGeneratorEntry(IPowerGenerator *powerGenerator, W generatedPower) :
            powerGenerator(powerGenerator), generatedPower(generatedPower) {}
    };

    /**
     * List of currently known power generators.
     */
    std::vector<PowerGeneratorEntry> powerGenerators;

    /**
     * Current total power generation.
     */
    W totalGeneratedPower;

  public:
    virtual int getNumPowerGenerators() { return powerGenerators.size(); }

    virtual IPowerGenerator *getPowerGenerator(int id);

    virtual int addPowerGenerator(IPowerGenerator *powerGenerator);

    virtual void removePowerGenerator(int id);

    virtual W getTotalPowerGeneration() { return totalGeneratedPower; }

    virtual void setPowerGeneration(int id, W generatedPower);
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_POWERSINKBASE_H


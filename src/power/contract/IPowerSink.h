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

#ifndef __INET_IPOWERSINK_H
#define __INET_IPOWERSINK_H

#include "IPowerGenerator.h"

namespace inet {

namespace power {

/**
 * This purely virtual interface provides an abstraction for different power
 * sinks. Power sinks consume power from multiple power generators.
 *
 * @author Levente Meszaros
 */
class INET_API IPowerSink
{
  public:
    /** @brief A signal used to publish power generation changes. */
    static simsignal_t powerGenerationChangedSignal;

  public:
    virtual ~IPowerSink() {}

    /**
     * Returns the number of power generators.
     */
    virtual int getNumPowerGenerators() = 0;

    /**
     * Returns the power generator for the provided id.
     */
    virtual IPowerGenerator *getPowerGenerator(int id) = 0;

    /**
     * Adds a new power generator to the power sink and returns its id.
     */
    virtual int addPowerGenerator(IPowerGenerator *powerGenerator) = 0;

    /**
     * Removes a previously added power generator from this power sink.
     */
    virtual void removePowerGenerator(int id) = 0;

    /**
     * Returns the current total power generation in the range [0, +infinity).
     */
    virtual W getTotalPowerGeneration() = 0;

    /**
     * Changes the generated power for the provided power generator.
     */
    virtual void setPowerGeneration(int id, W generatedPower) = 0;
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_IPOWERSINK_H


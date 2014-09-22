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

#ifndef __INET_IENERGYSOURCE_H
#define __INET_IENERGYSOURCE_H

#include "IEnergyConsumer.h"

namespace inet {

namespace power {

/**
 * This is an interface that should be implemented by energy source models to
 * integrate with other parts of the power model. Energy consumers will connect
 * to an energy source, and they notify it when their power consumption changes.
 *
 * See the corresponding NED file for more details.
 *
 * @author Levente Meszaros
 */
class INET_API IEnergySource
{
  public:
    /**
     * The signal that is used to publish power consumption changes.
     */
    static simsignal_t powerConsumptionChangedSignal;

  public:
    virtual ~IEnergySource() {}

    /**
     * Returns the number of energy consumers.
     */
    virtual int getNumEnergyConsumers() const = 0;

    /**
     * Returns the energy consumer for the provided id.
     */
    virtual const IEnergyConsumer *getEnergyConsumer(int energyConsumerId) const = 0;

    /**
     * Adds a new energy consumer to the energy source and returns its id.
     */
    virtual int addEnergyConsumer(const IEnergyConsumer *energyConsumer) = 0;

    /**
     * Removes a previously added energy consumer from this energy source.
     */
    virtual void removeEnergyConsumer(int energyConsumerId) = 0;

    /**
     * Returns the current total power consumption in the range [0, +infinity).
     */
    virtual W getTotalPowerConsumption() const = 0;

    /**
     * Returns the consumed power for the provided energy consumer in the range
     * [0, +infinity).
     */
    virtual W getPowerConsumption(int energyConsumerId) const = 0;

    /**
     * Changes the consumed power for the provided energy consumer in the range
     * [0, +infinity).
     */
    virtual void setPowerConsumption(int energyConsumerId, W consumedPower) = 0;
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_IENERGYSOURCE_H


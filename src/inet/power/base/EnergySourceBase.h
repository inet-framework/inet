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

#ifndef __INET_ENERGYSOURCEBASE_H
#define __INET_ENERGYSOURCEBASE_H

#include "inet/power/contract/IEnergySource.h"

namespace inet {

namespace power {

/**
 * This is an abstract base class that provides a list of energy consumers
 * attached to the energy source.
 *
 * @author Levente Meszaros
 */
class INET_API EnergySourceBase : public virtual IEnergySource
{
  protected:
    struct EnergyConsumerEntry
    {
      public:
        /**
         * The owner energy consumer.
         */
        const IEnergyConsumer *energyConsumer;

        /**
         * The current power consumption.
         */
        W consumedPower;

      public:
        EnergyConsumerEntry(const IEnergyConsumer *energyConsumer, W consumedPower) :
            energyConsumer(energyConsumer), consumedPower(consumedPower) {}
    };

    /**
     * List of currently known energy consumers.
     */
    std::vector<EnergyConsumerEntry> energyConsumers;

    /**
     * The current total power consumption.
     */
    W totalConsumedPower;

  protected:
    W computeTotalConsumedPower();

  public:
    EnergySourceBase();

    virtual int getNumEnergyConsumers() const { return energyConsumers.size(); }

    virtual const IEnergyConsumer *getEnergyConsumer(int energyConsumerId) const;

    virtual int addEnergyConsumer(const IEnergyConsumer *energyConsumer);

    virtual void removeEnergyConsumer(int energyConsumerId);

    virtual W getTotalPowerConsumption() const { return totalConsumedPower; }

    virtual W getPowerConsumption(int energyConsumerId) const;

    virtual void setPowerConsumption(int energyConsumerId, W consumedPower);
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_ENERGYSOURCEBASE_H


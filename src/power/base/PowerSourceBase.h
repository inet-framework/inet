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

#ifndef __INET_POWERSOURCEBASE_H
#define __INET_POWERSOURCEBASE_H

#include "IPowerSource.h"

namespace inet {

namespace power {

/**
 * This is an abstract base class for different power sources. It provides a
 * list of power consumers attached to the power source.
 *
 * @author Levente Meszaros
 */
class INET_API PowerSourceBase : public virtual cSimpleModule, public virtual IPowerSource
{
  protected:
    struct PowerConsumerEntry
    {
      public:
        /**
         * The owner power consumer.
         */
        IPowerConsumer *powerConsumer;

        /**
         * Current power consumption.
         */
        W consumedPower;

      public:
        PowerConsumerEntry(IPowerConsumer *powerConsumer, W consumedPower) :
            powerConsumer(powerConsumer), consumedPower(consumedPower) {}
    };

    /**
     * List of currently known power consumers.
     */
    std::vector<PowerConsumerEntry> powerConsumers;

    /**
     * Current total power consumption.
     */
    W totalConsumedPower;

  protected:
    W computeTotalConsumedPower();

  public:
    PowerSourceBase();

    virtual int getNumPowerConsumers() { return powerConsumers.size(); }

    virtual IPowerConsumer *getPowerConsumer(int id);

    virtual int addPowerConsumer(IPowerConsumer *powerConsumer);

    virtual void removePowerConsumer(int id);

    virtual W getTotalPowerConsumption() { return totalConsumedPower; }

    virtual W getPowerConsumption(int id);

    virtual void setPowerConsumption(int id, W consumedPower);
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_POWERSOURCEBASE_H


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

#ifndef __INET_IPOWERSOURCE_H
#define __INET_IPOWERSOURCE_H

#include "IPowerConsumer.h"

/**
 * This purely virtual interface provides an abstraction for different power sources.
 *
 * @author Levente Meszaros
 */
class INET_API IPowerSource
{
  public:
    /** @brief A signal used to publish power consumption changes. */
    static simsignal_t powerConsumptionChangedSignal;

  public:
    virtual ~IPowerSource() {}

    /**
     * Returns the number of power consumers.
     */
    virtual int getNumPowerConsumers() = 0;

    /**
     * Returns the power consumer for the provided index.
     */
    virtual IPowerConsumer *getPowerConsumer(int index) = 0;

    /**
     * Adds a new power consumer to the power source and returns its id.
     */
    virtual int addPowerConsumer(IPowerConsumer *powerConsumer) = 0;

    /**
     * Removes a previously added power consumer from this power source.
     */
    virtual void removePowerConsumer(int id) = 0;

    /**
     * Returns the nominal capacity [J] in the range [0, +infinity].
     */
    virtual double getNominalCapacity() = 0;

    /**
     * Returns the residual capacity [J] in the range [0, +infinity].
     */
    virtual double getResidualCapacity() = 0;

    /**
     * Returns the current total power consumption [W] in the range [0, +infinity).
     */
    virtual double getTotalPowerConsumption() = 0;

    /**
     * Changes the consumed power [W] for the provided consumer.
     */
    virtual void setPowerConsumption(int id, double consumedPower) = 0;
};

#endif

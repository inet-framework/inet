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

#ifndef __INET_BATTERY_H
#define __INET_BATTERY_H

#include "PowerSourceBase.h"

/**
 * This class implements a voltage regulated battery.
 *
 * @author Levente Meszaros
 */
class INET_API Battery : public PowerSourceBase
{
  private:
    /**
     * Specifies if the node should be crashed when the battery becomes depleted.
     */
    bool crashNodeWhenDepleted;

    /**
     * Nominal capacity [J].
     */
    double nominalCapacity;

    /**
     * Residual capacity [J].
     */
    double residualCapacity;

    /**
     * Nominal regulated voltage [V].
     */
    double nominalVoltage;

    /**
     * Internal resistance of the battery [Ω].
     */
    double internalResistance;

    /**
     * Last simulation time when residual capacity was updated.
     */
    simtime_t lastResidualCapacityUpdate;

    /**
     * Timer that is scheduled to the time when the battery will be depleted.
     */
    cMessage *depletedTimer;

  public:
    Battery();

    virtual ~Battery();

    virtual double getNominalCapacity() { return nominalCapacity; }

    virtual double getResidualCapacity() { updateResidualCapacity(); return residualCapacity; }

    virtual double getNominalVoltage() { return nominalVoltage; }

    virtual double getCurrentVoltage() { updateResidualCapacity(); ASSERT(false); return nominalVoltage; }

    virtual void setPowerConsumption(int id, double consumedPower);

  protected:
    virtual void initialize(int stage);

    virtual void handleMessage(cMessage *message);

    virtual void updateResidualCapacity();

    virtual void scheduleDepletedTimer();
};

#endif

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

#ifndef __INET_IDEALENERGYSTORAGE_H
#define __INET_IDEALENERGYSTORAGE_H

#include "inet/power/base/EnergyStorageBase.h"

namespace inet {

namespace power {

/**
 * This class implements an ideal energy storage.
 *
 * See the corresponding NED file for more details.
 *
 * @author Levente Meszaros
 */
class INET_API IdealEnergyStorage : public EnergyStorageBase
{
  protected:
    J energyBalance = J(0);
    simtime_t lastResidualCapacityUpdate;

  protected:
    virtual void initialize(int stage) override;

    void updateResidualCapacity();

  public:
    virtual J getNominalCapacity() override { return J(INFINITY); }
    virtual J getResidualCapacity() override { return J(INFINITY); }

    virtual void setPowerGeneration(int energyGeneratorId, W generatedPower) override;
    virtual void setPowerConsumption(int energyConsumerId, W consumedPower) override;
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_IDEALENERGYSTORAGE_H


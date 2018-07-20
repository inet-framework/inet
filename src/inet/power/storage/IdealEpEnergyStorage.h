//
// Copyright (C) OpenSim Ltd.
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

#ifndef __INET_IDEALEPENERGYSTORAGE_H
#define __INET_IDEALEPENERGYSTORAGE_H

#include "inet/power/base/EpEnergyStorageBase.h"

namespace inet {

namespace power {

/**
 * This class implements an ideal energy storage.
 *
 * See the corresponding NED file for more details.
 *
 * @author Levente Meszaros
 */
class INET_API IdealEpEnergyStorage : public EpEnergyStorageBase
{
  protected:
    J energyBalance = J(NaN);
    simtime_t lastEnergyBalanceUpdate = -1;

  protected:
    virtual void initialize(int stage) override;

    virtual void updateTotalPowerConsumption() override;
    virtual void updateTotalPowerGeneration() override;
    virtual void updateEnergyBalance();

  public:
    virtual J getNominalEnergyCapacity() const override { return J(INFINITY); }
    virtual J getResidualEnergyCapacity() const override { return J(INFINITY); }
    virtual J getEnergyBalance() { return energyBalance; }
};

} // namespace power

} // namespace inet

#endif // ifndef __INET_IDEALEPENERGYSTORAGE_H


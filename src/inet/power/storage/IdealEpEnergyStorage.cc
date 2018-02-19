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

#include "inet/power/storage/IdealEpEnergyStorage.h"

namespace inet {

namespace power {

Define_Module(IdealEpEnergyStorage);

void IdealEpEnergyStorage::initialize(int stage)
{
    EpEnergyStorageBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        energyBalance = J(0);
        lastEnergyBalanceUpdate = 0;
        WATCH(energyBalance);
    }
}

void IdealEpEnergyStorage::updateTotalPowerConsumption()
{
    updateEnergyBalance();
    EpEnergyStorageBase::updateTotalPowerConsumption();
}

void IdealEpEnergyStorage::updateTotalPowerGeneration()
{
    updateEnergyBalance();
    EpEnergyStorageBase::updateTotalPowerGeneration();
}

void IdealEpEnergyStorage::updateEnergyBalance()
{
    simtime_t currentSimulationTime = simTime();
    if (currentSimulationTime != lastEnergyBalanceUpdate) {
        energyBalance += s((currentSimulationTime - lastEnergyBalanceUpdate).dbl()) * (totalPowerGeneration - totalPowerConsumption);
        lastEnergyBalanceUpdate = currentSimulationTime;
        emit(residualEnergyCapacityChangedSignal, energyBalance.get());
    }
}

} // namespace power

} // namespace inet


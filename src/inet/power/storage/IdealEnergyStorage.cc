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

#include "inet/power/storage/IdealEnergyStorage.h"

namespace inet {

namespace power {

Define_Module(IdealEnergyStorage);

void IdealEnergyStorage::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        WATCH(energyBalance);
}

void IdealEnergyStorage::setPowerConsumption(int energyConsumerId, W consumedPower)
{
    Enter_Method_Silent();
    updateResidualCapacity();
    EnergySourceBase::setPowerConsumption(energyConsumerId, consumedPower);
}

void IdealEnergyStorage::setPowerGeneration(int energyGeneratorId, W generatedPower)
{
    Enter_Method_Silent();
    updateResidualCapacity();
    EnergySinkBase::setPowerGeneration(energyGeneratorId, generatedPower);
}

void IdealEnergyStorage::updateResidualCapacity()
{
    simtime_t now = simTime();
    if (now != lastResidualCapacityUpdate) {
        energyBalance += s((now - lastResidualCapacityUpdate).dbl()) * (totalGeneratedPower - totalConsumedPower);
        lastResidualCapacityUpdate = now;
        emit(residualCapacityChangedSignal, energyBalance.get());
    }
}

} // namespace power

} // namespace inet


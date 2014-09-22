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

#include "inet/power/base/EnergyStorageBase.h"

namespace inet {

namespace power {

int EnergyStorageBase::addEnergyConsumer(const IEnergyConsumer *energyConsumer)
{
    EnergySourceBase::addEnergyConsumer(energyConsumer);
    emit(powerConsumptionChangedSignal, totalConsumedPower.get());
    return energyConsumers.size() - 1;
}

void EnergyStorageBase::removeEnergyConsumer(int energyConsumerId)
{
    EnergySourceBase::removeEnergyConsumer(energyConsumerId);
    emit(powerConsumptionChangedSignal, totalConsumedPower.get());
}

void EnergyStorageBase::setPowerConsumption(int energyConsumerId, W consumedPower)
{
    EnergySourceBase::setPowerConsumption(energyConsumerId, consumedPower);
    emit(powerConsumptionChangedSignal, totalConsumedPower.get());
}

int EnergyStorageBase::addEnergyGenerator(const IEnergyGenerator *energyGenerator)
{
    EnergySinkBase::addEnergyGenerator(energyGenerator);
    emit(powerGenerationChangedSignal, totalGeneratedPower.get());
    return energyGenerators.size() - 1;
}

void EnergyStorageBase::removeEnergyGenerator(int energyGeneratorId)
{
    EnergySinkBase::removeEnergyGenerator(energyGeneratorId);
    emit(powerGenerationChangedSignal, totalGeneratedPower.get());
}

void EnergyStorageBase::setPowerGeneration(int energyGeneratorId, W generatedPower)
{
    EnergySinkBase::setPowerGeneration(energyGeneratorId, generatedPower);
    emit(powerGenerationChangedSignal, totalGeneratedPower.get());
}

} // namespace power

} // namespace inet


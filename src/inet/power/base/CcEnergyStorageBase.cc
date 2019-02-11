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

#include "inet/power/base/CcEnergyStorageBase.h"

namespace inet {

namespace power {

void CcEnergyStorageBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        totalCurrentConsumption = computeTotalCurrentConsumption();
        totalCurrentGeneration = computeTotalCurrentGeneration();
        WATCH(totalCurrentConsumption);
        WATCH(totalCurrentGeneration);
    }
}

void CcEnergyStorageBase::updateTotalCurrentConsumption()
{
    CcEnergySourceBase::updateTotalCurrentConsumption();
    emit(currentConsumptionChangedSignal, totalCurrentConsumption.get());
}

void CcEnergyStorageBase::updateTotalCurrentGeneration()
{
    CcEnergySinkBase::updateTotalCurrentGeneration();
    emit(currentGenerationChangedSignal, totalCurrentGeneration.get());
}

void CcEnergyStorageBase::addEnergyConsumer(const IEnergyConsumer *energyConsumer)
{
    Enter_Method("addEnergyConsumer");
    CcEnergySourceBase::addEnergyConsumer(energyConsumer);
}

void CcEnergyStorageBase::removeEnergyConsumer(const IEnergyConsumer *energyConsumer)
{
    Enter_Method("removeEnergyConsumer");
    CcEnergySourceBase::removeEnergyConsumer(energyConsumer);
}

void CcEnergyStorageBase::addEnergyGenerator(const IEnergyGenerator *energyGenerator)
{
    Enter_Method("addEnergyGenerator");
    CcEnergySinkBase::addEnergyGenerator(energyGenerator);
}

void CcEnergyStorageBase::removeEnergyGenerator(const IEnergyGenerator *energyGenerator)
{
    Enter_Method("removeEnergyGenerator");
    CcEnergySinkBase::removeEnergyGenerator(energyGenerator);
}

void CcEnergyStorageBase::receiveSignal(cComponent *source, simsignal_t signal, double value, cObject *details)
{
    Enter_Method_Silent("receiveSignal");
    if (signal == ICcEnergyConsumer::currentConsumptionChangedSignal)
        updateTotalCurrentConsumption();
    else if (signal == ICcEnergyGenerator::currentGenerationChangedSignal)
        updateTotalCurrentGeneration();
}

} // namespace power

} // namespace inet


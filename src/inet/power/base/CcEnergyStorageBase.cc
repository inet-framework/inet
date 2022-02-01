//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (signal == ICcEnergyConsumer::currentConsumptionChangedSignal)
        updateTotalCurrentConsumption();
    else if (signal == ICcEnergyGenerator::currentGenerationChangedSignal)
        updateTotalCurrentGeneration();
}

} // namespace power

} // namespace inet


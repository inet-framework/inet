//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/power/base/EpEnergyStorageBase.h"

namespace inet {

namespace power {

void EpEnergyStorageBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        totalPowerConsumption = computeTotalPowerConsumption();
        totalPowerGeneration = computeTotalPowerGeneration();
        WATCH(totalPowerConsumption);
        WATCH(totalPowerGeneration);
    }
}

void EpEnergyStorageBase::updateTotalPowerConsumption()
{
    EpEnergySourceBase::updateTotalPowerConsumption();
    emit(powerConsumptionChangedSignal, totalPowerConsumption.get());
}

void EpEnergyStorageBase::updateTotalPowerGeneration()
{
    EpEnergySinkBase::updateTotalPowerGeneration();
    emit(powerGenerationChangedSignal, totalPowerGeneration.get());
}

void EpEnergyStorageBase::addEnergyConsumer(const IEnergyConsumer *energyConsumer)
{
    Enter_Method("addEnergyConsumer");
    EpEnergySourceBase::addEnergyConsumer(energyConsumer);
}

void EpEnergyStorageBase::removeEnergyConsumer(const IEnergyConsumer *energyConsumer)
{
    Enter_Method("removeEnergyConsumer");
    EpEnergySourceBase::removeEnergyConsumer(energyConsumer);
}

void EpEnergyStorageBase::addEnergyGenerator(const IEnergyGenerator *energyGenerator)
{
    Enter_Method("addEnergyGenerator");
    EpEnergySinkBase::addEnergyGenerator(energyGenerator);
}

void EpEnergyStorageBase::removeEnergyGenerator(const IEnergyGenerator *energyGenerator)
{
    Enter_Method("removeEnergyGenerator");
    EpEnergySinkBase::removeEnergyGenerator(energyGenerator);
}

void EpEnergyStorageBase::receiveSignal(cComponent *source, simsignal_t signal, double value, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (signal == IEpEnergyConsumer::powerConsumptionChangedSignal)
        updateTotalPowerConsumption();
    else if (signal == IEpEnergyGenerator::powerGenerationChangedSignal)
        updateTotalPowerGeneration();
}

} // namespace power

} // namespace inet


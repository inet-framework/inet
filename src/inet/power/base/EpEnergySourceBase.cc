//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/power/base/EpEnergySourceBase.h"

namespace inet {

namespace power {

W EpEnergySourceBase::computeTotalPowerConsumption() const
{
    W totalPowerConsumption = W(0);
    for (auto energyConsumer : energyConsumers)
        totalPowerConsumption += check_and_cast<const IEpEnergyConsumer *>(energyConsumer)->getPowerConsumption();
    return totalPowerConsumption;
}

void EpEnergySourceBase::updateTotalPowerConsumption()
{
    totalPowerConsumption = computeTotalPowerConsumption();
}

void EpEnergySourceBase::addEnergyConsumer(const IEnergyConsumer *energyConsumer)
{
    EnergySourceBase::addEnergyConsumer(energyConsumer);
    auto module = const_cast<cModule *>(check_and_cast<const cModule *>(energyConsumer));
    module->subscribe(IEpEnergyConsumer::powerConsumptionChangedSignal, this);
    updateTotalPowerConsumption();
}

void EpEnergySourceBase::removeEnergyConsumer(const IEnergyConsumer *energyConsumer)
{
    EnergySourceBase::removeEnergyConsumer(energyConsumer);
    auto module = const_cast<cModule *>(check_and_cast<const cModule *>(energyConsumer));
    module->unsubscribe(IEpEnergyConsumer::powerConsumptionChangedSignal, this);
    updateTotalPowerConsumption();
}

} // namespace power

} // namespace inet


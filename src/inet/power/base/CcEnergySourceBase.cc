//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/power/base/CcEnergySourceBase.h"

namespace inet {

namespace power {

A CcEnergySourceBase::computeTotalCurrentConsumption() const
{
    A totalCurrentConsumption = A(0);
    for (auto energyConsumer : energyConsumers)
        totalCurrentConsumption += check_and_cast<const ICcEnergyConsumer *>(energyConsumer)->getCurrentConsumption();
    return totalCurrentConsumption;
}

void CcEnergySourceBase::updateTotalCurrentConsumption()
{
    totalCurrentConsumption = computeTotalCurrentConsumption();
}

void CcEnergySourceBase::addEnergyConsumer(const IEnergyConsumer *energyConsumer)
{
    EnergySourceBase::addEnergyConsumer(energyConsumer);
    auto module = const_cast<cModule *>(check_and_cast<const cModule *>(energyConsumer));
    module->subscribe(ICcEnergyConsumer::currentConsumptionChangedSignal, this);
    updateTotalCurrentConsumption();
}

void CcEnergySourceBase::removeEnergyConsumer(const IEnergyConsumer *energyConsumer)
{
    EnergySourceBase::removeEnergyConsumer(energyConsumer);
    auto module = const_cast<cModule *>(check_and_cast<const cModule *>(energyConsumer));
    module->unsubscribe(ICcEnergyConsumer::currentConsumptionChangedSignal, this);
    updateTotalCurrentConsumption();
}

} // namespace power

} // namespace inet


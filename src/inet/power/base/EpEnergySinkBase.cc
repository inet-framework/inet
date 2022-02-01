//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/power/base/EpEnergySinkBase.h"

namespace inet {

namespace power {

W EpEnergySinkBase::computeTotalPowerGeneration() const
{
    W totalPowerGeneration = W(0);
    for (auto energyGenerator : energyGenerators)
        totalPowerGeneration += check_and_cast<const IEpEnergyGenerator *>(energyGenerator)->getPowerGeneration();
    return totalPowerGeneration;
}

void EpEnergySinkBase::updateTotalPowerGeneration()
{
    totalPowerGeneration = computeTotalPowerGeneration();
}

void EpEnergySinkBase::addEnergyGenerator(const IEnergyGenerator *energyGenerator)
{
    EnergySinkBase::addEnergyGenerator(energyGenerator);
    auto module = const_cast<cModule *>(check_and_cast<const cModule *>(energyGenerator));
    module->subscribe(IEpEnergyGenerator::powerGenerationChangedSignal, this);
    updateTotalPowerGeneration();
}

void EpEnergySinkBase::removeEnergyGenerator(const IEnergyGenerator *energyGenerator)
{
    EnergySinkBase::removeEnergyGenerator(energyGenerator);
    auto module = const_cast<cModule *>(check_and_cast<const cModule *>(energyGenerator));
    module->unsubscribe(IEpEnergyGenerator::powerGenerationChangedSignal, this);
    updateTotalPowerGeneration();
}

} // namespace power

} // namespace inet


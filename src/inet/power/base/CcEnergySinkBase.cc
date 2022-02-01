//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/power/base/CcEnergySinkBase.h"

namespace inet {

namespace power {

A CcEnergySinkBase::computeTotalCurrentGeneration() const
{
    A totalCurrentGeneration = A(0);
    for (auto energyGenerator : energyGenerators)
        totalCurrentGeneration += check_and_cast<const ICcEnergyGenerator *>(energyGenerator)->getCurrentGeneration();
    return totalCurrentGeneration;
}

void CcEnergySinkBase::updateTotalCurrentGeneration()
{
    totalCurrentGeneration = computeTotalCurrentGeneration();
}

void CcEnergySinkBase::addEnergyGenerator(const IEnergyGenerator *energyGenerator)
{
    EnergySinkBase::addEnergyGenerator(energyGenerator);
    auto module = const_cast<cModule *>(check_and_cast<const cModule *>(energyGenerator));
    module->subscribe(ICcEnergyGenerator::currentGenerationChangedSignal, this);
    updateTotalCurrentGeneration();
}

void CcEnergySinkBase::removeEnergyGenerator(const IEnergyGenerator *energyGenerator)
{
    EnergySinkBase::removeEnergyGenerator(energyGenerator);
    auto module = const_cast<cModule *>(check_and_cast<const cModule *>(energyGenerator));
    module->unsubscribe(ICcEnergyGenerator::currentGenerationChangedSignal, this);
    updateTotalCurrentGeneration();
}

} // namespace power

} // namespace inet


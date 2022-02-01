//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/power/base/EnergySourceBase.h"

#include <algorithm>

#include "inet/common/stlutils.h"

namespace inet {

namespace power {

const IEnergyConsumer *EnergySourceBase::getEnergyConsumer(int index) const
{
    return energyConsumers[index];
}

void EnergySourceBase::addEnergyConsumer(const IEnergyConsumer *energyConsumer)
{
    energyConsumers.push_back(energyConsumer);
}

void EnergySourceBase::removeEnergyConsumer(const IEnergyConsumer *energyConsumer)
{
    auto it = find(energyConsumers, energyConsumer);
    if (it == energyConsumers.end())
        throw cRuntimeError("Energy consumer not found");
    else
        energyConsumers.erase(it);
}

} // namespace power

} // namespace inet


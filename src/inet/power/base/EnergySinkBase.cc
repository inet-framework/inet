//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/power/base/EnergySinkBase.h"

#include <algorithm>

#include "inet/common/stlutils.h"

namespace inet {

namespace power {

const IEnergyGenerator *EnergySinkBase::getEnergyGenerator(int index) const
{
    return energyGenerators[index];
}

void EnergySinkBase::addEnergyGenerator(const IEnergyGenerator *energyGenerator)
{
    energyGenerators.push_back(energyGenerator);
}

void EnergySinkBase::removeEnergyGenerator(const IEnergyGenerator *energyGenerator)
{
    auto it = find(energyGenerators, energyGenerator);
    if (it == energyGenerators.end())
        throw cRuntimeError("Energy generator not found");
    else
        energyGenerators.erase(it);
}

} // namespace power

} // namespace inet


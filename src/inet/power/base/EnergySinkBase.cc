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

#include "inet/power/base/EnergySinkBase.h"

namespace inet {

namespace power {

EnergySinkBase::EnergySinkBase() :
    totalGeneratedPower(W(0))
{
}

void EnergySinkBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        WATCH(totalGeneratedPower);
}

W EnergySinkBase::computeTotalGeneratedPower()
{
    W totalGeneratedPower = W(0);
    for (auto& elem : energyGenerators)
        totalGeneratedPower += (elem).generatedPower;
    return totalGeneratedPower;
}

const IEnergyGenerator *EnergySinkBase::getEnergyGenerator(int energyGeneratorId) const
{
    return energyGenerators[energyGeneratorId].energyGenerator;
}

int EnergySinkBase::addEnergyGenerator(const IEnergyGenerator *energyGenerator)
{
    energyGenerators.push_back(EnergyGeneratorEntry(energyGenerator, energyGenerator->getPowerGeneration()));
    totalGeneratedPower = computeTotalGeneratedPower();
    return energyGenerators.size() - 1;
}

void EnergySinkBase::removeEnergyGenerator(int energyGeneratorId)
{
    energyGenerators[energyGeneratorId].generatedPower = W(0);
    energyGenerators[energyGeneratorId].energyGenerator = nullptr;
    totalGeneratedPower = computeTotalGeneratedPower();
}

W EnergySinkBase::getPowerGeneration(int energyGeneratorId) const
{
    return energyGenerators[energyGeneratorId].generatedPower;
}

void EnergySinkBase::setPowerGeneration(int energyGeneratorId, W generatedPower)
{
    energyGenerators[energyGeneratorId].generatedPower = generatedPower;
    totalGeneratedPower = computeTotalGeneratedPower();
}

} // namespace power

} // namespace inet


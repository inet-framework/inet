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


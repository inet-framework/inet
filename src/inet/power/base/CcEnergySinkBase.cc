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


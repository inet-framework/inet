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


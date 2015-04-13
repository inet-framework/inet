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

#include "inet/power/base/EnergySourceBase.h"

namespace inet {

namespace power {

EnergySourceBase::EnergySourceBase() :
    totalConsumedPower(W(0))
{
}

void EnergySourceBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
        WATCH(totalConsumedPower);
}

const IEnergyConsumer *EnergySourceBase::getEnergyConsumer(int energyConsumerId) const
{
    return energyConsumers[energyConsumerId].energyConsumer;
}

W EnergySourceBase::computeTotalConsumedPower()
{
    W totalConsumedPower = W(0);
    for (auto& elem : energyConsumers)
        totalConsumedPower += (elem).consumedPower;
    return totalConsumedPower;
}

int EnergySourceBase::addEnergyConsumer(const IEnergyConsumer *energyConsumer)
{
    energyConsumers.push_back(EnergyConsumerEntry(energyConsumer, energyConsumer->getPowerConsumption()));
    totalConsumedPower = computeTotalConsumedPower();
    return energyConsumers.size() - 1;
}

void EnergySourceBase::removeEnergyConsumer(int energyConsumerId)
{
    energyConsumers[energyConsumerId].consumedPower = W(0);
    energyConsumers[energyConsumerId].energyConsumer = nullptr;
    totalConsumedPower = computeTotalConsumedPower();
}

W EnergySourceBase::getPowerConsumption(int energyConsumerId) const
{
    return energyConsumers[energyConsumerId].consumedPower;
}

void EnergySourceBase::setPowerConsumption(int energyConsumerId, W consumedPower)
{
    energyConsumers[energyConsumerId].consumedPower = consumedPower;
    totalConsumedPower = computeTotalConsumedPower();
}

} // namespace power

} // namespace inet


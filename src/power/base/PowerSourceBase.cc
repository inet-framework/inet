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

#include "inet/power/base/PowerSourceBase.h"

namespace inet {

namespace power {

PowerSourceBase::PowerSourceBase() :
    totalConsumedPower(W(0))
{
}

IPowerConsumer *PowerSourceBase::getPowerConsumer(int id)
{
    return powerConsumers[id].powerConsumer;
}

W PowerSourceBase::computeTotalConsumedPower()
{
    W totalConsumedPower = W(0);
    for (std::vector<PowerConsumerEntry>::iterator it = powerConsumers.begin(); it != powerConsumers.end(); it++)
        totalConsumedPower += (*it).consumedPower;
    return totalConsumedPower;
}

int PowerSourceBase::addPowerConsumer(IPowerConsumer *powerConsumer)
{
    powerConsumers.push_back(PowerConsumerEntry(powerConsumer, powerConsumer->getPowerConsumption()));
    totalConsumedPower = computeTotalConsumedPower();
    emit(powerConsumptionChangedSignal, totalConsumedPower.get());
    return powerConsumers.size() - 1;
}

void PowerSourceBase::removePowerConsumer(int id)
{
    powerConsumers[id].consumedPower = W(0);
    powerConsumers[id].powerConsumer = NULL;
    totalConsumedPower = computeTotalConsumedPower();
    emit(powerConsumptionChangedSignal, totalConsumedPower.get());
}

W PowerSourceBase::getPowerConsumption(int id)
{
    return powerConsumers[id].consumedPower;
}

void PowerSourceBase::setPowerConsumption(int id, W consumedPower)
{
    powerConsumers[id].consumedPower = consumedPower;
    totalConsumedPower = computeTotalConsumedPower();
    emit(powerConsumptionChangedSignal, totalConsumedPower.get());
}

} // namespace power

} // namespace inet


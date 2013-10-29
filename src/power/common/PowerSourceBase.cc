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

#include "PowerSourceBase.h"

IPowerConsumer *PowerSourceBase::getPowerConsumer(int index)
{
    return powerConsumers[index].powerConsumer;
}

int PowerSourceBase::addPowerConsumer(IPowerConsumer *powerConsumer)
{
    powerConsumers.push_back(PowerConsumerEntry(powerConsumer, 0));
    return powerConsumers.size() - 1;
}

void PowerSourceBase::removePowerConsumer(int id)
{
    totalConsumedPower -= powerConsumers[id].consumedPower;
    powerConsumers[id].consumedPower = 0;
    powerConsumers[id].powerConsumer = NULL;
}

void PowerSourceBase::setPowerConsumption(int id, double consumedPower)
{
    this->totalConsumedPower += consumedPower - powerConsumers[id].consumedPower;
    powerConsumers[id].consumedPower = consumedPower;
    emit(powerConsumptionChangedSignal, getResidualCapacity());
}

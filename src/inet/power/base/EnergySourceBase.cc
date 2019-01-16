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

#include <algorithm>

#include "inet/power/base/EnergySourceBase.h"

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
    auto it = std::find(energyConsumers.begin(), energyConsumers.end(), energyConsumer);
    if (it == energyConsumers.end())
        throw cRuntimeError("Energy consumer not found");
    else
        energyConsumers.erase(it);
}

} // namespace power

} // namespace inet


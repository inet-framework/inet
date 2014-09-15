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

#include "PowerSinkBase.h"

namespace inet {

namespace power {

IPowerGenerator *PowerSinkBase::getPowerGenerator(int id)
{
    return powerGenerators[id].powerGenerator;
}

int PowerSinkBase::addPowerGenerator(IPowerGenerator *powerGenerator)
{
    powerGenerators.push_back(PowerGeneratorEntry(powerGenerator, W(0)));
    return powerGenerators.size() - 1;
}

void PowerSinkBase::removePowerGenerator(int id)
{
    totalGeneratedPower -= powerGenerators[id].generatedPower;
    powerGenerators[id].generatedPower = W(0);
    powerGenerators[id].powerGenerator = NULL;
}

void PowerSinkBase::setPowerGeneration(int id, W generatedPower)
{
    totalGeneratedPower += generatedPower - powerGenerators[id].generatedPower;
    powerGenerators[id].generatedPower = generatedPower;
    emit(powerGenerationChangedSignal, totalGeneratedPower.get());
}

} // namespace power

} // namespace inet


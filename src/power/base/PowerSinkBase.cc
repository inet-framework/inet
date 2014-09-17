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

PowerSinkBase::PowerSinkBase() :
    totalGeneratedPower(W(0))
{
}

W PowerSinkBase::computeTotalGeneratedPower()
{
    W totalGeneratedPower = W(0);
    for (std::vector<PowerGeneratorEntry>::iterator it = powerGenerators.begin(); it != powerGenerators.end(); it++)
        totalGeneratedPower += (*it).generatedPower;
    return totalGeneratedPower;
}

IPowerGenerator *PowerSinkBase::getPowerGenerator(int id)
{
    return powerGenerators[id].powerGenerator;
}

int PowerSinkBase::addPowerGenerator(IPowerGenerator *powerGenerator)
{
    powerGenerators.push_back(PowerGeneratorEntry(powerGenerator, powerGenerator->getPowerGeneration()));
    totalGeneratedPower = computeTotalGeneratedPower();
    emit(powerGenerationChangedSignal, totalGeneratedPower.get());
    return powerGenerators.size() - 1;
}

void PowerSinkBase::removePowerGenerator(int id)
{
    powerGenerators[id].generatedPower = W(0);
    powerGenerators[id].powerGenerator = NULL;
    totalGeneratedPower = computeTotalGeneratedPower();
    emit(powerGenerationChangedSignal, totalGeneratedPower.get());
}

W PowerSinkBase::getPowerGeneration(int id)
{
    return powerGenerators[id].generatedPower;
}

void PowerSinkBase::setPowerGeneration(int id, W generatedPower)
{
    powerGenerators[id].generatedPower = generatedPower;
    totalGeneratedPower = computeTotalGeneratedPower();
    emit(powerGenerationChangedSignal, totalGeneratedPower.get());
}

} // namespace power

} // namespace inet


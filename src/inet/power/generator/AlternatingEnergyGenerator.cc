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

#include "inet/power/generator/AlternatingEnergyGenerator.h"

namespace inet {

namespace power {

Define_Module(AlternatingEnergyGenerator);

AlternatingEnergyGenerator::AlternatingEnergyGenerator() :
    isSleeping(false),
    energyGeneratorId(-1),
    energySink(nullptr),
    powerGeneration(W(NaN)),
    timer(nullptr)
{
}

AlternatingEnergyGenerator::~AlternatingEnergyGenerator()
{
    cancelAndDelete(timer);
}

void AlternatingEnergyGenerator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        timer = new cMessage("timer");
        const char *energySinkModule = par("energySinkModule");
        energySink = dynamic_cast<IEnergySink *>(getModuleByPath(energySinkModule));
        if (!energySink)
            throw cRuntimeError("Energy sink module '%s' not found", energySinkModule);
        energyGeneratorId = energySink->addEnergyGenerator(this);
        updatePowerGeneration();
        scheduleIntervalTimer();
    }
}

void AlternatingEnergyGenerator::handleMessage(cMessage *message)
{
    if (message == timer) {
        isSleeping = !isSleeping;
        updatePowerGeneration();
        scheduleIntervalTimer();
    }
    else
        throw cRuntimeError("Unknown message");
}

void AlternatingEnergyGenerator::updatePowerGeneration()
{
    powerGeneration = isSleeping ? W(0) : W(par("powerGeneration"));
    energySink->setPowerGeneration(energyGeneratorId, powerGeneration);
    emit(IEnergySink::powerGenerationChangedSignal, powerGeneration.get());
}

void AlternatingEnergyGenerator::scheduleIntervalTimer()
{
    scheduleAt(simTime() + (isSleeping ? par("sleepInterval") : par("generationInterval")), timer);
}

} // namespace power

} // namespace inet


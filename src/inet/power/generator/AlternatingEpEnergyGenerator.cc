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

#include "inet/power/generator/AlternatingEpEnergyGenerator.h"

namespace inet {

namespace power {

Define_Module(AlternatingEpEnergyGenerator);

AlternatingEpEnergyGenerator::~AlternatingEpEnergyGenerator()
{
    cancelAndDelete(timer);
}

void AlternatingEpEnergyGenerator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        const char *energySinkModule = par("energySinkModule");
        energySink = dynamic_cast<IEpEnergySink *>(getModuleByPath(energySinkModule));
        if (!energySink)
            throw cRuntimeError("Energy sink module '%s' not found", energySinkModule);
        timer = new cMessage("timer");
        updatePowerGeneration();
        scheduleIntervalTimer();
        WATCH(isSleeping);
        WATCH(powerGeneration);
    }
    else if (stage == INITSTAGE_POWER)
        energySink->addEnergyGenerator(this);
}

void AlternatingEpEnergyGenerator::handleMessage(cMessage *message)
{
    if (message == timer) {
        isSleeping = !isSleeping;
        updatePowerGeneration();
        scheduleIntervalTimer();
    }
    else
        throw cRuntimeError("Unknown message");
}

void AlternatingEpEnergyGenerator::updatePowerGeneration()
{
    powerGeneration = isSleeping ? W(0) : W(par("powerGeneration"));
    emit(IEpEnergySink::powerGenerationChangedSignal, powerGeneration.get());
    auto text = "power: " + powerGeneration.str();
    getDisplayString().setTagArg("t", 0, text.c_str());
}

void AlternatingEpEnergyGenerator::scheduleIntervalTimer()
{
    scheduleAfter((isSleeping ? par("sleepInterval") : par("generationInterval")), timer);
}

} // namespace power

} // namespace inet


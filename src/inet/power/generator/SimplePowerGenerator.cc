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

#include "SimplePowerGenerator.h"

namespace inet {

namespace power {

Define_Module(SimplePowerGenerator);

SimplePowerGenerator::SimplePowerGenerator() :
    isSleeping(false),
    powerGeneratorId(-1),
    powerSink(NULL),
    powerGeneration(W(sNaN)),
    timer(NULL)
{
}

SimplePowerGenerator::~SimplePowerGenerator()
{
    cancelAndDelete(timer);
}

void SimplePowerGenerator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        timer = new cMessage("timer");
        const char *powerSinkModule = par("powerSinkModule");
        powerSink = dynamic_cast<IPowerSink *>(getModuleByPath(powerSinkModule));
        if (!powerSink)
            throw cRuntimeError("Power sink module '%s' not found", powerSinkModule);
        powerGeneratorId = powerSink->addPowerGenerator(this);
        updatePowerGeneration();
        scheduleIntervalTimer();
    }
}

void SimplePowerGenerator::handleMessage(cMessage *message)
{
    if (message == timer) {
        isSleeping = !isSleeping;
        updatePowerGeneration();
        scheduleIntervalTimer();
    }
    else
        throw cRuntimeError("Unknown message");
}

void SimplePowerGenerator::updatePowerGeneration()
{
    powerGeneration = isSleeping ? W(0) : W(par("powerGeneration"));
    powerSink->setPowerGeneration(powerGeneratorId, powerGeneration);
    emit(IPowerSink::powerGenerationChangedSignal, powerGeneration.get());
}

void SimplePowerGenerator::scheduleIntervalTimer()
{
    scheduleAt(simTime() + (isSleeping ? par("sleepInterval") : par("generationInterval")), timer);
}

} // namespace power

} // namespace inet


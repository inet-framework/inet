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

#include "inet/power/consumer/SimplePowerConsumer.h"

namespace inet {

namespace power {

Define_Module(SimplePowerConsumer);

SimplePowerConsumer::SimplePowerConsumer() :
    isSleeping(false),
    powerConsumerId(-1),
    powerSource(NULL),
    powerConsumption(W(sNaN)),
    timer(NULL)
{
}

SimplePowerConsumer::~SimplePowerConsumer()
{
    cancelAndDelete(timer);
}

void SimplePowerConsumer::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        timer = new cMessage("timer");
        const char *powerSourceModule = par("powerSourceModule");
        powerSource = dynamic_cast<IPowerSource *>(getModuleByPath(powerSourceModule));
        if (!powerSource)
            throw cRuntimeError("Power source module '%s' not found", powerSourceModule);
        powerConsumerId = powerSource->addPowerConsumer(this);
        updatePowerConsumption();
        scheduleIntervalTimer();
    }
}

void SimplePowerConsumer::handleMessage(cMessage *message)
{
    if (message == timer) {
        isSleeping = !isSleeping;
        updatePowerConsumption();
        scheduleIntervalTimer();
    }
    else
        throw cRuntimeError("Unknown message");
}

void SimplePowerConsumer::updatePowerConsumption()
{
    powerConsumption = isSleeping ? W(0) : W(par("powerConsumption"));
    powerSource->setPowerConsumption(powerConsumerId, powerConsumption);
    emit(IPowerSource::powerConsumptionChangedSignal, powerConsumption.get());
}

void SimplePowerConsumer::scheduleIntervalTimer()
{
    scheduleAt(simTime() + (isSleeping ? par("sleepInterval") : par("consumptionInterval")), timer);
}

} // namespace power

} // namespace inet


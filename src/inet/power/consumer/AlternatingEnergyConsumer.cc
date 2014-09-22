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

#include "inet/power/consumer/AlternatingEnergyConsumer.h"

namespace inet {

namespace power {

Define_Module(AlternatingEnergyConsumer);

AlternatingEnergyConsumer::AlternatingEnergyConsumer() :
    isSleeping(false),
    energyConsumerId(-1),
    energySource(NULL),
    powerConsumption(W(sNaN)),
    timer(NULL)
{
}

AlternatingEnergyConsumer::~AlternatingEnergyConsumer()
{
    cancelAndDelete(timer);
}

void AlternatingEnergyConsumer::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        timer = new cMessage("timer");
        const char *energySourceModule = par("energySourceModule");
        energySource = dynamic_cast<IEnergySource *>(getModuleByPath(energySourceModule));
        if (!energySource)
            throw cRuntimeError("Energy source module '%s' not found", energySourceModule);
        energyConsumerId = energySource->addEnergyConsumer(this);
        updatePowerConsumption();
        scheduleIntervalTimer();
    }
}

void AlternatingEnergyConsumer::handleMessage(cMessage *message)
{
    if (message == timer) {
        isSleeping = !isSleeping;
        updatePowerConsumption();
        scheduleIntervalTimer();
    }
    else
        throw cRuntimeError("Unknown message");
}

void AlternatingEnergyConsumer::updatePowerConsumption()
{
    powerConsumption = isSleeping ? W(0) : W(par("powerConsumption"));
    energySource->setPowerConsumption(energyConsumerId, powerConsumption);
    emit(IEnergySource::powerConsumptionChangedSignal, powerConsumption.get());
}

void AlternatingEnergyConsumer::scheduleIntervalTimer()
{
    scheduleAt(simTime() + (isSleeping ? par("sleepInterval") : par("consumptionInterval")), timer);
}

} // namespace power

} // namespace inet


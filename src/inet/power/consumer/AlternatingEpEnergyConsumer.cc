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

#include "inet/power/consumer/AlternatingEpEnergyConsumer.h"

namespace inet {

namespace power {

Define_Module(AlternatingEpEnergyConsumer);

AlternatingEpEnergyConsumer::~AlternatingEpEnergyConsumer()
{
    cancelAndDelete(timer);
}

void AlternatingEpEnergyConsumer::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        const char *energySourceModule = par("energySourceModule");
        energySource = dynamic_cast<IEpEnergySource *>(getModuleByPath(energySourceModule));
        if (!energySource)
            throw cRuntimeError("Energy source module '%s' not found", energySourceModule);
        timer = new cMessage("timer");
        updatePowerConsumption();
        scheduleIntervalTimer();
        WATCH(isSleeping);
        WATCH(powerConsumption);
    }
    else if (stage == INITSTAGE_POWER)
        energySource->addEnergyConsumer(this);
}

void AlternatingEpEnergyConsumer::handleMessage(cMessage *message)
{
    if (message == timer) {
        isSleeping = !isSleeping;
        updatePowerConsumption();
        scheduleIntervalTimer();
    }
    else
        throw cRuntimeError("Unknown message");
}

void AlternatingEpEnergyConsumer::updatePowerConsumption()
{
    powerConsumption = isSleeping ? W(0) : W(par("powerConsumption"));
    emit(IEpEnergySource::powerConsumptionChangedSignal, powerConsumption.get());
}

void AlternatingEpEnergyConsumer::scheduleIntervalTimer()
{
    scheduleAt(simTime() + (isSleeping ? par("sleepInterval") : par("consumptionInterval")), timer);
}

} // namespace power

} // namespace inet


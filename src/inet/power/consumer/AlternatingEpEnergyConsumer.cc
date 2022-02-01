//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
    scheduleAfter((isSleeping ? par("sleepInterval") : par("consumptionInterval")), timer);
}

} // namespace power

} // namespace inet


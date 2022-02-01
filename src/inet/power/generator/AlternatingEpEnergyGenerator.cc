//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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


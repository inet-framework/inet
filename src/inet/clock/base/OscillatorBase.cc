//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/clock/base/OscillatorBase.h"

#include "inet/common/IPrintableObject.h"

simsignal_t OscillatorBase::driftRateChangedSignal = cComponent::registerSignal("driftRateChanged");
namespace inet {

void OscillatorBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        if (par("emitNumTicksSignal")) {
            tickTimer = new cMessage("OscillatorTickTimer");
            tickTimer->setSchedulingPriority(par("oscillatorTickEventSchedulingPriority"));
        }
    }
    else if (stage == INITSTAGE_CLOCK) {
        if (tickTimer != nullptr) {
            numTicks = 0;
            EV_DEBUG << "Initialized oscillator" << EV_FIELD(numTicks) << EV_ENDL;
            emit(numTicksChangedSignal, numTicks);
            scheduleTickTimer();
        }
    }
}

void OscillatorBase::handleMessage(cMessage *msg)
{
    if (msg == tickTimer)
        handleTickTimer();
    else
        throw cRuntimeError("Unknown message");
}

void OscillatorBase::handleTickTimer()
{
    numTicks++;
    EV_DEBUG << "Handling oscillator tick event" << EV_FIELD(numTicks) << EV_ENDL;
    emit(numTicksChangedSignal, numTicks);
    scheduleTickTimer();
}

std::string OscillatorBase::resolveDirective(char directive) const
{
    switch (directive) {
        case 'n':
            return getNominalTickLength().str() + " s";
        case 'o':
            return getComputationOrigin().str() + " s";
        default:
            return SimpleModule::resolveDirective(directive);
    }
}

} // namespace inet


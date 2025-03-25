//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/clock/base/OscillatorBase.h"

#include "inet/common/IPrintableObject.h"

namespace inet {

simsignal_t OscillatorBase::driftRateChangedSignal = cComponent::registerSignal("driftRateChanged");

void OscillatorBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        displayStringTextFormat = par("displayStringTextFormat");
        if (par("emitNumTicksSignal")) {
            tickTimer = new cMessage("TickTimer");
            scheduleAfter(0, tickTimer);
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
    EV_INFO << "Handling tick" << EV_FIELD(numTicks) << EV_ENDL;
    emit(numTicksChangedSignal, numTicks);
    scheduleTickTimer();
    numTicks++;
}

void OscillatorBase::refreshDisplay() const
{
    if (getEnvir()->isGUI()) {
        auto text = StringFormat::formatString(displayStringTextFormat, this);
        getDisplayString().setTagArg("t", 0, text.c_str());
    }
}

std::string OscillatorBase::resolveDirective(char directive) const
{
    switch (directive) {
        case 'n':
            return getNominalTickLength().str() + " s";
        case 'o':
            return getComputationOrigin().str() + " s";
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
}

} // namespace inet


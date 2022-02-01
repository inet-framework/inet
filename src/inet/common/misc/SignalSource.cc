//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/misc/SignalSource.h"

namespace inet {

Define_Module(SignalSource);

void SignalSource::initialize()
{
    startTime = par("startTime");
    endTime = par("endTime");
    signal = registerSignal(par("signalName"));
    scheduleAt(startTime, new cMessage("timer"));
}

void SignalSource::handleMessage(cMessage *msg)
{
    if (endTime < 0 || simTime() < endTime) {
        double value = par("value");
        emit(signal, value);
        scheduleAfter(par("interval"), msg);
    }
    else {
        delete msg;
    }
}

void SignalSource::finish()
{
}

} // namespace inet


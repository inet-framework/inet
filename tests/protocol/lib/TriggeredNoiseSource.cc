// SPDX-License-Identifier: LGPL-3.0-or-later

#include "TriggeredNoiseSource.h"

namespace inet {
namespace protocoltest {

Define_Module(TriggeredNoiseSource);

void TriggeredNoiseSource::scheduleSleepTimer()
{
    // Only an explicit trigger starts a transmission.
}

void TriggeredNoiseSource::trigger()
{
    Enter_Method("trigger");
    if (sleepTimer->isScheduled() || transmissionTimer->isScheduled())
        throw cRuntimeError("interfere: source '%s' is busy", getFullPath().c_str());
    scheduleAt(simTime(), sleepTimer);
}

void TriggeredNoiseSource::startTransmission()
{
    NoiseSource::startTransmission();
    emit(registerSignal("interferenceStarted"), 1L);
}

void TriggeredNoiseSource::endTransmission()
{
    NoiseSource::endTransmission();
    emit(registerSignal("interferenceEnded"), 1L);
}

} // namespace protocoltest
} // namespace inet

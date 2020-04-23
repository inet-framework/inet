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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/common/ModuleAccess.h"
#include "inet/queueing/gate/PeriodicGate.h"

namespace inet {
namespace queueing {

Define_Module(PeriodicGate);

void PeriodicGate::initialize(int stage)
{
    PacketGateBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        isOpen_ = par("initiallyOpen");
        const char *durationsAsString = par("durations");
        cStringTokenizer tokenizer(durationsAsString);
        while (tokenizer.hasMoreTokens())
            durations.push_back(SimTime::parse(tokenizer.nextToken()));
        changeTimer = new cMessage("ChangeTimer");
    }
    else if (stage == INITSTAGE_QUEUEING) {
        if (index < (int)durations.size())
            scheduleChangeTimer();
    }
}

void PeriodicGate::handleMessage(cMessage *message)
{
    if (message == changeTimer) {
        processChangeTimer();
        scheduleChangeTimer();
    }
    else
        throw cRuntimeError("Unknown message");
}

void PeriodicGate::scheduleChangeTimer()
{
    ASSERT(0 <= index && index < (int)durations.size());
    scheduleAt(simTime() + durations[index], changeTimer);
    index = (index + 1) % durations.size();
}

void PeriodicGate::processChangeTimer()
{
    if (isOpen_)
        close();
    else
        open();
}

} // namespace queueing
} // namespace inet


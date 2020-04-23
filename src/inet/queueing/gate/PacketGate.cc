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

#include <algorithm>
#include "inet/common/ModuleAccess.h"
#include "inet/queueing/gate/PacketGate.h"

namespace inet {
namespace queueing {

Define_Module(PacketGate);

void PacketGate::initialize(int stage)
{
    PacketGateBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        isOpen_ = par("initiallyOpen");
        openTime = par("openTime");
        closeTime = par("closeTime");
        changeTimer = new cMessage("ChangeTimer");
    }
    else if (stage == INITSTAGE_QUEUEING)
        scheduleChangeTimer();
}

void PacketGate::handleMessage(cMessage *message)
{
    if (message == changeTimer) {
        processChangeTimer();
        scheduleChangeTimer();
    }
    else
        throw cRuntimeError("Unknown message");
}

void PacketGate::scheduleChangeTimer()
{
    simtime_t changeTime;
    if (isOpen_)
        changeTime = closeTime;
    else
        changeTime = openTime;
    if (changeTime > simTime())
        scheduleAt(changeTime, changeTimer);
}

void PacketGate::processChangeTimer()
{
    if (isOpen_)
        close();
    else
        open();
}

} // namespace queueing
} // namespace inet


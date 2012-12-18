//
// Copyright (C) 2012 OpenSim Ltd
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
// @author Zoltan Bojthe
//


#include "CloudDelayerBase.h"

#include "IPv4ControlInfo.h"
#include "IPv6ControlInfo.h"

Define_Module(CloudDelayerBase);

#define SRCPAR "incomingInterfaceID"

void CloudDelayerBase::handleMessage(cMessage *msg)
{
    int srcID = -1, destID = -1;

    if (msg->hasPar(SRCPAR))
    {
        cMsgPar& p = msg->par(SRCPAR);
        srcID = p.longValue();
        delete msg->getParList().remove(&p);
    }

    cObject *ci = msg->getControlInfo();
    if (dynamic_cast<IPv4ControlInfo*>(ci))
        destID = (dynamic_cast<IPv4ControlInfo*>(ci))->getInterfaceId();
    if (dynamic_cast<IPv4RoutingDecision*>(ci))
        destID = (dynamic_cast<IPv4RoutingDecision*>(ci))->getInterfaceId();
    else if (dynamic_cast<IPv6ControlInfo*>(ci))
        destID = (dynamic_cast<IPv6ControlInfo*>(ci))->getInterfaceId();
    else
        throw cRuntimeError("Cannot determine destination interface id from packet");

    simtime_t propDelay;
    bool isDrop;
    calculateDropAndDelay(msg, srcID, destID, isDrop, propDelay);
    if (isDrop)
    {
        //TODO emit?
        EV << "Message " << msg->info() << " dropped in cloud.\n";
        delete msg;
    }
    else
    {
        //TODO emit?
        EV << "Message " << msg->info() << " delayed with " << propDelay*1000.0 << "ms in cloud.\n";
        sendDelayed(msg, propDelay, "ipOut");
    }
}

void CloudDelayerBase::calculateDropAndDelay(const cMessage *msg, int srcID, int destID, bool& outDrop, simtime_t& outDelay)
{
    outDrop = false;
    outDelay = SIMTIME_ZERO;
}


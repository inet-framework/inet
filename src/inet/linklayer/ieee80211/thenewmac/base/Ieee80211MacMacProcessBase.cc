//
// Copyright (C) 2015 OpenSim Ltd.
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

#include "Ieee80211MacMacProcessBase.h"

namespace inet {
namespace ieee80211 {

void Ieee80211MacMacProcessBase::handleMessage(cMessage* msg)
{
    sdlProcess->insertSignal(msg);
    sdlProcess->run();
}

cMessage* Ieee80211MacMacProcessBase::createSignal(const char *name, Ieee80211MacSignal *signal)
{
    cMessage *msg = new cMessage(name);
    msg->setKind(signal->getSignalType());
    msg->setControlInfo(signal);
    return msg;
}

void Ieee80211MacMacProcessBase::createSignal(cPacket* packet, Ieee80211MacSignal* signal)
{
    packet->setKind(signal->getSignalType());
    packet->setControlInfo(signal);
}

void Ieee80211MacMacProcessBase::emitResetMac()
{
    scheduleAt(simTime(), resetMac);
}

Ieee80211MacMacProcessBase::~Ieee80211MacMacProcessBase()
{
    cancelAndDelete(resetMac);
}


} /* namespace inet */
} /* namespace ieee80211 */


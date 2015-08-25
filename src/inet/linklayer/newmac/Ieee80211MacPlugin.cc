//
// Copyright (C) 2015 Andras Varga
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
// Author: Andras Varga
//

#include "Ieee80211MacPlugin.h"
#include "Ieee80211NewMac.h"

namespace inet {
namespace ieee80211 {

void Ieee80211MacPlugin::scheduleAt(simtime_t t, cMessage* msg)
{
    msg->setContextPointer(this);
    mac->scheduleAt(t, msg);
}

cMessage* Ieee80211MacPlugin::cancelEvent(cMessage* msg)
{
    ASSERT(msg->getContextPointer() == this);
    mac->cancelEvent(msg);
    return msg;
}

Ieee80211MacPlugin::~Ieee80211MacPlugin()
{
}

}

} /* namespace inet */


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

#include "MacPlugin.h"

namespace inet {
namespace ieee80211 {

void MacPlugin::scheduleAt(simtime_t t, cMessage *msg)
{
    cContextSwitcher tmp(ownerModule);
    msg->setContextPointer(this);
    ownerModule->scheduleAt(t, msg);
}

cMessage *MacPlugin::cancelEvent(cMessage *msg)
{
    cContextSwitcher tmp(ownerModule);
    ASSERT(msg->getContextPointer() == nullptr || msg->getContextPointer() == this);
    ownerModule->cancelEvent(msg);
    return msg;
}

void MacPlugin::cancelAndDelete(cMessage *msg)
{
    cContextSwitcher tmp(ownerModule);
    ASSERT(msg->getContextPointer() == nullptr || msg->getContextPointer() == this);
    ownerModule->cancelAndDelete(msg);
}

} // namespace ieee80211
} // namespace inet


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

#ifndef __INET_MACPLUGIN_H
#define __INET_MACPLUGIN_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace ieee80211 {

/**
 * Base class for classes in the MAC that are not modules themselves
 * but need to schedule timers. MacPlugin implements timers by using
 * the facilities of the "host" or "container" module that the plugin
 * is part of.
 */
class INET_API MacPlugin : public cOwnedObject
{
    protected:
        cSimpleModule *ownerModule = nullptr;

    public:
        MacPlugin(cSimpleModule *ownerModule) : ownerModule(ownerModule) {}
        virtual ~MacPlugin() {}

    public:
        virtual void handleSelfMessage(cMessage *msg) = 0;
        virtual void scheduleAt(simtime_t t, cMessage *msg);
        virtual cMessage *cancelEvent(cMessage *msg);
        virtual void cancelAndDelete(cMessage *msg);

};

} // namespace ieee80211
} // namespace inet

#endif


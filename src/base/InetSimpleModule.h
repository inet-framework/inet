//
// Copyright (C) 2004, 2008 Andras Varga
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

#ifndef __INET_INETSIMPLEMODULE_H
#define __INET_INETSIMPLEMODULE_H

#include <omnetpp.h>

class InetSimpleModule : public cSimpleModule
{
public:
    InetSimpleModule(unsigned stacksize = 0) : cSimpleModule(stacksize) {}

    /**
     * Sends a message through the gate given with its ID.
     */
    int sendSync(cMessage *msg, int gateid);

    /**
     * Sends a message through the gate given with its name and index
     * (if multiple gate).
     */
    int sendSync(cMessage *msg, const char *gatename, int gateindex=-1);

    /**
     * Sends a message through the gate given with its pointer.
     */
    int sendSync(cMessage *msg, cGate *outputgate);

    void pubHandleMessage(cMessage *msg);  // public wrapper around handleMessage()
};

#endif



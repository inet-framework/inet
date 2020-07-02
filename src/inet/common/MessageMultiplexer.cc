//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/MessageMultiplexer.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"

namespace inet {

Define_Module(MessageMultiplexer);

// TODO: optimize gate access throughout this class
// TODO: factoring out some methods could also help

void MessageMultiplexer::initialize()
{
    outGate = gate("out");
}

void MessageMultiplexer::arrived(cMessage *message, cGate *inGate, simtime_t t)
{
    outGate->deliver(message, t);
}

void MessageMultiplexer::handleMessage(cMessage *message)
{
    send(message, outGate);
}

void MessageMultiplexer::handleRegisterService(const Protocol& protocol, cGate *g, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterService");
    if (g->getType() == cGate::INPUT) {
        registerService(protocol, outGate, servicePrimitive);
    }
    else {
        int size = gateSize("in");
        for (int i = 0; i < size; i++)
            registerService(protocol, gate("in", i), servicePrimitive);
    }
}

void MessageMultiplexer::handleRegisterProtocol(const Protocol& protocol, cGate *g, ServicePrimitive servicePrimitive)
{
    Enter_Method("handleRegisterProtocol");
    if (g->getType() == cGate::INPUT) {
        registerProtocol(protocol, outGate, servicePrimitive);
    }
    else {
        int size = gateSize("in");
        for (int i = 0; i < size; i++)
            registerProtocol(protocol, gate("in", i), servicePrimitive);
    }
}

} // namespace inet


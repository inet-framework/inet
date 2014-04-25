//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// author: Zoltan Bojthe

#ifndef __INET_TCPLISTENERTHREAD
#define __INET_TCPLISTENERTHREAD

#include "INETDefs.h"

#include "ByteArrayMessage.h"
#include "ModuleAccess.h"
#include "NodeOperations.h"
#include "NodeStatus.h"
#include "TCPTunnelThreadBase.h"

/**
 * Thread for an active tunnel thread
 */
class INET_API TCPListenerThread : public TCPTunnelThreadBase
{
  public:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);

    virtual void handleSelfMessage(cMessage *msg) { throw cRuntimeError("listener socket doesn't accept self messages"); }
    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent) { throw cRuntimeError("listener socket doesn't accept incoming data packets"); }
};

#endif


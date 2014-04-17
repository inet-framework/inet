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

#ifndef __INET_TCPCLIENTTUNNELTHREAD
#define __INET_TCPCLIENTTUNNELTHREAD

#include <platdep/sockets.h>

#include "INETDefs.h"

#include "ByteArrayMessage.h"
#include "ModuleAccess.h"
#include "NodeOperations.h"
#include "NodeStatus.h"
#include "SocketsRTScheduler.h"
#include "TCPSrvHostApp.h"
#include "TCPTunnelThreadBase.h"

/**
 * Thread for an active tunnel thread
 */
class INET_API TCPActiveTunnelThread : public TCPTunnelThreadBase
{
};

/**
 * Thread for a passive tunnel thread (a listener tunnel)
 */
class INET_API TCPPassiveTunnelThread : public TCPTunnelThreadBase
{
  public:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);

    virtual void realSocketAccept(cMessage *msg);
};

#endif


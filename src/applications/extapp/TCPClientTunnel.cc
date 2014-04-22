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

#include <platdep/sockets.h>

#include "INETDefs.h"

#include "ByteArrayMessage.h"
#include "ModuleAccess.h"
#include "NodeOperations.h"
#include "NodeStatus.h"
#include "SocketsRTScheduler.h"
#include "TCPMultiThreadApp.h"
#include "TCPTunnelThreadBase.h"


/**
 * listen on a real socket on localhost, create threads for accepted real connections
 */
class INET_API TCPClientTunnel : public TCPMultiThreadCtrl
{
  protected:
    SocketsRTScheduler *rtScheduler;

  public:
    TCPClientTunnel();
    virtual ~TCPClientTunnel();
    TCPThreadBase *createNewThreadFor(cMessage *msg);

  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void handleSelfMessage(cMessage *msg) { throw cRuntimeError("this module doesn't use self messages"); };
};

//Register_Class(TCPClientTunnelThread);
Define_Module(TCPClientTunnel);

TCPClientTunnel::TCPClientTunnel() :
        rtScheduler(NULL)
{
}

TCPClientTunnel::~TCPClientTunnel()
{
}

void TCPClientTunnel::initialize(int stage)
{
    TCPMultiThreadCtrl::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        rtScheduler = check_and_cast<SocketsRTScheduler *>(simulation.getScheduler());
    }
}

TCPThreadBase *TCPClientTunnel::createNewThreadFor(cMessage *msg)
{
    TCPTunnelThreadBase *thread = check_and_cast<TCPTunnelThreadBase *>(TCPMultiThreadCtrl::createNewThreadFor(msg));
    return thread;
}


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
#include "TCPSrvHostApp.h"


class INET_API TCPServerTunnelThread : public TCPServerThreadBase
{
  protected:
    int fd;
  public:
    TCPServerTunnelThread() : fd(INVALID_SOCKET) {}
    virtual ~TCPServerTunnelThread();

    virtual void established();
    virtual void dataArrived(cMessage *msg, bool urgent);
    virtual void timerExpired(cMessage *timer);     // for processing messages from SocketsRTScheduler

    /*
     * Called when the client closes the connection. By default it closes
     * our side too, but it can be redefined to do something different.
     */
    virtual void peerClosed() { TCPServerThreadBase::peerClosed(); }

    /*
     * Called when the connection closes (successful TCP teardown). By default
     * it deletes this thread, but it can be redefined to do something different.
     */
    virtual void closed() { TCPServerThreadBase::closed(); }

    /*
     * Called when the connection breaks (TCP error). By default it deletes
     * this thread, but it can be redefined to do something different.
     */
    virtual void failure(int code) { TCPServerThreadBase::failure(code); }
};

class INET_API TCPServerTunnel : public TCPSrvHostApp
{
  protected:
    SocketsRTScheduler *rtScheduler;

  public:
    TCPServerTunnel();
    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
};

Register_Class(TCPServerTunnelThread);
Register_Class(TCPServerTunnel);

TCPServerTunnel::TCPServerTunnel()
    : rtScheduler(NULL)
{
}

void TCPServerTunnel::initialize(int stage)
{
    TCPSrvHostApp::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        rtScheduler = check_and_cast<SocketsRTScheduler *>(simulation.getScheduler());
    }
}


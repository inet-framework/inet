//
// Copyright (C) 2014 OpenSim Ltd.
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

#include <platdep/sockets.h>

#include "INETDefs.h"

#include "ByteArrayMessage.h"
#include "SocketsRTScheduler.h"
#include "TCPSocket.h"
#include "TCPThread.h"

class TCPInetListenerThread : public cOwnedObject, public TCPSocket::CallbackInterface, public ITCPAppThread
{
  protected:
    TCPSocket inetSocket;
    cModule *appModule;
  public:
    TCPInetListenerThread();
    virtual ~TCPInetListenerThread();
    virtual void init(TCPMultithreadApp *module, cGate *toTcp, cMessage *msg);
    virtual void processMessage(cMessage *msg);
    virtual int getConnectionId() const { return inetSocket.getConnectionId(); }

    virtual void acceptInetConnection(cModule *module, cMessage *msg);

    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent);
    virtual void socketEstablished(int connId, void *yourPtr);
    virtual void socketPeerClosed(int connId, void *yourPtr);
    virtual void socketClosed(int connId, void *yourPtr);
    virtual void socketFailure(int connId, void *yourPtr, int code);
    virtual void socketStatusArrived(int connId, void *yourPtr, TCPStatusInfo *status);
};


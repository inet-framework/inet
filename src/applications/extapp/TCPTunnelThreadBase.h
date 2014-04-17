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

#ifndef __INET_TCPTUNNELTHREAD
#define __INET_TCPTUNNELTHREAD

#include "INETDefs.h"

#include "TCPMultiThreadApp.h"

class ByteArrayMessage;

class INET_API TCPTunnelThreadBase : public TCPThreadBase
{
  protected:
    int realSocket;

  public:
    TCPTunnelThreadBase();
    virtual ~TCPTunnelThreadBase();

    virtual void socketEstablished(int connId, void *yourPtr);
    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent);
    virtual void handleSelfMessage(cMessage *timer); // for processing messages from SocketsRTScheduler

    /*
     * Called when the client closes the connection. By default it closes
     * our side too, but it can be redefined to do something different.
     */
    virtual void socketPeerClosed(int connId, void *yourPtr);

    /*
     * Called when the connection closes (successful TCP teardown). By default
     * it deletes this thread, but it can be redefined to do something different.
     */
    virtual void socketClosed(int connId, void *yourPtr);

    /*
     * Called when the connection breaks (TCP error). By default it deletes
     * this thread, but it can be redefined to do something different.
     */
    virtual void socketFailure(int connId, void *yourPtr, int code);

    virtual void realDataArrived(ByteArrayMessage *msg);
    virtual void realSocketClosed(cMessage *msg);
    virtual void realSocketAccept(cMessage *msg);
};

#endif // __INET_TCPTUNNELTHREAD


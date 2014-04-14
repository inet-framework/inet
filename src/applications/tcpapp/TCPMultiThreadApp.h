//
// Copyright (C) 2004 Andras Varga
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

#ifndef __INET_TCPMULTITHREADAPP_H
#define __INET_TCPMULTITHREADAPP_H

#include "INETDefs.h"
#include "TCPSocket.h"
#include "TCPSocketMap.h"
#include "ILifecycle.h"
#include "LifecycleOperation.h"

//forward declaration:
class TCPThreadBase;

/**
 * Hosts a server application, to be subclassed from TCPServerProcess (which
 * is a sSimpleModule). Creates one instance (using dynamic module creation)
 * for each incoming connection. More info in the corresponding NED file.
 */
class INET_API TCPMultiThreadApp : public cSimpleModule, public ILifecycle
{
  protected:
    typedef std::map<int, TCPThreadBase*> TCPThreadMap;
    TCPThreadMap threadMap;

    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
    virtual void updateDisplay();

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);
    virtual TCPThreadBase *findThreadFor(cMessage *msg);

  public:
    virtual void removeThread(TCPThreadBase *thread);
};

/**
 * Abstract base class for server processes to be used with TCPMultiThreadApp.
 * Subclasses need to be registered using the Register_Class() macro.
 *
 * @see TCPMultiThreadApp
 */
class INET_API TCPThreadBase : public cSimpleModule, public TCPSocket::CallbackInterface
{
  protected:
    TCPMultiThreadApp *hostmod;
    TCPSocket *sock; // ptr into socketMap managed by TCPMultiThreadApp

    // internal: TCPSocket::CallbackInterface methods
    virtual void socketDataArrived(int, void *, cPacket *msg, bool urgent) { dataArrived(msg, urgent); }
    virtual void socketEstablished(int, void *) { established(); }
    virtual void socketPeerClosed(int, void *) { peerClosed(); }
    virtual void socketClosed(int, void *) { closed(); }
    virtual void socketFailure(int, void *, int code) { failure(code); }
    virtual void socketStatusArrived(int, void *, TCPStatusInfo *status) { statusArrived(status); }

  public:
    TCPThreadBase() { hostmod = NULL; sock = NULL; }
    virtual ~TCPThreadBase() {}

    // internal: called by TCPMultiThreadApp after creating this module
    virtual void init(TCPMultiThreadApp *hostmodule, TCPSocket *socket) { hostmod = hostmodule; sock = socket; }

    /*
     * Returns the socket object
     */
    virtual TCPSocket *getSocket() { return sock; }

    /*
     * Returns pointer to the host module
     */
    virtual TCPMultiThreadApp *getHostModule() { return hostmod; }

    /**
     * Called when connection is established. To be redefined.
     */
    virtual void established() = 0;

    /*
     * Called when a data packet arrives. To be redefined.
     */
    virtual void dataArrived(cMessage *msg, bool urgent) = 0;

    /*
     * Called when a timer (scheduled via scheduleAt()) expires. To be redefined.
     */
    virtual void timerExpired(cMessage *timer) = 0;

    /*
     * Called when the client closes the connection. By default it closes
     * our side too, but it can be redefined to do something different.
     */
    virtual void peerClosed() { getSocket()->close(); }

    /*
     * Called when the connection closes (successful TCP teardown). By default
     * it deletes this thread, but it can be redefined to do something different.
     */
    virtual void closed() { hostmod->removeThread(this); }

    /*
     * Called when the connection breaks (TCP error). By default it deletes
     * this thread, but it can be redefined to do something different.
     */
    virtual void failure(int code) { hostmod->removeThread(this); }

    /*
     * Called when a status arrives in response to getSocket()->getStatus().
     * By default it deletes the status object, redefine it to add code
     * to examine the status.
     */
    virtual void statusArrived(TCPStatusInfo *status) { delete status; }
};

#endif

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
 * is a cSimpleModule). Creates one instance (using dynamic module creation)
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
    virtual void handleSelfMessage(cMessage *msg) = 0;

  public:
    virtual TCPThreadBase *createNewThreadFor(cMessage *msg);
    virtual void removeThread(TCPThreadBase *thread);
};

/**
 * Abstract base class for processes to be used with TCPMultiThreadApp.
 * Subclasses need to be registered using the Register_Class() macro.
 *
 * @see TCPMultiThreadApp
 */
class INET_API TCPThreadBase : public cSimpleModule, public TCPSocket::CallbackInterface
{
  protected:
    TCPMultiThreadApp *hostmod;
    TCPSocket socket;

    // TCPSocket::CallbackInterface methods
    virtual void socketPeerClosed(int, void *) { getSocket()->close(); }
    virtual void socketClosed(int, void *) { hostmod->removeThread(this); }
    virtual void socketFailure(int, void *, int code) { hostmod->removeThread(this); }
    virtual void socketStatusArrived(int, void *, TCPStatusInfo *status) { delete status; }

  public:
    TCPThreadBase() { hostmod = NULL; }
    virtual ~TCPThreadBase() {}

    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);

    virtual void connect(Address addr, int port);

    /*
     * Returns the socket object
     */
    virtual TCPSocket *getSocket() { return &socket; }

    /*
     * Returns pointer to the host module
     */
    virtual TCPMultiThreadApp *getHostModule() { return hostmod; }

    /*
     * Called when a timer (scheduled via scheduleAt()) expires. To be redefined.
     */
    virtual void handleSelfMessage(cMessage *msg) = 0;

    /*
     * Called when a status arrives in response to getSocket()->getStatus().
     * By default it deletes the status object, redefine it to add code
     * to examine the status.
     */
    virtual void statusArrived(TCPStatusInfo *status) { delete status; }
};

#endif


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

#ifndef __INET_TCPSRVHOSTAPP_H
#define __INET_TCPSRVHOSTAPP_H

#include "inet/common/INETDefs.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "inet/transportlayer/contract/tcp/TCPSocketMap.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/LifecycleOperation.h"

namespace inet {

//forward declaration:
class TCPServerThreadBase;

/**
 * Hosts a server application, to be subclassed from TCPServerProcess (which
 * is a sSimpleModule). Creates one instance (using dynamic module creation)
 * for each incoming connection. More info in the corresponding NED file.
 */
class INET_API TCPSrvHostApp : public cSimpleModule, public ILifecycle
{
  protected:
    TCPSocket serverSocket;
    TCPSocketMap socketMap;

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual void updateDisplay();

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override
    { Enter_Method_Silent(); throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName()); return true; }

  public:
    virtual void removeThread(TCPServerThreadBase *thread);
};

/**
 * Abstract base class for server processes to be used with TCPSrvHostApp.
 * Subclasses need to be registered using the Register_Class() macro.
 *
 * @see TCPSrvHostApp
 */
class INET_API TCPServerThreadBase : public cObject, public TCPSocket::CallbackInterface
{
  protected:
    TCPSrvHostApp *hostmod;
    TCPSocket *sock;    // ptr into socketMap managed by TCPSrvHostApp

    // internal: TCPSocket::CallbackInterface methods
    virtual void socketDataArrived(int, void *, cPacket *msg, bool urgent) override { dataArrived(msg, urgent); }
    virtual void socketEstablished(int, void *) override { established(); }
    virtual void socketPeerClosed(int, void *) override { peerClosed(); }
    virtual void socketClosed(int, void *) override { closed(); }
    virtual void socketFailure(int, void *, int code) override { failure(code); }
    virtual void socketStatusArrived(int, void *, TCPStatusInfo *status) override { statusArrived(status); }

  public:

    TCPServerThreadBase() { sock = nullptr; hostmod = nullptr; }
    virtual ~TCPServerThreadBase() {}

    // internal: called by TCPSrvHostApp after creating this module
    virtual void init(TCPSrvHostApp *hostmodule, TCPSocket *socket) { hostmod = hostmodule; sock = socket; }

    /*
     * Returns the socket object
     */
    virtual TCPSocket *getSocket() { return sock; }

    /*
     * Returns pointer to the host module
     */
    virtual TCPSrvHostApp *getHostModule() { return hostmod; }

    /**
     * Schedule an event. Do not use getContextPointer() of cMessage, because
     * TCPServerThreadBase uses it for its own purposes.
     */
    virtual void scheduleAt(simtime_t t, cMessage *msg) { msg->setContextPointer(this); hostmod->scheduleAt(t, msg); }

    /*
     *  Cancel an event
     */
    virtual void cancelEvent(cMessage *msg) { hostmod->cancelEvent(msg); }

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

} // namespace inet

#endif // ifndef __INET_TCPSRVHOSTAPP_H


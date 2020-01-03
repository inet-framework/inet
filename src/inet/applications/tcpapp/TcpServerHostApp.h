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

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/socket/SocketMap.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {

//forward declaration:
class TcpServerThreadBase;

/**
 * Hosts a server application, to be subclassed from TCPServerProcess (which
 * is a sSimpleModule). Creates one instance (using dynamic module creation)
 * for each incoming connection. More info in the corresponding NED file.
 */
class INET_API TcpServerHostApp : public ApplicationBase, public TcpSocket::ICallback
{
  protected:
    TcpSocket serverSocket;
    SocketMap socketMap;
    typedef std::set<TcpServerThreadBase *> ThreadSet;
    ThreadSet threadSet;

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    virtual void socketDataArrived(TcpSocket* socket, Packet *packet, bool urgent) override { throw cRuntimeError("Unexpected data"); }
    virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) override;
    virtual void socketEstablished(TcpSocket *socket) override {}
    virtual void socketPeerClosed(TcpSocket *socket) override {}
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override {}
    virtual void socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) override { }
    virtual void socketDeleted(TcpSocket *socket) override {}

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  public:
    virtual ~TcpServerHostApp() { socketMap.deleteSockets(); }
    virtual void removeThread(TcpServerThreadBase *thread);
    virtual void threadClosed(TcpServerThreadBase *thread);

    friend class TcpServerThreadBase;
};

/**
 * Abstract base class for server processes to be used with TcpServerHostApp.
 * Subclasses need to be registered using the Register_Class() macro.
 *
 * @see TcpServerHostApp
 */
class INET_API TcpServerThreadBase : public cSimpleModule, public TcpSocket::ICallback
{
  protected:
    TcpServerHostApp *hostmod;
    TcpSocket *sock;    // ptr into socketMap managed by TcpServerHostApp

    // internal: TcpSocket::ICallback methods
    virtual void socketDataArrived(TcpSocket *socket, Packet *msg, bool urgent) override { dataArrived(msg, urgent); }
    virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) override { socket->accept(availableInfo->getNewSocketId()); }
    virtual void socketEstablished(TcpSocket *socket) override { established(); }
    virtual void socketPeerClosed(TcpSocket *socket) override { peerClosed(); }
    virtual void socketClosed(TcpSocket *socket) override { hostmod->threadClosed(this); }
    virtual void socketFailure(TcpSocket *socket, int code) override { failure(code); }
    virtual void socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) override { statusArrived(status); }
    virtual void socketDeleted(TcpSocket *socket) override { if (socket == sock) sock = nullptr; }

    virtual void refreshDisplay() const override;

  public:

    TcpServerThreadBase() { sock = nullptr; hostmod = nullptr; }
    virtual ~TcpServerThreadBase() { delete sock; }

    // internal: called by TcpServerHostApp after creating this module
    virtual void init(TcpServerHostApp *hostmodule, TcpSocket *socket) { hostmod = hostmodule; sock = socket; }

    /*
     * Returns the socket object
     */
    virtual TcpSocket *getSocket() { return sock; }

    /*
     * Returns pointer to the host module
     */
    virtual TcpServerHostApp *getHostModule() { return hostmod; }

    /**
     * Called when connection is established. To be redefined.
     */
    virtual void established() = 0;

    /*
     * Called when a data packet arrives. To be redefined.
     */
    virtual void dataArrived(Packet *msg, bool urgent) = 0;

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
     * Called when the connection breaks (TCP error). By default it deletes
     * this thread, but it can be redefined to do something different.
     */
    virtual void failure(int code) { hostmod->removeThread(this); }

    /*
     * Called when a status arrives in response to getSocket()->getStatus().
     * By default it deletes the status object, redefine it to add code
     * to examine the status.
     */
    virtual void statusArrived(TcpStatusInfo *status) { }
};

} // namespace inet

#endif // ifndef __INET_TCPSRVHOSTAPP_H


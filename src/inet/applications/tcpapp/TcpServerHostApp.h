//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPSERVERHOSTAPP_H
#define __INET_TCPSERVERHOSTAPP_H

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/IModuleInterfaceLookup.h"
#include "inet/common/socket/SocketMap.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {

using namespace inet::queueing;

class TcpServerThreadBase;

/**
 * Opens a TCP server socket, and launches one dynamically created module
 * for each incoming connection. More info in the corresponding NED file.
 */
class INET_API TcpServerHostApp : public ApplicationBase, public TcpSocket::ICallback, public IPassivePacketSink, public IModuleInterfaceLookup
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

    virtual void socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent) override { throw cRuntimeError("Unexpected data"); }
    virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) override;
    virtual void socketEstablished(TcpSocket *socket) override {}
    virtual void socketPeerClosed(TcpSocket *socket) override {}
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override {}
    virtual void socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) override {}
    virtual void socketDeleted(TcpSocket *socket) override { socketMap.removeSocket(socket); }

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  public:
    virtual ~TcpServerHostApp() { socketMap.deleteSockets(); }
    virtual void removeThread(TcpServerThreadBase *thread);
    virtual void threadClosed(TcpServerThreadBase *thread);

    virtual bool canPushSomePacket(const cGate *gate) const override { return gate->isName("socketIn"); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return gate->isName("socketIn"); }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }

    virtual cGate *lookupModuleInterface(cGate *gate, const std::type_info& type, const cObject *arguments, int direction) override;

    friend class TcpServerThreadBase;
};

/**
 * Abstract base class for server process modules to be used with TcpServerHostApp.
 *
 * @see TcpServerHostApp
 */
class INET_API TcpServerThreadBase : public cSimpleModule, public TcpSocket::ICallback
{
  protected:
    TcpServerHostApp *hostmod;
    TcpSocket *sock; // ptr into socketMap managed by TcpServerHostApp

    // internal: TcpSocket::ICallback methods
    virtual void socketDataArrived(TcpSocket *socket, Packet *msg, bool urgent) override;
    virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) override;
    virtual void socketEstablished(TcpSocket *socket) override;
    virtual void socketPeerClosed(TcpSocket *socket) override;
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override;
    virtual void socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) override;
    virtual void socketDeleted(TcpSocket *socket) override;

    virtual void refreshDisplay() const override;

  public:

    TcpServerThreadBase();
    virtual ~TcpServerThreadBase();

    // internal: called by TcpServerHostApp after creating this module
    virtual void init(TcpServerHostApp *hostmodule, TcpSocket *socket);

    virtual void close();

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
    virtual void peerClosed() { close(); }

    /*
     * Called when a status arrives in response to getSocket()->getStatus().
     * By default it deletes the status object, redefine it to add code
     * to examine the status.
     */
    virtual void statusArrived(TcpStatusInfo *status) {}
};

} // namespace inet

#endif


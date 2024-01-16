//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPSOCKET_H
#define __INET_TCPSOCKET_H

#include "inet/common/packet/ChunkQueue.h"
#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/socket/ISocket.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/contract/tcp/TcpCommand_m.h"

namespace inet {

class TcpStatusInfo;

/**
 * TcpSocket is a convenience class, to make it easier to manage TCP connections
 * from your application models. You'd have one (or more) TcpSocket object(s)
 * in your application simple module class, and call its member functions
 * (bind(), listen(), connect(), etc.) to open, close or abort a TCP connection.
 *
 * TcpSocket chooses and remembers the connId for you, assembles and sends command
 * packets (such as OPEN_ACTIVE, OPEN_PASSIVE, CLOSE, ABORT, etc.) to TCP,
 * and can also help you deal with packets and notification messages arriving
 * from TCP.
 *
 * A session which opens a connection from local port 1000 to 10.0.0.2:2000,
 * sends 16K of data and closes the connection may be as simple as this
 * (the code can be placed in your handleMessage() or activity()):
 *
 * <pre>
 *   TcpSocket socket;
 *   socket.connect(Address("10.0.0.2"), 2000);
 *
 *   Packet *packet = new Packet("data", makeShared<ByteCountChunk>(B(16*1024)));
 *   socket.send(packet);
 *
 *   socket.close();
 * </pre>
 *
 * Dealing with packets and notification messages coming from TCP is somewhat
 * more cumbersome. Basically you have two choices: you either process those
 * messages yourself, or let TcpSocket do part of the job. For the latter, you
 * give TcpSocket a callback object of the type TcpSocket::ICallback, on which
 * it will invoke the appropriate member functions: socketEstablished(),
 * socketDataArrived(), socketFailure(), socketPeerClosed(), etc. The callback
 * object can be your simple module class too.
 *
 * Note that receiving data (i.e. callbacks to socketDataArrived()) are subject
 * to the read mode. Namely, the socket can be in one of two modes: "autoread"
 * and "explicit-read". In "autoread" mode, TCP immediately forwards received
 * data to the app, while in "explicit-read" mode, the app must issue explicit
 * READ commands. The latter can be done with the read() member function. The
 * socket is created in "autoread" mode by default; "explicit-read" mode can be
 * selected by calling setAutoRead(false) before the connection is established.
 * Note that sockets created by forking off a listening socket inherit the mode
 * from the listening socket.
 *
 * Note that data arrive piecewise; the BufferingCallback utility class
 * is provided to simplify the job of assembling the data into a single message.
 *
 * This code skeleton example shows how to set up a TcpSocket to use the module
 * itself as callback object:
 *
 * <pre>
 * class MyModule : public cSimpleModule, public TcpSocket::ICallback
 * {
 *     TcpSocket socket;
 *     virtual void socketDataArrived(TcpSocket *tcpSocket, Packet *packet, bool urgent);
 *     virtual void socketFailure(int connId, int code);
 *     ...
 * };
 *
 * void MyModule::initialize() {
 *     socket.setCallback(this);
 * }
 *
 * void MyModule::handleMessage(cMessage *msg) {
 *     if (socket.belongsToSocket(msg))
 *         socket.processMessage(msg); // dispatch to socketXXX() methods
 *     else
 *         ...
 * }
 *
 * void MyModule::socketDataArrived(TcpSocket *tcpSocket, Packet *packet, bool) {
 *     EV << "Received TCP data, " << packet->getByteLength() << " bytes\\n";
 *     delete packet;
 * }
 *
 * void MyModule::socketFailure(TcpSocket *tcpSocket, int code) {
 *     if (code==TCP_I_CONNECTION_RESET)
 *         EV << "Connection reset!\\n";
 *     else if (code==TCP_I_CONNECTION_REFUSED)
 *         EV << "Connection refused!\\n";
 *     else if (code==TCP_I_TIMEOUT)
 *         EV << "Connection timed out!\\n";
 * }
 * </pre>
 *
 * If you need to manage a large number of sockets (e.g. in a server
 * application which handles multiple incoming connections), the SocketMap
 * class may be useful. The following code fragment could be used to handle
 * incoming connections:
 *
 * <pre>
 * TcpSocket *socket = check_and_cast_nullable<TcpSocket*>socketMap.findSocketFor(msg);
 * if (!socket) {
 *     // not yet in socketMap, must be new incoming connection: add to socketMap
 *     socket = new TcpSocket(msg);
 *     socket->setOutputGate(gate("tcpOut"));
 *     socket->setCallback(this);
 *     socketMap.addSocket(socket);
 * }
 *
 * // dispatch to socketEstablished(), socketDataArrived(), socketPeerClosed()
 * // or socketFailure()
 * socket->processMessage(msg);
 * </pre>
 *
 * @see SocketMap
 */
class INET_API TcpSocket : public ISocket
{
  public:
    /**
     * Callback interface for TCP sockets, see setCallback() and processMessage() for more info.
     *
     */
    // Note: This class is not subclassed from cObject, to allow module classes
    // implement it without creating diamond inheritance
    class INET_API ICallback {
      public:
        virtual ~ICallback() {}
        /**
         * Notifies about data arrival, packet ownership is transferred to the callee.
         */
        virtual void socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent) = 0;
        virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) = 0;
        virtual void socketEstablished(TcpSocket *socket) = 0;
        virtual void socketPeerClosed(TcpSocket *socket) = 0;
        virtual void socketClosed(TcpSocket *socket) = 0;
        virtual void socketFailure(TcpSocket *socket, int code) = 0;
        virtual void socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) = 0;
        virtual void socketDeleted(TcpSocket *socket) = 0;
    };

    /**
     * Utility callback class for TCP sockets. It inserts incoming data into a
     * buffer in TcpSocket, with the buffer being accessible via getReadBuffer().
     */
    class INET_API BufferingCallback : public ICallback {
      public:
        virtual void socketDataArrived(TcpSocket *socket) = 0;

        virtual void socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent) override {
            socket->getReadBuffer()->push(packet->peekData());
            delete packet;
            socketDataArrived(socket);
        }
    };

    enum State { NOT_BOUND, BOUND, LISTENING, CONNECTING, CONNECTED, PEER_CLOSED, LOCALLY_CLOSED, CLOSED, SOCKERROR };

  protected:
    int connId = -1;
    State sockstate = NOT_BOUND;

    L3Address localAddr;
    int localPrt = -1;
    L3Address remoteAddr;
    int remotePrt = -1;

    bool autoRead = true;

    ICallback *cb = nullptr;
    void *userData = nullptr;
    cGate *gateToTcp = nullptr;
    std::string tcpAlgorithmClass;

    ChunkQueue *receiveQueue = nullptr;

  protected:
    void sendToTcp(cMessage *msg, int c = -1);

    // internal: implementation behind listen() and listenOnce()
    void listen(bool fork);

  public:
    /**
     * Constructor. The getConnectionId() method returns a valid Id right after
     * constructor call.
     */
    TcpSocket();

    /**
     * Constructor, to be used with forked sockets (see listen()).
     * The new connId will be picked up from the message: it should have
     * arrived from TCP and contain TCPCommmand control info.
     */
    TcpSocket(cMessage *msg);

    /**
     * Copy constructor
     */
    TcpSocket(const TcpSocket& other);

    /**
     * Constructor, to be used with forked sockets (see listen()).
     */
    TcpSocket(TcpAvailableInfo *availableInfo);

    /**
     * Destructor
     */
    ~TcpSocket();

    /**
     * Returns the internal connection Id. TCP uses the (gate index, connId) pair
     * to identify the connection when it receives a command from the application
     * (or TcpSocket).
     */
    int getSocketId() const override { return connId; }

    /**
     * Returns the buffer where BufferingCallback collects received data.
     * The user can peek the buffer to determine whether enough data has arrived
     * that can be meaningfully processed (e.g. a whole protocol message), and
     * if so, extract the data from the buffer and process it.
     */
    ChunkQueue *getReadBuffer() { if (receiveQueue == nullptr) receiveQueue = new ChunkQueue(); return receiveQueue; }

    void *getUserData() const { return userData; }
    void setUserData(void *userData) { this->userData = userData; }

    /**
     * Returns the socket state, one of NOT_BOUND, CLOSED, LISTENING, CONNECTING,
     * CONNECTED, etc. Messages received from TCP must be routed through
     * processMessage() in order to keep socket state up-to-date.
     */
    TcpSocket::State getState() const { return sockstate; }

    /**
     * Returns name of socket state code returned by getState().
     */
    static const char *stateName(TcpSocket::State state);

    void setState(TcpSocket::State state) { sockstate = state; }

    /** @name Getter functions */
    //@{
    L3Address getLocalAddress() const { return localAddr; }
    int getLocalPort() const { return localPrt; }
    L3Address getRemoteAddress() const { return remoteAddr; }
    int getRemotePort() const { return remotePrt; }
    bool getAutoRead() { return autoRead; }
    //@}

    /**
     * Set autoRead mode on/off.
     *
     * The autoRead is on by default. In this mode, incoming data
     * is immediately forwarded by TCP connection to the socket. This turns
     * off TCP flow control because the application is capable of
     * receiving any amount of data at any time.
     *
     * When autoRead is set to off, it operates similarly to the Unix
     * socket API. In this mode, the TCP retains the received data in a
     * buffer and only sends it to the socket when the read() function
     * is called.
     *
     * Please note that this setting can only be used before calling
     * connect() or listen(). When a connection is established in a listening
     * socket, a new fork of the socket is automatically created, and the
     * connection is established with this new fork. Sockets created from
     * a fork operation of a listening socket will inherit this setting from
     * the parent listen socket.
     */
    void setAutoRead(bool autoRead);

    /** @name Opening and closing connections, sending data */
    //@{

    /**
     * Sets the gate on which to send to TCP. Must be invoked before socket
     * can be used. Example: <tt>socket.setOutputGate(gate("tcpOut"));</tt>
     */
    void setOutputGate(cGate *toTcp) { gateToTcp = toTcp; }

    /**
     * Bind the socket to a local port number.
     */
    void bind(int localPort);

    /**
     * Bind the socket to a local port number and IP address (useful with
     * multi-homing).
     */
    void bind(L3Address localAddr, int localPort);

    /**
     * Returns the current tcpAlgorithmClass parameter.
     */
    const char *getTCPAlgorithmClass() const { return tcpAlgorithmClass.c_str(); }

    /**
     * Sets the tcpAlgorithmClass parameter of the next connect() or listen() call.
     */
    void setTCPAlgorithmClass(const char *tcpAlgorithmClass) { this->tcpAlgorithmClass = tcpAlgorithmClass; }

    /**
     * Initiates passive OPEN, creating a "forking" connection that will listen
     * on the port you bound the socket to. Every incoming connection will
     * get a new connId (and thus, must be handled with a new TcpSocket object),
     * while the original connection (original connId) will keep listening on
     * the port. The new TcpSocket object must be created with the
     * TcpSocket(cMessage *msg) constructor.
     *
     * If you need to handle multiple incoming connections, the TcpSocketMap
     * class can also be useful, and TcpServerHostApp shows how to put it all
     * together. See also TcpOpenCommand documentation (neddoc) for more info.
     */
    void listen() { listen(true); }

    /**
     * Initiates passive OPEN to create a non-forking listening connection.
     * Non-forking means that TCP will accept the first incoming
     * connection, and refuse subsequent ones.
     *
     * See TcpOpenCommand documentation (neddoc) for more info.
     */
    void listenOnce() { listen(false); }

    /**
     * Accepts a new incoming connection reported as available.
     */
    void accept(int socketId);

    /**
     * Active OPEN to the given remote socket.
     */
    void connect(L3Address remoteAddr, int remotePort);

    /**
     * This function is only in use in "explicit-read" mode, i.e. when the
     * autoRead is turned off with setAutoRead(false).
     *
     * It sends a READ request message to TCP, specifying maximum amount of data
     * it wishes to read. The TCP connection module responds by sending data as
     * soon as any is available. Only one READ request can be active at a time;
     * secondary requests are rejected. This method returns immediately; data
     * will be delivered to the socket callback asynchronously.
     */
    void read(int32_t numBytes);

    /**
     * Sends data packet.
     */
    virtual void send(Packet *msg) override;

    /**
     * Sends command.
     */
    void sendCommand(Request *msg);

    /**
     * Closes the local end of the connection. With TCP, a CLOSE operation
     * means "I have no more data to send", and thus results in a one-way
     * connection until the remote TCP closes too (or the FIN_WAIT_1 timeout
     * expires)
     */
    void close() override;

    /**
     * Aborts the connection.
     */
    void abort();

    /**
     * Destroy the connection.
     */
    virtual void destroy() override;

    /**
     * Causes TCP to reply with a fresh TcpStatusInfo, attached to a dummy
     * message as getControlInfo(). The reply message can be recognized by its
     * message kind TCP_I_STATUS, or (if a callback object is used)
     * the socketStatusArrived() method of the callback object will be
     * called.
     */
    void requestStatus();

    /**
     * Set the TTL (Ipv6: Hop Limit) field on sent packets.
     */
    void setTimeToLive(int ttl);

    /**
     * Sets the Ipv4 / Ipv6 dscp fields of packets
     * sent from the TCP socket.
     */
    void setDscp(short dscp);

    /**
     * Sets the Ipv4 Type of Service / Ipv6 Traffic Class fields of packets
     * sent from the TCP socket.
     */
    void setTos(short tos);

    /**
     * Required to re-connect with a "used" TcpSocket object.
     * By default, a TcpSocket object is tied to a single TCP connection,
     * via the connectionId. When the connection gets closed or aborted,
     * you cannot use the socket to connect again (by connect() or listen())
     * unless you obtain a new connectionId by calling this method.
     *
     * BEWARE if you use TcpSocketMap! TcpSocketMap uses connectionId
     * to find TCPSockets, so after calling this method you have to remove
     * the socket from your TcpSocketMap, and re-add it. Otherwise TcpSocketMap
     * will get confused.
     *
     * The reason why one must obtain a new connectionId is that TCP still
     * has to maintain the connection data structure (identified by the old
     * connectionId) internally for a while (2 maximum segment lifetimes = 240s)
     * after it reported "connection closed" to us.
     */
    void renewSocket();

    /**
     * Returns true when the socket is opened
     */
    virtual bool isOpen() const override;
    //@}

    /** @name Handling of messages arriving from TCP */
    //@{
    /**
     * Returns true if the message belongs to this socket instance (message
     * has a TcpCommand as getControlInfo(), and the connId in it matches
     * that of the socket.)
     */
    virtual bool belongsToSocket(cMessage *msg) const override;

    /**
     * Sets a callback object, to be used with processMessage().
     * This callback object may be your simple module itself (if it
     * multiply inherits from ICallback too, that is you
     * declared it as
     * <pre>
     * class MyAppModule : public cSimpleModule, public TcpSocket::ICallback
     * </pre>
     * and redefined the necessary virtual functions; or you may use
     * dedicated class (and objects) for this purpose.
     *
     * TcpSocket doesn't delete the callback object in the destructor
     * or on any other occasion.
     */
    void setCallback(ICallback *cb);

    /**
     * Examines the message (which should have arrived from TCP),
     * updates socket state, and if there is a callback object installed
     * (see setCallback(), class ICallback), dispatches
     * to the appropriate method.
     *
     * The method deletes the message, unless (1) there is a callback object
     * installed AND (2) the message is payload (message kind TCP_I_DATA or
     * TCP_I_URGENT_DATA) when the responsibility of destruction is on the
     * socketDataArrived() callback method.
     *
     * IMPORTANT: for performance reasons, this method doesn't check that
     * the message belongs to this socket, i.e. belongsToSocket(msg) would
     * return true!
     */
    void processMessage(cMessage *msg) override;
    //@}
};

} // namespace inet

#endif


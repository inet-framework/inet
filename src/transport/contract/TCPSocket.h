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


#ifndef __INET_TCPSOCKET_H
#define __INET_TCPSOCKET_H

#include "INETDefs.h"

#include "TCPCommand_m.h"
#include "IPvXAddress.h"

class TCPStatusInfo;


/**
 * TCPSocket is a convenience class, to make it easier to manage TCP connections
 * from your application models. You'd have one (or more) TCPSocket object(s)
 * in your application simple module class, and call its member functions
 * (bind(), listen(), connect(), etc.) to open, close or abort a TCP connection.
 *
 * TCPSocket chooses and remembers the connId for you, assembles and sends command
 * packets (such as OPEN_ACTIVE, OPEN_PASSIVE, CLOSE, ABORT, etc.) to TCP,
 * and can also help you deal with packets and notification messages arriving
 * from TCP.
 *
 * A session which opens a connection from local port 1000 to 10.0.0.2:2000,
 * sends 16K of data and closes the connection may be as simple as this
 * (the code can be placed in your handleMessage() or activity()):
 *
 * <pre>
 *   TCPSocket socket;
 *   socket.connect(IPvXAddress("10.0.0.2"), 2000);
 *
 *   msg = new cMessage("data1");
 *   msg->setByteLength(16*1024);  // 16K
 *   socket.send(msg);
 *
 *   socket.close();
 * </pre>
 *
 * Dealing with packets and notification messages coming from TCP is somewhat
 * more cumbersome. Basically you have two choices: you either process those
 * messages yourself, or let TCPSocket do part of the job. For the latter,
 * you give TCPSocket a callback object on which it'll invoke the appropriate
 * member functions: socketEstablished(), socketDataArrived(), socketFailure(),
 * socketPeerClosed(), etc (these are methods of TCPSocket::CallbackInterface).,
 * The callback object can be your simple module class too.
 *
 * This code skeleton example shows how to set up a TCPSocket to use the module
 * itself as callback object:
 *
 * <pre>
 * class MyModule : public cSimpleModule, public TCPSocket::CallbackInterface
 * {
 *     TCPSocket socket;
 *     virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent);
 *     virtual void socketFailure(int connId, void *yourPtr, int code);
 *     ...
 * };
 *
 * void MyModule::initialize() {
 *     socket.setCallbackObject(this,NULL);
 * }
 *
 * void MyModule::handleMessage(cMessage *msg) {
 *     if (socket.belongsToSocket(msg))
 *         socket.processMessage(msg); // dispatch to socketXXXX() methods
 *     else
 *         ...
 * }
 *
 * void MyModule::socketDataArrived(int, void *, cPacket *msg, bool) {
 *     ev << "Received TCP data, " << msg->getByteLength() << " bytes\\n";
 *     delete msg;
 * }
 *
 * void MyModule::socketFailure(int, void *, int code) {
 *     if (code==TCP_I_CONNECTION_RESET)
 *         ev << "Connection reset!\\n";
 *     else if (code==TCP_I_CONNECTION_REFUSED)
 *         ev << "Connection refused!\\n";
 *     else if (code==TCP_I_TIMEOUT)
 *         ev << "Connection timed out!\\n";
 * }
 * </pre>
 *
 * If you need to manage a large number of sockets (e.g. in a server
 * application which handles multiple incoming connections), the TCPSocketMap
 * class may be useful. The following code fragment to handle incoming
 * connections is from the LDP module:
 *
 * <pre>
 * TCPSocket *socket = socketMap.findSocketFor(msg);
 * if (!socket)
 * {
 *     // not yet in socketMap, must be new incoming connection: add to socketMap
 *     socket = new TCPSocket(msg);
 *     socket->setOutputGate(gate("tcpOut"));
 *     socket->setCallbackObject(this, NULL);
 *     socketMap.addSocket(socket);
 * }
 * // dispatch to socketEstablished(), socketDataArrived(), socketPeerClosed()
 * // or socketFailure()
 * socket->processMessage(msg);
 * </pre>
 *
 * @see TCPSocketMap
 */
class INET_API TCPSocket
{
  public:
    /**
     * Abstract base class for your callback objects. See setCallbackObject()
     * and processMessage() for more info.
     *
     * Note: this class is not subclassed from cObject, because
     * classes may have both this class and cSimpleModule as base class,
     * and cSimpleModule is already a cObject.
     */
    class CallbackInterface
    {
      public:
        virtual ~CallbackInterface() {}
        virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent) = 0;
        virtual void socketEstablished(int connId, void *yourPtr) {}
        virtual void socketPeerClosed(int connId, void *yourPtr) {}
        virtual void socketClosed(int connId, void *yourPtr) {}
        virtual void socketFailure(int connId, void *yourPtr, int code) {}
        virtual void socketStatusArrived(int connId, void *yourPtr, TCPStatusInfo *status) {delete status;}
    };

    enum State {NOT_BOUND, BOUND, LISTENING, CONNECTING, CONNECTED, PEER_CLOSED, LOCALLY_CLOSED, CLOSED, SOCKERROR};

  protected:
    int connId;
    int sockstate;

    IPvXAddress localAddr;
    int localPrt;
    IPvXAddress remoteAddr;
    int remotePrt;

    CallbackInterface *cb;
    void *yourPtr;

    cGate *gateToTcp;

    TCPDataTransferMode dataTransferMode;
    std::string tcpAlgorithmClass;

  protected:
    void sendToTCP(cMessage *msg);

    // internal: implementation behind listen() and listenOnce()
    void listen(bool fork);

  public:
    /**
     * Constructor. The getConnectionId() method returns a valid Id right after
     * constructor call.
     */
    TCPSocket();

    /**
     * Constructor, to be used with forked sockets (see listen()).
     * The new connId will be picked up from the message: it should have
     * arrived from TCP and contain TCPCommmand control info.
     */
    TCPSocket(cMessage *msg);

    /**
     * Destructor
     */
    ~TCPSocket() {}

    /**
     * Returns the internal connection Id. TCP uses the (gate index, connId) pair
     * to identify the connection when it receives a command from the application
     * (or TCPSocket).
     */
    int getConnectionId() const  {return connId;}

    /**
     * Returns the socket state, one of NOT_BOUND, CLOSED, LISTENING, CONNECTING,
     * CONNECTED, etc. Messages received from TCP must be routed through
     * processMessage() in order to keep socket state up-to-date.
     */
    int getState()   {return sockstate;}

    /**
     * Returns name of socket state code returned by getState().
     */
    static const char *stateName(int state);

    /** @name Getter functions */
    //@{
    IPvXAddress getLocalAddress() {return localAddr;}
    int getLocalPort() {return localPrt;}
    IPvXAddress getRemoteAddress() {return remoteAddr;}
    int getRemotePort() {return remotePrt;}
    //@}

    /** @name Opening and closing connections, sending data */
    //@{

    /**
     * Sets the gate on which to send to TCP. Must be invoked before socket
     * can be used. Example: <tt>socket.setOutputGate(gate("tcpOut"));</tt>
     */
    void setOutputGate(cGate *toTcp)  {gateToTcp = toTcp;}

    /**
     * Bind the socket to a local port number.
     */
    void bind(int localPort);

    /**
     * Bind the socket to a local port number and IP address (useful with
     * multi-homing).
     */
    void bind(IPvXAddress localAddr, int localPort);

    /**
     * Returns the current dataTransferMode parameter.
     * @see TCPCommand
     */
    TCPDataTransferMode getDataTransferMode() const {return dataTransferMode;}

    /**
     * Returns the current tcpAlgorithmClass parameter.
     */
    const char *getTCPAlgorithmClass() const {return tcpAlgorithmClass.c_str();}

    /**
     * Convert a string to TCPDataTransferMode enum.
     * Returns TCP_TRANSFER_UNDEFINED when string has an invalid value
     * Generate runtime error, when string is NULL;
     */
    static TCPDataTransferMode convertStringToDataTransferMode(const char * transferMode);

    /**
     * Sets the dataTransferMode parameter of the subsequent connect() or listen() calls.
     * @see TCPCommand
     */
    void setDataTransferMode(TCPDataTransferMode transferMode) { dataTransferMode = transferMode; }

    /**
     * Read "dataTransferMode" parameter from ini/ned, and set dataTransferMode member value
     *
     * Generate runtime error when parameter is missing or value is invalid.
     */
    void readDataTransferModePar(cComponent &component);

    /**
     * Sets the tcpAlgorithmClass parameter of the next connect() or listen() call.
     */
    void setTCPAlgorithmClass(const char *tcpAlgorithmClass) { this->tcpAlgorithmClass = tcpAlgorithmClass; }

    /**
     * Initiates passive OPEN, creating a "forking" connection that will listen
     * on the port you bound the socket to. Every incoming connection will
     * get a new connId (and thus, must be handled with a new TCPSocket object),
     * while the original connection (original connId) will keep listening on
     * the port. The new TCPSocket object must be created with the
     * TCPSocket(cMessage *msg) constructor.
     *
     * If you need to handle multiple incoming connections, the TCPSocketMap
     * class can also be useful, and TCPSrvHostApp shows how to put it all
     * together. See also TCPOpenCommand documentation (neddoc) for more info.
     */
    void listen()  {listen(true);}

    /**
     * Initiates passive OPEN to create a non-forking listening connection.
     * Non-forking means that TCP will accept the first incoming
     * connection, and refuse subsequent ones.
     *
     * See TCPOpenCommand documentation (neddoc) for more info.
     */
    void listenOnce()  {listen(false);}

    /**
     * Active OPEN to the given remote socket.
     */
    void connect(IPvXAddress remoteAddr, int remotePort);

    /**
     * Sends data packet.
     */
    void send(cMessage *msg);

    /**
     * Closes the local end of the connection. With TCP, a CLOSE operation
     * means "I have no more data to send", and thus results in a one-way
     * connection until the remote TCP closes too (or the FIN_WAIT_1 timeout
     * expires)
     */
    void close();

    /**
     * Aborts the connection.
     */
    void abort();

    /**
     * Causes TCP to reply with a fresh TCPStatusInfo, attached to a dummy
     * message as getControlInfo(). The reply message can be recognized by its
     * message kind TCP_I_STATUS, or (if a callback object is used)
     * the socketStatusArrived() method of the callback object will be
     * called.
     */
    void requestStatus();

    /**
     * Required to re-connect with a "used" TCPSocket object.
     * By default, a TCPSocket object is tied to a single TCP connection,
     * via the connectionId. When the connection gets closed or aborted,
     * you cannot use the socket to connect again (by connect() or listen())
     * unless you obtain a new connectionId by calling this method.
     *
     * BEWARE if you use TCPSocketMap! TCPSocketMap uses connectionId
     * to find TCPSockets, so after calling this method you have to remove
     * the socket from your TCPSocketMap, and re-add it. Otherwise TCPSocketMap
     * will get confused.
     *
     * The reason why one must obtain a new connectionId is that TCP still
     * has to maintain the connection data structure (identified by the old
     * connectionId) internally for a while (2 maximum segment lifetimes = 240s)
     * after it reported "connection closed" to us.
     */
    void renewSocket();
    //@}

    /** @name Handling of messages arriving from TCP */
    //@{
    /**
     * Returns true if the message belongs to this socket instance (message
     * has a TCPCommand as getControlInfo(), and the connId in it matches
     * that of the socket.)
     */
    bool belongsToSocket(cMessage *msg);

    /**
     * Returns true if the message belongs to any TCPSocket instance.
     * (This basically checks if the message has a TCPCommand attached to
     * it as getControlInfo().)
     */
    static bool belongsToAnyTCPSocket(cMessage *msg);

    /**
     * Sets a callback object, to be used with processMessage().
     * This callback object may be your simple module itself (if it
     * multiply inherits from CallbackInterface too, that is you
     * declared it as
     * <pre>
     * class MyAppModule : public cSimpleModule, public TCPSocket::CallbackInterface
     * </pre>
     * and redefined the necessary virtual functions; or you may use
     * dedicated class (and objects) for this purpose.
     *
     * TCPSocket doesn't delete the callback object in the destructor
     * or on any other occasion.
     *
     * YourPtr is an optional pointer. It may contain any value you wish --
     * TCPSocket will not look at it or do anything with it except passing
     * it back to you in the CallbackInterface calls. You may find it
     * useful if you maintain additional per-connection information:
     * in that case you don't have to look it up by connId in the callbacks,
     * you can have it passed to you as yourPtr.
     */
    void setCallbackObject(CallbackInterface *cb, void *yourPtr = NULL);

    /**
     * Examines the message (which should have arrived from TCP),
     * updates socket state, and if there is a callback object installed
     * (see setCallbackObject(), class CallbackInterface), dispatches
     * to the appropriate method of it with the same yourPtr that
     * you gave in the setCallbackObject() call.
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
    void processMessage(cMessage *msg);
    //@}
};

#endif



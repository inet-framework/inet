//
// Copyright (C) 2004 Andras Varga
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#ifndef __TCPSOCKET_H
#define __TCPSOCKET_H


#include <omnetpp.h>
#include "TCPCommand_m.h"
#include "IPAddress.h"

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
 *   socket.connect(IPAddress("10.0.0.2"), 2000);
 *
 *   msg = new cMessage("data1");
 *   msg->setLength(8*16*1024);  // 16K
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
 *    TCPSocket socket;
 *    virtual void socketDataArrived(int connId, void *yourPtr, cMessage *msg, bool urgent);
 *    virtual void socketFailure(int connId, void *yourPtr, int code);
 *    ...
 * };
 *
 * void MyModule::initialize() {
 *    socket.setCallbackObject(this,NULL);
 * }
 *
 * void MyModule::handleMessage(cMessage *msg) {
 *    if (socket.belongsToSocket(msg))
 *       socket.processMessage(msg);
 *    else
 *       ...
 * }
 *
 * void MyModule::socketDataArrived(int, void *, cMessage *msg, bool) {
 *     ev << "Received TCP data, " << msg->length()/8 << " bytes\\n";
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
 * class may be useful.
 *
 * @see TCPSocketMap
 */
class TCPSocket
{
  public:
    /**
     * Abstract base class for your callback objects. See setCallbackObject()
     * and processMessage() for more info.
     *
     * Note: this class is not subclassed from cPolymorphic, because
     * classes may have both this class and cSimpleModule as base class,
     * and cSimpleModule is already a cPolymorphic.
     */
    class CallbackInterface
    {
      public:
        virtual ~CallbackInterface() {}
        virtual void socketDataArrived(int connId, void *yourPtr, cMessage *msg, bool urgent) = 0;
        virtual void socketEstablished(int connId, void *yourPtr) {}
        virtual void socketPeerClosed(int connId, void *yourPtr) {}
        virtual void socketClosed(int connId, void *yourPtr) {}
        virtual void socketFailure(int connId, void *yourPtr, int code) {}
        virtual void socketStatusArrived(int connId, void *yourPtr, TCPStatusInfo *status) {delete status;}
    };

    enum State {NOT_BOUND, CLOSED, LISTENING, CONNECTING, CONNECTED, PEER_CLOSED, LOCALLY_CLOSED, SOCKERROR};

  protected:
    int connId;
    int sockstate;

    IPAddress localAddr;
    int localPrt;
    IPAddress remoteAddr;
    int remotePrt;

    CallbackInterface *cb;
    void *yourPtr;

    cGate *gateToTcp;

  protected:
    void sendToTCP(cMessage *msg);

  public:
    /**
     * Constructor. The connectionId() method returns a valid Id right after
     * constructor call.
     */
    TCPSocket();

    /**
     * Constructor, to be used with forked sockets (see listen()).
     * The connId will be picked up from the message: it should have arrived
     * from TCPMain and contain TCPCommmand control info.
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
    int connectionId() const  {return connId;}

    /**
     * Returns the socket state, one of NOT_BOUND, CLOSED, LISTENING, CONNECTING,
     * CONNECTED, etc. Messages received from TCP must be routed through
     * processMessage() in order to keep socket state up-to-date.
     */
    int state()   {return sockstate;}

    /**
     * Returns name of socket state code returned by state().
     */
    static const char *stateName(int state);

    /** @name Getter functions */
    //@{
    IPAddress localAddress() {return localAddr;}
    int localPort() {return localPrt;}
    IPAddress remoteAddress() {return remoteAddr;}
    int remotePort() {return remotePrt;}
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
    void bind(IPAddress localAddr, int localPort);

    //
    // TBD add support for these options too!
    //  string sendQueueClass;
    //  string receiveQueueClass;
    //  string tcpAlgorithmClass;
    //

    /**
     * Initiates passive OPEN. If fork=true, you'll have to create a new
     * TCPSocket object for each incoming connection, and this socket
     * will keep listening on the port. If fork=false, the first incoming
     * connection will be accepted, and TCP will refuse subsequent ones.
     * See TCPOpenCommand documentation (neddoc) for more info.
     */
    void listen(bool fork=false);

    /**
     * Active OPEN to the given remote socket.
     */
    void connect(IPAddress remoteAddr, int remotePort);

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
     * message as controlInfo(). The reply message can be recognized by its
     * message kind TCP_I_STATUS, or (if a callback object is used)
     * the socketStatusArrived() method of the callback object will be
     * called.
     */
    void requestStatus();
    //@}

    /** @name Handling of messages arriving from TCP */
    //@{
    /**
     * Returns true if the message belongs to this socket instance (message
     * has a TCPCommand as controlInfo(), and the connId in it matches
     * that of the socket.)
     */
    bool belongsToSocket(cMessage *msg);

    /**
     * Returns true if the message belongs to any TCPSocket instance.
     * (This basically checks if the message has a TCPCommand attached to
     * it as controlInfo().)
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
    void setCallbackObject(CallbackInterface *cb, void *yourPtr=NULL);

    /**
     * Examines the message (which should have arrived from TCPMain),
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



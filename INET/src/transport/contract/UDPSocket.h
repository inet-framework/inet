//
// Copyright (C) 2005 Andras Varga
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


#ifndef __INET_UDPSOCKET_H
#define __INET_UDPSOCKET_H

#include <omnetpp.h>
#include "UDPControlInfo_m.h"


/**
 * UDPSocket is a convenience class, to make it easier to manage TCP connections
 * from your application models. You'd have one (or more) UDPSocket object(s)
 * in your application simple module class, and call its member functions
 * (bind(), listen(), connect(), etc.) to open, close or abort a TCP connection.
 *
 * UDPSocket chooses and remembers the sockId for you, assembles and sends command
 * packets (such as OPEN_ACTIVE, OPEN_PASSIVE, CLOSE, ABORT, etc.) to TCP,
 * and can also help you deal with packets and notification messages arriving
 * from TCP.
 *
 * A session which opens a connection from local port 1000 to 10.0.0.2:2000,
 * sends 16K of data and closes the connection may be as simple as this
 * (the code can be placed in your handleMessage() or activity()):
 *
 * <pre>
 *   UDPSocket socket;
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
 * messages yourself, or let UDPSocket do part of the job. For the latter,
 * you give UDPSocket a callback object on which it'll invoke the appropriate
 * member functions: socketEstablished(), socketDataArrived(), socketFailure(),
 * socketPeerClosed(), etc (these are methods of UDPSocket::CallbackInterface).,
 * The callback object can be your simple module class too.
 *
 * This code skeleton example shows how to set up a UDPSocket to use the module
 * itself as callback object:
 *
 * <pre>
 * class MyModule : public cSimpleModule, public UDPSocket::CallbackInterface
 * {
 *    UDPSocket socket;
 *    virtual void socketDataArrived(int sockId, void *yourPtr, cPacket *msg, bool urgent);
 *    virtual void socketFailure(int sockId, void *yourPtr, int code);
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
 * void MyModule::socketDatagramArrived(int, void *, cMessage *msg, UDPControlInfo *ctrl) {
 *     ev << "Received UDP packet, " << msg->getByteLength() << " bytes\\n";
 *     delete msg;
 * }
 *
 * void MyModule::socketPeerClosed(int, void *, int code) {
 *     ev << "Socket peer closed!\\n";
 * }
 * </pre>
 *
 * If you need to manage a large number of sockets, the UDPSocketMap
 * class may be useful.
 *
 * @see UDPSocketMap
 */
class INET_API UDPSocket
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
        virtual void socketDatagramArrived(int sockId, void *yourPtr, cMessage *msg, UDPControlInfo *ctrl) = 0;
        virtual void socketPeerClosed(int sockId, void *yourPtr) {}
    };

    enum State {NOT_BOUND, BOUND};  // FIXME needed?

  protected:
    int sockId;
    int usrId;
    int sockstate;

    IPvXAddress localAddr; // needed for sendTo() as local address
    int localPrt;
    IPvXAddress remoteAddr; // needed to enable send(msg) call
    int remotePrt; // needed to enable send(msg) call
    int mcastIfaceId;

    CallbackInterface *cb;
    void *yourPtr;

    cGate *gateToUdp;

  protected:
    void sendToUDP(cMessage *msg);

  public:
    /**
     * Constructor. The getSocketId() method returns a valid Id right after
     * constructor call.
     */
    UDPSocket();

    /**
     * Destructor
     */
    ~UDPSocket() {}

    /**
     * Returns the internal socket Id.
     */
    int getSocketId() const  {return sockId;}

    /**
     * Sets userId to an arbitrary value. (This value will be sent back to us
     * by UDP in UDPControlInfo if we receive a packet on this socket.)
     */
    void setUserId(int userId);

    /**
     * Returns the userId.
     */
    int getUserId() const  {return usrId;}

    /**
     * Returns the socket state, one of NOT_BOUND, BOUND, etc.
     * Messages received from UDP must be routed through
     * processMessage() in order to keep socket state up-to-date.
     */
    int getState()   {return sockstate;}

    /**
     * Returns name of socket state code returned by getState().
     */
    static const char *stateName(int state);

    /**
     * Generates a new socket id.
     */
    static int generateSocketId();

    /** @name Getter functions */
    //@{
    IPvXAddress getLocalAddress() {return localAddr;}
    int getLocalPort() {return localPrt;}
    //@}

    /** @name Opening and closing connections, sending data */
    //@{

    /**
     * Sets the gate on which to send to UDP. Must be invoked before socket
     * can be used. Example: <tt>socket.setOutputGate(gate("udpOut"));</tt>
     */
    void setOutputGate(cGate *toUdp)  {gateToUdp = toUdp;}

    /**
     * Bind the socket to a local port number. Use port=0 for ephemeral port.
     */
    void bind(int localPort);

    /**
     * Bind the socket to a local port number and IP address (useful with
     * multi-homing or multicast addresses). Use port=0 for an ephemeral port.
     */
    void bind(IPvXAddress localAddr, int localPort);

    /**
     * Connects to a remote UDP socket. This has two effects:
     * (1) this socket will only receive packets from specified address/port,
     * and (2) you can use send() (as opposed to sendTo()) to send packets.
     */
    void connect(IPvXAddress remoteAddr, int remotePort);

    /**
     * Set the output interface for sending multicast packets (like the Unix
     * IP_MULTICAST_IF socket option). The argument is the interface's Id
     * in InterfaceTable.
     */
    void setMulticastInterfaceId(int interfaceId) {mcastIfaceId = interfaceId;}

    /**
     * Returns the output interface for sending multicast packets.
     */
    int getMulticastInterfaceId() const {return mcastIfaceId;}

    /**
     * Sends a data packet to the given address and port.
     */
    void sendTo(cMessage *msg, IPvXAddress destAddr, int destPort);

    /**
     * Sends a data packet to the address and port specified previously
     * in a connect() call.
     */
    void send(cMessage *msg);

    /**
     * Unbinds the socket. There is no need for renewSocket() as with TCPSocket.
     */
    void close();
    //@}

    /** @name Handling of messages arriving from UDP */
    //@{
    /**
     * Returns true if the message belongs to this socket instance (message
     * has a UDPControlInfo as getControlInfo(), and the sockId in it matches
     * that of the socket.)
     */
    bool belongsToSocket(cMessage *msg);

    /**
     * Returns true if the message belongs to any UDPSocket instance.
     * (This basically checks if the message has an UDPControlInfo attached to
     * it as getControlInfo().)
     */
    static bool belongsToAnyUDPSocket(cMessage *msg);

    /**
     * Sets a callback object, to be used with processMessage().
     * This callback object may be your simple module itself (if it
     * multiply inherits from CallbackInterface too, that is you
     * declared it as
     * <pre>
     * class MyAppModule : public cSimpleModule, public UDPSocket::CallbackInterface
     * </pre>
     * and redefined the necessary virtual functions; or you may use
     * dedicated class (and objects) for this purpose.
     *
     * UDPSocket doesn't delete the callback object in the destructor
     * or on any other occasion.
     *
     * YourPtr is an optional pointer. It may contain any value you wish --
     * UDPSocket will not look at it or do anything with it except passing
     * it back to you in the CallbackInterface calls. You may find it
     * useful if you maintain additional per-connection information:
     * in that case you don't have to look it up by sockId in the callbacks,
     * you can have it passed to you as yourPtr.
     */
    void setCallbackObject(CallbackInterface *cb, void *yourPtr=NULL);

    /**
     * Examines the message (which should have arrived from UDP),
     * and if there is a callback object installed (see setCallbackObject(),
     * class CallbackInterface), dispatches to the appropriate method of
     * it with the same yourPtr that you gave in the setCallbackObject() call.
     *
     * IMPORTANT: for performance reasons, this method doesn't check that
     * the message belongs to this socket, i.e. belongsToSocket(msg) would
     * return true!
     */
    void processMessage(cMessage *msg);
    //@}
};

#endif



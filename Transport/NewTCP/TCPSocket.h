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
#include "IPAddress.h"

class TCPStatusInfo;


/**
 * TCPSocket is a convenience class, to make it easier to manage TCP connections
 * from your application models. You'd have one (or more) TCPSocket object(s)
 * in your application simple module class, and call its member functions
 * (bind(), accept(), connect(), etc.) to open, close or abort a TCP connection.
 *
 * TCPSocket chooses and remembers the connId for you, assembles and sends command
 * packets (such as OPEN_ACTIVE, OPEN_PASSIVE, CLOSE, ABORT, etc.) to TCP,
 * and can also help you deal with packets and notification messages arriving
 * from TCP.
 *
 * An example session which opens a connection from local port 1000 to
 * 10.0.0.2:2000, sends 16K of data and closes the connection looks
 * like this (the code can be placed in your handleMessage() or activity()):
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
 * member functions: dataArrived(), established(), connectionReset(),
 * remoteTCPClosed(), etc (actually these are methods of TCPSocket::CallBackInterface,
 * and their names are prefixed with "socket"). The callback object can be
 * your simple module class too (for this you'll have to use multiple inheritance).
 *
 * This code skeleton example shows how to set up a TCPSocket to use the module
 * itself as callback object:
 *
 * <pre>
 * class MyModule : public cSimpleModule, public TCPSocket::CallBackInterface {
 *    TCPSocket socket;
 *    virtual void socketDataArrived(void *yourPtr, cMessage *msg, bool urgent);
 *    virtual void socketConnectionReset(void *yourPtr);
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
 * void MyModule::socketDataArrived(void *, cMessage *msg, bool) {
 *     ev << "Received TCP data, " << msg->length()/8 << " bytes\n";
 *     delete msg;
 * }
 *
 * void MyModule::socketConnectionReset(void *) {
 *     ev << "Connection Reset!!!\n";
 * }
 * </pre>
 *
 */
class TCPSocket
{
  public:
    /**
     * Abstract base class for your callback objects. See setCallbackObject()
     * and processMessage() for more info.
     */
    class CallBackInterface
    {
      public:
        virtual ~CallBackInterface() {}
        virtual void socketDataArrived(void *yourPtr, cMessage *msg, bool urgent) = 0;
        virtual void socketEstablished(void *yourPtr) {}
        virtual void socketRemoteTCPClosed(void *yourPtr) {}
        virtual void socketClosed(void *yourPtr) {}
        virtual void socketConnectionReset(void *yourPtr) {}
        virtual void socketStatusArrived(void *yourPtr, TCPStatusInfo *status) {delete status;}
    };

  protected:
    static int nextConnId;
    int connId;

    bool isBound;
    IPAddress localAddr;
    int localPort;

    CallBackInterface *cb;
    void *yourPtr;

  protected:
    void sendToTCP(cMessage *msg);

  public:
    /**
     * Constructor. Doesn't do much more than choose a unique connId.
     */
    TCPSocket();
    ~TCPSocket() {}

    /**
     * Returns the internal connection Id. TCP uses the (gate index, connId) pair
     * to identify the connection when it receives a command from the application
     * (or TCPSocket).
     */
    int connectionId() const  {return connId;}

    /** @name Opening and closing connections, sending data */
    //@{

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
    // FIXME add support for these options too!
    //  string sendQueueClass;
    //  string receiveQueueClass;
    //  string tcpAlgorithmClass;
    //

    /**
     * Initiates passive OPEN.
     */
    void accept();

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
     * multiply inherits from CallBackInterface too, that is you
     * declared it as
     * <pre>
     * class MyAppModule : public cSimpleModule, public TCPSocket::CallBackInterface
     * </pre>
     * and redefined the necessary virtual functions; or you may use
     * dedicated class (and objects) for this purpose.
     *
     * TCPSocket doesn't delete the callback object in the destructor
     * or on any other occasion.
     */
    void setCallbackObject(CallBackInterface *cb, void *yourPtr=NULL);

    /**
     * Checks if the message belongs to this socket (see belongsToSocket()),
     * and if so, processes it; otherwise it does nothing.
     * "Processing" means that the appropriate method of the callback object
     * (see setCallbackObject(), class CallBackInterface) will get called,
     * with the same yourPtr that you gave in the setCallbackObject() call.
     * If no callback object is installed, this method throws an error.
     */
    void processMessage(cMessage *msg);
    //@}
};

#endif



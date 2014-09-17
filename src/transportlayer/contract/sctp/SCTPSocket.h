//
// Copyright (C) 2008 Irene Ruengeler
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_SCTPSOCKET_H
#define __INET_SCTPSOCKET_H

#include <vector>

#include "inet/common/INETDefs.h"

#include "inet/networklayer/common/L3Address.h"

namespace inet {

class SCTPStatusInfo;
namespace sctp { class SCTP; }

typedef std::vector<L3Address> AddressVector;

class INET_API SCTPSocket
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
        virtual void socketDataArrived(int assocId, void *yourPtr, cPacket *msg, bool urgent) = 0;
        virtual void socketDataNotificationArrived(int assocId, void *yourPtr, cPacket *msg) = 0;
        virtual void socketEstablished(int assocId, void *yourPtr, uint64 buffer) {}
        virtual void socketPeerClosed(int assocId, void *yourPtr) {}
        virtual void socketClosed(int assocId, void *yourPtr) {}
        virtual void socketFailure(int assocId, void *yourPtr, int code) {}
        virtual void socketStatusArrived(int assocId, void *yourPtr, SCTPStatusInfo *status) {}    // {delete status;}
        virtual void sendRequestArrived() {}
        virtual void msgAbandonedArrived(int assocId) {}
        virtual void shutdownReceivedArrived(int connId) {}
        virtual void sendqueueFullArrived(int connId) {}
        virtual void sendqueueAbatedArrived(int connId, uint64 buffer) {}
        virtual void addressAddedArrived(int assocId, L3Address localAddr, L3Address remoteAddr) {}
    };

    enum State { NOT_BOUND, CLOSED, LISTENING, CONNECTING, CONNECTED, PEER_CLOSED, LOCALLY_CLOSED, SOCKERROR };

  protected:
    int assocId;
    int sockId;
    int sockstate;
    bool oneToOne;

    L3Address localAddr;
    AddressVector localAddresses;

    int localPrt;
    L3Address remoteAddr;
    AddressVector remoteAddresses;
    int remotePrt;
    int fsmStatus;
    int inboundStreams;
    int outboundStreams;
    int lastStream;

    CallbackInterface *cb;
    void *yourPtr;

  protected:
    void sendToSCTP(cPacket *msg);

  public:
    cGate *gateToSctp;
    /**
     * Constructor. The connectionId() method returns a valid Id right after
     * constructor call.
     */
    // SCTPSocket();
    SCTPSocket(bool type = true);

    /**
     * Constructor, to be used with forked sockets (see listen()).
     * The assocId will be picked up from the message: it should have arrived
     * from SCTPMain and contain SCTPCommmand control info.
     */
    SCTPSocket(cPacket *msg);

    /**
     * Destructor
     */
    ~SCTPSocket();

    /**
     * Returns the internal connection Id. SCTP uses the (gate index, assocId) pair
     * to identify the connection when it receives a command from the application
     * (or SCTPSocket).
     */
    int getConnectionId() const { return assocId; }

    /**
     * Returns the socket state, one of NOT_BOUND, CLOSED, LISTENING, CONNECTING,
     * CONNECTED, etc. Messages received from SCTP must be routed through
     * processMessage() in order to keep socket state up-to-date.
     */
    int getState() { return sockstate; }

    /**
     * Returns name of socket state code returned by state().
     */
    static const char *stateName(int state);

    /** @name Getter functions */
    //@{
    AddressVector getLocalAddresses() { return localAddresses; }
    int getLocalPort() { return localPrt; }
    AddressVector getRemoteAddresses() { return remoteAddresses; }
    int getRemotePort() { return remotePrt; }
    L3Address getRemoteAddr() { return remoteAddr; }
    //@}

    /** @name Opening and closing connections, sending data */
    //@{

    /**
     * Sets the gate on which to send to SCTP. Must be invoked before socket
     * can be used. Example: <tt>socket.setOutputGate(gate("sctpOut"));</tt>
     */
    void setOutputGate(cGate *toSctp) { gateToSctp = toSctp; };
    void setOutboundStreams(int streams) { outboundStreams = streams; };
    void setInboundStreams(int streams) { inboundStreams = streams; };
    int getOutboundStreams() { return outboundStreams; };
    int getLastStream() { return lastStream; };
    void setStreamPriority(uint32 stream, uint32 priority);

    /**
     * Bind the socket to a local port number.
     */
    void bind(int localPort);

    /**
     * Bind the socket to a local port number and IP address (useful with
     * multi-homing).
     */
    void bind(L3Address localAddr, int localPort);

    void bindx(AddressVector localAddr, int localPort);

    void addAddress(L3Address addr);
    //
    // TBD add support for these options too!
    //  string sendQueueClass;
    //  string receiveQueueClass;
    //  string sctpAlgorithmClass;
    //

    /**
     * Initiates passive OPEN. If fork=true, you'll have to create a new
     * SCTPSocket object for each incoming connection, and this socket
     * will keep listening on the port. If fork=false, the first incoming
     * connection will be accepted, and SCTP will refuse subsequent ones.
     * See SCTPOpenCommand documentation (neddoc) for more info.
     */
    void listen(bool fork = false, bool streamReset = false, uint32 requests = 0, uint32 messagesToPush = 0);

    /**
     * Active OPEN to the given remote socket.
     */
    void connect(L3Address remoteAddress, int32 remotePort, bool streamReset, int32 prMethod, uint32 numRequests);

    void connectx(AddressVector remoteAddresses, int32 remotePort, bool streamReset = false, int32 prMethod = 0, uint32 numRequests = 0);

    /**
     * Sends data packet.
     */
    void send(cPacket *msg, bool last = true, bool primary = true);
    void send(cPacket *msg, int prMethod, double prValue, bool last);
    void send(cPacket *msg, int prMethod, double prValue, bool last, int32 streamId);

    void sendNotification(cPacket *msg);
    void sendRequest(cPacket *msg);
    /**
     * Closes the local end of the connection. With SCTP, a CLOSE operation
     * means "I have no more data to send", and thus results in a one-way
     * connection until the remote SCTP closes too (or the FIN_WAIT_1 timeout
     * expires)
     */
    void close();

    /**
     * Aborts the association.
     */
    void abort();
    void shutdown();
    /**
     * Causes SCTP to reply with a fresh SCTPStatusInfo, attached to a dummy
     * message as controlInfo(). The reply message can be recognized by its
     * message kind SCTP_I_STATUS, or (if a callback object is used)
     * the socketStatusArrived() method of the callback object will be
     * called.
     */
    void requestStatus();
    //@}

    /** @name Handling of messages arriving from SCTP */
    //@{
    /**
     * Returns true if the message belongs to this socket instance (message
     * has a SCTPCommand as controlInfo(), and the assocId in it matches
     * that of the socket.)
     */
    bool belongsToSocket(cPacket *msg);

    /**
     * Returns true if the message belongs to any SCTPSocket instance.
     * (This basically checks if the message has a SCTPCommand attached to
     * it as controlInfo().)
     */
    static bool belongsToAnySCTPSocket(cPacket *msg);

    /**
     * Sets a callback object, to be used with processMessage().
     * This callback object may be your simple module itself (if it
     * multiply inherits from CallbackInterface too, that is you
     * declared it as
     * <pre>
     * class MyAppModule : public cSimpleModule, public SCTPSocket::CallbackInterface
     * </pre>
     * and redefined the necessary virtual functions; or you may use
     * dedicated class (and objects) for this purpose.
     *
     * SCTPSocket doesn't delete the callback object in the destructor
     * or on any other occasion.
     *
     * YourPtr is an optional pointer. It may contain any value you wish --
     * SCTPSocket will not look at it or do anything with it except passing
     * it back to you in the CallbackInterface calls. You may find it
     * useful if you maintain additional per-connection information:
     * in that case you don't have to look it up by assocId in the callbacks,
     * you can have it passed to you as yourPtr.
     */
    void setCallbackObject(CallbackInterface *cb, void *yourPtr = NULL);

    /**
     * Examines the message (which should have arrived from SCTPMain),
     * updates socket state, and if there is a callback object installed
     * (see setCallbackObject(), class CallbackInterface), dispatches
     * to the appropriate method of it with the same yourPtr that
     * you gave in the setCallbackObject() call.
     *
     * The method deletes the message, unless (1) there is a callback object
     * installed AND (2) the message is payload (message kind SCTP_I_DATA or
     * SCTP_I_URGENT_DATA) when the responsibility of destruction is on the
     * socketDataArrived() callback method.
     *
     * IMPORTANT: for performance reasons, this method doesn't check that
     * the message belongs to this socket, i.e. belongsToSocket(msg) would
     * return true!
     */
    void processMessage(cPacket *msg);
    //@}

    void setState(int state) { sockstate = state; };
};

} // namespace inet

#endif // ifndef __INET_SCTPSOCKET_H



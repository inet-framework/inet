//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2015 Thomas Dreibholz
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

#include "inet/common/packet/Message.h"
#include "inet/common/socket/ISocket.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"

namespace inet {

typedef std::vector<L3Address> AddressVector;

typedef struct {
    int maxInitRetrans;
    int maxInitRetransTimeout;
    double rtoInitial;
    double rtoMin;
    double rtoMax;
    int sackFrequency;
    double sackPeriod;
    int maxBurst;
    int fragPoint;
    int nagle;
    bool enableHeartbeats;
    int pathMaxRetrans;
    double hbInterval;
    int assocMaxRtx;
} SocketOptions;

typedef struct {
    int inboundStreams;
    int outboundStreams;
    bool streamReset;
} AppSocketOptions;

class INET_API SctpSocket : public ISocket
{
  public:
    /**
     * Abstract base class for your callback objects. See setCallback()
     * and processMessage() for more info.
     *
     * Note: this class is not subclassed from cObject, because
     * classes may have both this class and cSimpleModule as base class,
     * and cSimpleModule is already a cObject.
     */
    class INET_API ICallback
    {
      public:
        virtual ~ICallback() {}
        virtual void socketDataArrived(SctpSocket *socket, Packet *packet, bool urgent) = 0;
        virtual void socketDataNotificationArrived(SctpSocket *socket, Message *msg) = 0;
        virtual void socketEstablished(SctpSocket *socket, unsigned long int buffer) {}
        virtual void socketPeerClosed(SctpSocket *socket) {}
        virtual void socketClosed(SctpSocket *socket) {}
        virtual void socketFailure(SctpSocket *socket, int code) {}
        virtual void socketStatusArrived(SctpSocket *socket, SctpStatusReq *status) { }
        virtual void socketDeleted(SctpSocket *socket) {}
        virtual void sendRequestArrived(SctpSocket *socket) {}
        virtual void msgAbandonedArrived(SctpSocket *socket) {}
        virtual void shutdownReceivedArrived(SctpSocket *socket) {}
        virtual void sendqueueFullArrived(SctpSocket *socket) {}
        virtual void sendqueueAbatedArrived(SctpSocket *socket, unsigned long int buffer) {}
        virtual void addressAddedArrived(SctpSocket *socket, L3Address localAddr, L3Address remoteAddr) {}
    };

    enum State { NOT_BOUND, CLOSED, LISTENING, CONNECTING, CONNECTED, PEER_CLOSED, LOCALLY_CLOSED, SOCKERROR };

  protected:
    int assocId;
    static int32 nextAssocId;
    int sockstate;
    bool oneToOne;
    bool appLimited;

    L3Address localAddr;
    AddressVector localAddresses;

    int localPrt;
    L3Address remoteAddr;
    AddressVector remoteAddresses;
    int remotePrt;
    int fsmStatus;
    int lastStream;

    /* parameters used in the socket API */
    SocketOptions *sOptions;
    AppSocketOptions *appOptions;

    ICallback *cb;
    void *userData;

  protected:
    void sendToSctp(cMessage *msg);

  public:
    cGate *gateToSctp;
    int interfaceIdToTun = -1;
    /**
     * Constructor. The connectionId() method returns a valid Id right after
     * constructor call.
     */
    SctpSocket(bool type = true);

    /**
     * Constructor, to be used with forked sockets (see listen()).
     * The assocId will be picked up from the message: it should have arrived
     * from SctpMain and contain SctpCommmand control info.
     */
    SctpSocket(cMessage *msg);

    /**
     * Destructor
     */
    ~SctpSocket();

    void *getUserData() const { return userData; }
    void setUserData(void *userData) { this->userData = userData; }

    /**
     * Returns the internal connection Id. SCTP uses the (gate index, assocId) pair
     * to identify the connection when it receives a command from the application
     * (or SctpSocket).
     */
    int getSocketId() const override { return assocId; }

    /**
     * Generates a new integer, to be used as assocId. (assocId is part of the key
     * which associates connections with their apps).
     */
    static int32 getNewAssocId() { return getEnvir()->getUniqueNumber(); }

    /**
     * Returns the socket state, one of NOT_BOUND, CLOSED, LISTENING, CONNECTING,
     * CONNECTED, etc. Messages received from SCTP must be routed through
     * processMessage() in order to keep socket state up-to-date.
     */
    int getState() const { return sockstate; }

    /**
     * Returns name of socket state code returned by state().
     */
    static const char *stateName(int state);

    /** @name Getter functions */
    //@{
    AddressVector getLocalAddresses() const { return localAddresses; }
    int getLocalPort() const { return localPrt; }
    AddressVector getRemoteAddresses() const { return remoteAddresses; }
    int getRemotePort() const { return remotePrt; }
    L3Address getRemoteAddr() const { return remoteAddr; }
    //@}

    /** @name Opening and closing connections, sending data */
    //@{

    /**
     * Sets the gate on which to send to SCTP. Must be invoked before socket
     * can be used. Example: <tt>socket.setOutputGate(gate("sctpOut"));</tt>
     */
    void setOutputGate(cGate *toSctp) { gateToSctp = toSctp; };

    /**
     * Setter and getter methods for socket and API Parameters
     */
    void setOutboundStreams(int streams) { appOptions->outboundStreams = streams; };
    void setInboundStreams(int streams) { appOptions->inboundStreams = streams; };
    void setAppLimited(bool option) { appLimited = option; };
    void setStreamReset(int option) { appOptions->streamReset = option; };
    void setStreamPriority(uint32 stream, uint32 priority);
    void setMaxInitRetrans(int option) { sOptions->maxInitRetrans = option; };
    void setMaxInitRetransTimeout(int option) { sOptions->maxInitRetransTimeout = option; };
    void setRtoInitial(double option) { sOptions->rtoInitial = option; };
    void setRtoMin(double option) { sOptions->rtoMin = option; };
    void setRtoMax(double option) { sOptions->rtoMax = option; };
    void setSackFrequency(int option) { sOptions->sackFrequency = option; };
    void setSackPeriod(double option) { sOptions->sackPeriod = option; };
    void setMaxBurst(int option) { sOptions->maxBurst = option; };
    void setFragPoint(int option) { sOptions->fragPoint = option; };
    void setNagle(int option) { sOptions->nagle = option; };
    void setPathMaxRetrans(int option) { sOptions->pathMaxRetrans = option; };
    void setEnableHeartbeats(bool option) { sOptions->enableHeartbeats = option; };
    void setHbInterval(double option) { sOptions->hbInterval = option; };
    void setRtoInfo(double initial, double max, double min);
    void setAssocMaxRtx(int option) { sOptions->assocMaxRtx = option; };

    void setUserOptions(SocketOptions* msg) { delete sOptions; sOptions = msg; };

    int getOutboundStreams() { return appOptions->outboundStreams; };
    int getInboundStreams() { return appOptions->inboundStreams; };
    bool getStreamReset() { return appOptions->streamReset; }
    int getLastStream() { return lastStream; };
    double getRtoInitial() { return sOptions->rtoInitial; };
    int getMaxInitRetransTimeout() { return sOptions->maxInitRetransTimeout; };
    int getMaxInitRetrans() { return sOptions->maxInitRetrans; };
    int getMaxBurst() { return sOptions->maxBurst; };
    int getFragPoint() { return sOptions->fragPoint; };
    int getAssocMaxRtx() { return sOptions->assocMaxRtx; };

    void getSocketOptions();

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
     * SctpSocket object for each incoming connection, and this socket
     * will keep listening on the port. If fork=false, the first incoming
     * connection will be accepted, and SCTP will refuse subsequent ones.
     * See SctpOpenCommand documentation (neddoc) for more info.
     */
    void listen(bool fork = true, bool streamReset = false, uint32 requests = 0, uint32 messagesToPush = 0);

    void listen(uint32 requests = 0, bool fork = false, uint32 messagesToPush = 0, bool options = false, int32 fd = -1);

    void accept(int socketId);

    /**
     * Active OPEN to the given remote socket.
     */
    void connect(L3Address remoteAddress, int32 remotePort, bool streamReset = false, int32 prMethod = 0, uint32 numRequests = 0);

    void connect(int32 fd, L3Address remoteAddress, int32 remotePort, uint32 numRequests, bool options = false);

    /**
     * Active OPEN to the given remote socket.
     * The current implementation just calls connect() with the first address
     * of the given list. This behaviour may be improved in the future.
     */
    void connectx(AddressVector remoteAddresses, int32 remotePort, bool streamReset = false, int32 prMethod = 0, uint32 numRequests = 0);

    void accept(int32 assocId, int32 fd);

    /**
     * Send data message
     */
    void send(Packet *packet);

    /**
     * Send notification.
     */
    void sendNotification(cMessage *msg);

    /**
     * Send request.
     */
    void sendRequest(cMessage *msg);

    /**
     * Closes the local end of the connection. With SCTP, a CLOSE operation
     * means "I have no more data to send", and thus results in a one-way
     * connection until the remote SCTP closes too (or the FIN_WAIT_1 timeout
     * expires)
     */
    void close(int id = -1);

    /**
     * Aborts the association.
     */
    void abort();
    void shutdown(int id = -1);
    /**
     * Causes SCTP to reply with a fresh SctpStatusInfo, attached to a dummy
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
     * has a SctpCommand as controlInfo(), and the assocId in it matches
     * that of the socket.)
     */
    virtual bool belongsToSocket(cMessage *msg) const override;

    /**
     * Sets a callback object, to be used with processMessage().
     * This callback object may be your simple module itself (if it
     * multiply inherits from CallbackInterface too, that is you
     * declared it as
     * <pre>
     * class MyAppModule : public cSimpleModule, public SctpSocket::CallbackInterface
     * </pre>
     * and redefined the necessary virtual functions; or you may use
     * dedicated class (and objects) for this purpose.
     *
     * SctpSocket doesn't delete the callback object in the destructor
     * or on any other occasion.
     */
    void setCallback(ICallback *cb);

    /**
     * Examines the message (which should have arrived from SctpMain),
     * updates socket state, and if there is a callback object installed
     * (see setCallback(), class CallbackInterface), dispatches
     * to the appropriate method.
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
    void processMessage(cMessage *msg) override;
    //@}

    void setState(int state) { sockstate = state; };

    void setTunInterface(int id) { interfaceIdToTun = id; };

    int getTunInterface() { return interfaceIdToTun; };
};

} // namespace inet

#endif // ifndef __INET_SCTPSOCKET_H



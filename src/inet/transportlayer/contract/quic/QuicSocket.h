//
// Copyright (C) 2018 CaDS HAW Hamburg BCK, Denis Lugowski, Marvin Butkereit
// based on the Work of Copyright (C) 2005,2011 Andras Varga
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

#ifndef __INET_QUICSOCKET_H
#define __INET_QUICSOCKET_H

#include <vector>
#include "inet/common/INETDefs.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/socket/ISocket.h"
#include "inet/networklayer/common/L3Address.h"
#include "QuicCommand_m.h"

namespace inet {

//class QUICDataIndication;

/**
 * QUICSocket is a convenience class, to make it easier to send and receive
 * QUIC packets from your application models. You'd have one (or more)
 * QUICSocket object(s) in your application simple module class, and call
 * its member functions (bind(), connect(), sendTo(), etc.) to create and
 * configure a socket, and to send datagrams.
 *
 * QUICSocket chooses and remembers the sockId for you, assembles and sends command
 * packets such as QUIC_C_BIND to QUIC, and can also help you deal with packets and
 * notification messages arriving from QUIC.
 *
 * Here is a code fragment that creates an QUIC socket and sends a 1K packet
 * over it (the code can be placed in your handleMessage() or activity()):
 *
 * <pre>
 *   QUICSocket socket;
 *   socket.setOutputGate(gate("quicOut"));
 *   socket.connect(Address("10.0.0.2"), 2000);
 *
 *   cPacket *pk = new cPacket("dgram");
 *   pk->setByteLength(1024);
 *   socket.send(pk);
 *
 *   socket.close();
 * </pre>
 *
 * Processing messages sent up by the QUIC module is relatively straightforward.
 * You only need to distinguish between data packets and error notifications,
 * by checking the message kind (should be either QUIC_I_DATA or QUIC_I_ERROR),
 * and casting the control info to QUICDataIndication or QUICErrorIndication.
 * USPSocket provides some help for this with the belongsToSocket() and
 * belongsToAnyQUICSocket() methods.
 */
class INET_API QuicSocket : public ISocket
{
public:
    /**
     * Callback interface for QUIC sockets, see setCallback() and processMessage() for more info.
     *
     * Note: this class is not subclassed from cObject, because
     * classes may have both this class and cSimpleModule as base class,
     * and cSimpleModule is already a cObject.
     */
    class INET_API ICallback
    {
      public:
        virtual ~ICallback() {}
        /**
         * Notifies about data arrival, packet ownership is transferred to the callee.
         */
        virtual void socketDataArrived(QuicSocket* socket, Packet *packet) = 0;
        virtual void socketAvailable(QuicSocket *socket, QuicAvailableInfo *availableInfo) = 0;
        virtual void socketEstablished(QuicSocket *socket) = 0;
        virtual void socketClosed(QuicSocket *socket) = 0;
        virtual void socketDeleted(QuicSocket *socket) = 0;
        virtual void socketSendQueueFull(QuicSocket *socket) = 0;
        virtual void socketSendQueueDrain(QuicSocket *socket) = 0;
        virtual void socketMsgRejected(QuicSocket *socket) = 0;
    };

    enum State { NOT_BOUND, BOUND, LISTENING, CONNECTING, CONNECTED, PEER_CLOSED, LOCALLY_CLOSED, CLOSED, SOCKERROR };

protected:

    int socketId;
    //uint64_t connectionID;
    cGate *gateToQuic;

    //uint64_t maxNumStreams = 0;
    //uint64_t lastStream = 0;
    ICallback *cb = nullptr;
    State socketState;

    L3Address localAddr;
    int localPort;
    L3Address remoteAddr;
    int remotePort;

protected:
    void sendToQuic(cMessage *msg);

public:

    /**
     * Constructor. The getSocketId() method returns a valid Id right after
     * constructor call.
     */
    QuicSocket();

    /**
     * Destructor
     */
    ~QuicSocket();

    /**
     * Generates a new socket id.
     */
    static int generateSocketId();

    /**
     * Generates a new connection id.
     */
    //static int generateSocketId();

    /** @name Opening and closing connections, sending data */
    //@{
    /**
     * Sets the gate on which to send to QUIC. Must be invoked before socket
     * can be used. Example: <tt>socket.setOutputGate(gate("quicOut"));</tt>
     */
    void setOutputGate(cGate *toQuic)
    {
        gateToQuic = toQuic;
    }

    //void setMaxNumStreams(int maxNumStreams) { this->maxNumStreams = maxNumStreams; };

    /**
     * Bind the socket to a local port number. Use port=0 for ephemeral port.
     */
    void bind(int localPort);

    /**
     * Bind the socket to a local port number and IP address (useful with
     * multi-homing or multicast addresses). Use port=0 for an ephemeral port.
     */
    void bind(L3Address localAddr, int localPort);

    /**
     * Invokes PASSIVE_OPEN and makes the server listen on the binded port.
     */
    void listen();

    /**
     * Connects to a remote QUIC socket. This has two effects:
     * (1) this socket will only receive packets from specified address/port,
     * and (2) you can use send() (as opposed to sendTo()) to send packets.
     */
    void connect(L3Address remoteAddr, int remotePort);

    /**
     * Sends a data packet to the address and port specified previously
     * in a connect() call.
     */
    void send(Packet *msg) override;

    /**
     * Sends control info command with expectedDataSize of given stream to QUIC.
     */
    void recv(QuicRecvCommand *ctrInfo);
    void recv(long length, long streamId);

    /**
     * Sends control info command with priority of given stream to QUIC.
     * Is only active when priority-based scheduling has been enabled.
     */

    //void sendRequest(cMessage *msg);

    /**
     * Send request.
     */

    //void setStreamPriority(uint64_t streamID, uint32_t priority);

    /**
     * Unbinds the socket. Once closed, a closed socket may be bound to another
     * (or the same) port, and reused.
     */
    void close() override;

    virtual bool isOpen() const override;
    virtual bool belongsToSocket(cMessage *msg) const override;
    virtual void destroy() override;
    void processMessage(cMessage *msg) override;
    int getSocketId() const override { return -1; }

    /**
     * Accepts a new incoming connection reported as available.
     */
    void accept(int socketId);

    void setCallback(ICallback *cb) {
        this->cb = cb;
    }
};

} // namespace inet

#endif // ifndef __INET_QUICSOCKET_H


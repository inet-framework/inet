//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_QUICSOCKET_H
#define __INET_QUICSOCKET_H

#include "inet/common/INETDefs.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/socket/ISocket.h"
#include "inet/networklayer/common/L3Address.h"
#include "QuicCommand_m.h"

namespace inet {

/**
 * QuicSocket is a convenience class, to make it easier to send and receive
 * QUIC packets from your application models. You'd have one (or more)
 * QuicSocket object(s) in your application simple module class, and call
 * its member functions (bind(), connect(), send(), etc.) to create and
 * configure a socket, and to send messages.
 *
 * QuicSocket chooses and remembers the socketId for you, assembles and sends
 * command packets such as QUIC_C_BIND to QUIC, and can also help you deal with
 * packets and notification messages arriving from QUIC.
 */
class INET_API QuicSocket : public ISocket
{
public:
    /**
     * Callback interface for QUIC sockets, see setCallback() and
     * processMessage() for more info.
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
         * Notifies that data is available to read from the QUIC module.
         *
         * @param socket The involved socket object.
         * @param dataInfo Information about the data, such as the length of
         * the data and the ID of the QUIC stream that contains the data.
         */
        virtual void socketDataAvailable(QuicSocket* socket, QuicDataInfo *dataInfo) = 0;

        /**
         * New data arrived at the application. Called after a recv call.
         *
         * @param socket The involved socket object.
         * @param packet The packet with the arrived data.
         */
        virtual void socketDataArrived(QuicSocket* socket, Packet *packet) = 0;

        /**
         * Notifies that a new connection is available to accept. Called after
         * a listen call and an incoming connection initiation.
         *
         * @param socket The involved socket object.
         */
        virtual void socketConnectionAvailable(QuicSocket *socket) = 0;

        /**
         * Notifies that the QUIC connection is established and ready to
         * transfer data.
         *
         * @param socket The involved socket object.
         */
        virtual void socketEstablished(QuicSocket *socket) = 0;

        /**
         * Notifies that the QUIC connection is closed.
         *
         * @param socket The involved socket object.
         */
        virtual void socketClosed(QuicSocket *socket) = 0;

        /**
         * Notifies that this QUIC socket is deleted.
         *
         * @param socket The involved socket object.
         */
        virtual void socketDeleted(QuicSocket *socket) = 0;

        /**
         * Notifies that QUIC's send queue is full and that the app should stop
         * sending data.
         *
         * @param socket The involved socket object.
         */
        virtual void socketSendQueueFull(QuicSocket *socket) = 0;

        /**
         * Notifies that QUIC's send queue is draining and that the app can
         * continue to send data.
         *
         * @param socket The involved socket object.
         */
        virtual void socketSendQueueDrain(QuicSocket *socket) = 0;

        /**
         * Notifies that QUIC had to reject a message. This happens if the app
         * sends data when QUIC's send queue is full.
         *
         * @param socket The involved socket object.
         */
        virtual void socketMsgRejected(QuicSocket *socket) = 0;
    };

    /**
     * States of a QUIC socket.
     */
    enum State { NOT_BOUND, BOUND, LISTENING, CONNECTING, CONNECTED, CLOSED };

private:
    int socketId;
    cGate *gateToQuic;

    ICallback *cb = nullptr;
    State socketState;

    L3Address localAddr;
    uint16_t localPort;
    L3Address remoteAddr;
    uint16_t remotePort;

    /**
     * Generates a new socket ID.
     *
     * @return The generated socket ID.
     */
    static int generateSocketId();

    /**
     * Sends the given message to the QUIC module.
     *
     * @param The message to send.
     */
    void sendToQuic(cMessage *msg);

public:
    /**
     * Creates a QuicSocket object and initiates the socket ID.
     */
    QuicSocket();

    /**
     * Runs before this socket is deleted. Calls the socketDeleted callback
     * function.
     */
    ~QuicSocket();

    /**
     * Sets the gate on which to send to QUIC. Must be invoked before socket
     * can be used. Example: <tt>socket.setOutputGate(gate("quicOut"));</tt>
     *
     * @param The gate to QUIC.
     */
    void setOutputGate(cGate *toQuic)
    {
        gateToQuic = toQuic;
    }

    /**
     * Binds the socket to a local IP address and port number.
     *
     * @param localAddr The local IP address.
     * @param localPort The local port number.
     */
    void bind(L3Address localAddr, uint16_t localPort);

    /**
     * Invokes PASSIVE_OPEN and makes the server listen on the binded port.
     */
    void listen();

    /**
     * Accepts a new connection that were initiated on this listening socket
     * and reported as available.
     *
     * @return The newly created socket for the accepted connection.
     */
    QuicSocket *accept();

    /**
     * Connects to a remote QUIC socket. This will initiate the QUIC connection
     * setup. After the connection reported as established, data can be sent
     * using the send method.
     *
     * @param remoteAddr The IP address of the remote end point.
     * @param remotePort The UDP port number of the remote end point.
     */
    void connect(L3Address remoteAddr, uint16_t remotePort);

    /**
     * Sends a message over the specified stream.
     *
     * @param msg The message to send.
     * @param streamId The ID of the QUIC stream to use for the transfer.
     */
    void send(Packet *msg, uint64_t streamId);

    /**
     * Calls send(msg, 0)
     *
     * @param msg The message to send.
     */
    void send(Packet *msg) override;

    /**
     * Requests to receive data with the specified length from the specified
     * stream from QUIC.
     *
     * @param length The length in bytes to receive from QUIC.
     * @param streamId The ID of the QUIC stream to receive data from.
     */
    void recv(int64_t length, uint64_t streamId);

    /**
     * Closes the socket and, if connected, the connection.
     */
    void close() override;

    /**
     * Checks if the socket is open.
     *
     * @return true if the socket is open, false otherwise.
     */
    virtual bool isOpen() const override;

    /**
     * Checks if the given message belongs to this socket.
     *
     * @param msg The message to check
     * @return true if the message belongs to this socket, false otherwise.
     */
    virtual bool belongsToSocket(cMessage *msg) const override;

    /**
     * Destroys the socket. Currently not implemented.
     */
    virtual void destroy() override;

    /**
     * Processes the given messages and, if a callback object is set, calls
     * the corresponding callback function.
     *
     * @param msg The message to process.
     */
    void processMessage(cMessage *msg) override;

    /**
     * @return The ID of this socket.
     */
    int getSocketId() const override {
        return socketId;
    }

    /**
     * Sets the callback object that provides the methods defined by ICallback.
     * If set, this socket calls the corresponding callback method when
     * processing a message.
     *
     * @param cb The object that provides the callback methods.
     */
    void setCallback(ICallback *cb) {
        this->cb = cb;
    }

    L3Address getLocalAddr() {
        return localAddr;
    }

    uint16_t getLocalPort() {
        return localPort;
    }

    L3Address getRemoteAddr() {
        return remoteAddr;
    }

    uint16_t getRemotePort() {
        return remotePort;
    }
};

} // namespace inet

#endif // ifndef __INET_QUICSOCKET_H


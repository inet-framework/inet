//
// Copyright (C) 2005,2011 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_UDPSOCKET_H
#define __INET_UDPSOCKET_H

#include <vector>

#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/socket/ISocket.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo.h"

namespace inet {

/**
 * UdpSocket is a convenience class, to make it easier to send and receive
 * UDP packets from your application models. You'd have one (or more)
 * UdpSocket object(s) in your application simple module class, and call
 * its member functions (bind(), connect(), sendTo(), etc.) to create and
 * configure a socket, and to send datagrams.
 *
 * UdpSocket chooses and remembers the socketId for you, assembles and sends command
 * packets such as UDP_C_BIND to UDP, and can also help you deal with packets and
 * notification messages arriving from UDP.
 *
 * Here is a code fragment that creates an UDP socket and sends a 1K packet
 * over it (the code can be placed in your handleMessage() or activity()):
 *
 * <pre>
 *   UdpSocket socket;
 *   socket.setOutputGate(gate("udpOut"));
 *   socket.connect(Address("10.0.0.2"), 2000);
 *
 *   cPacket *pk = new cPacket("dgram");
 *   pk->setByteLength(1024);
 *   socket.send(pk);
 *
 *   socket.close();
 * </pre>
 *
 * Processing messages sent up by the UDP module is relatively straightforward.
 * You only need to distinguish between data packets and error notifications,
 * by checking the message kind (should be either UDP_I_DATA or UDP_I_ERROR),
 * and casting the control info to UDPDataIndication or UdpErrorIndication.
 * USPSocket provides some help for this with the belongsToSocket() and
 * belongsToAnyUDPSocket() methods.
 */
class INET_API UdpSocket : public ISocket
{
  public:
    class INET_API ICallback {
      public:
        virtual ~ICallback() {}

        /**
         * Notifies about data arrival, packet ownership is transferred to the callee.
         */
        virtual void socketDataArrived(UdpSocket *socket, Packet *packet) = 0;

        /**
         * Notifies about error indication arrival, indication ownership is transferred to the callee.
         */
        virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) = 0;

        /**
         * Notifies about socket closed, indication ownership is transferred to the callee.
         */
        virtual void socketClosed(UdpSocket *socket) = 0;
    };
    enum State { CONNECTED, CLOSED };

  protected:
    int socketId;
    ICallback *cb = nullptr;
    void *userData = nullptr;
    cGate *gateToUdp = nullptr;
    State sockState = CLOSED;

  protected:
    void sendToUDP(cMessage *msg);

  public:
    /**
     * Constructor. The getSocketId() method returns a valid Id right after
     * constructor call.
     */
    UdpSocket();

    /**
     * Destructor
     */
    ~UdpSocket() {}

    void *getUserData() const { return userData; }
    void setUserData(void *userData) { this->userData = userData; }
    State getState() const { return sockState; }

    /**
     * Returns the internal socket Id.
     */
    int getSocketId() const override { return socketId; }

    /** @name Opening and closing connections, sending data */
    //@{

    /**
     * Sets the gate on which to send to UDP. Must be invoked before socket
     * can be used. Example: <tt>socket.setOutputGate(gate("udpOut"));</tt>
     */
    void setOutputGate(cGate *toUdp) { gateToUdp = toUdp; }

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
     * Connects to a remote UDP socket. This has two effects:
     * (1) this socket will only receive packets from specified address/port,
     * and (2) you can use send() (as opposed to sendTo()) to send packets.
     */
    void connect(L3Address remoteAddr, int remotePort);

    /**
     * Set the TTL (Ipv6: Hop Limit) field on sent packets.
     */
    void setTimeToLive(int ttl);

    /**
     * Sets the Ipv4 / Ipv6 dscp fields of packets
     * sent from the UDP socket.
     */
    void setDscp(short dscp);

    /**
     * Sets the Ipv4 Type of Service / Ipv6 Traffic Class fields of packets
     * sent from the UDP socket.
     */
    void setTos(short tos);

    /**
     * Set the Broadcast option on the UDP socket. This will cause the
     * socket to receive broadcast packets as well.
     */
    void setBroadcast(bool broadcast);

    /**
     * The boolean value specifies whether sent multicast packets should be
     * looped back to the local sockets (like the Unix IP_MULTICAST_LOOP
     * socket option).
     */
    void setMulticastLoop(bool value);

    /**
     * Set the output interface for sending multicast packets (like the Unix
     * IP_MULTICAST_IF socket option). The argument is the interface's ID in
     * InterfaceTable.
     */
    void setMulticastOutputInterface(int interfaceId);

    /**
     * Set the ReuseAddress option on the UDP socket. It is possible to use
     * an already bound address/port in the bind() command only if both the
     * original and the new socket set the ReuseAddress flag to true.
     * Note that only one socket will receive the packets arrived at that
     * address/port (the last bound one). This option works like REUSE_ADDR
     * socket option in Linux.
     */
    void setReuseAddress(bool value);

    /**
     * Adds the socket to the given multicast group, that is, UDP packets
     * arriving to the given multicast address will be passed up to the socket.
     * One can also optionally specify the output interface for packets sent to
     * that address.
     */
    void joinMulticastGroup(const L3Address& multicastAddr, int interfaceId = -1);

    /**
     * Joins the socket to each multicast group that are registered with
     * any of the interfaces.
     */
    void joinLocalMulticastGroups(MulticastGroupList mgl);

    /**
     * Causes the socket to leave the given multicast group, i.e. UDP packets
     * arriving to the given multicast address will no longer passed up to the socket.
     */
    void leaveMulticastGroup(const L3Address& multicastAddr);

    /**
     * Causes the socket to leave each multicast groups that are registered with
     * any of the interfaces.
     */
    void leaveLocalMulticastGroups(MulticastGroupList mgl);

    /**
     * Blocks multicast traffic of the specified group address from specific sources.
     * Use this method only if joinMulticastGroup() was previously called for that group.
     */
    void blockMulticastSources(int interfaceId, const L3Address& multicastAddr, const std::vector<L3Address>& sourceList);

    /**
     * Unblocks the multicast traffic of the specified group address from the specified sources.
     * Use this method only if the traffic was previously blocked by calling blockMulticastSources().
     */
    void unblockMulticastSources(int interfaceId, const L3Address& multicastAddr, const std::vector<L3Address>& sourceList);

    /**
     * Adds the socket to the given multicast group and source addresses, that is,
     * UDP packets arriving from one of the sources and to the given multicast address
     * will be passed up to the socket.
     */
    void joinMulticastSources(int interfaceId, const L3Address& multicastAddr, const std::vector<L3Address>& sourceList);

    /**
     * Causes the socket to leave the given multicast group for the specified sources,
     * i.e. UDP packets arriving from those sources to the given multicast address
     * will no longer passed up to the socket.
     */
    void leaveMulticastSources(int interfaceId, const L3Address& multicastAddr, const std::vector<L3Address>& sourceList);

    /**
     * Sets the source filter for the given multicast group.
     * If filterMode is INCLUDE, then UDP packets arriving from one of the sources and to
     * to given multicast group will be passed up to the socket. If filterMode is EXCLUDE,
     * then all UDP packets arriving to the given multicast group will be passed up except
     * those that arrive from the specified sources.
     */
    void setMulticastSourceFilter(int interfaceId, const L3Address& multicastAddr, UdpSourceFilterMode filterMode, const std::vector<L3Address>& sourceList);

    /**
     * Sends a data packet to the given address and port.
     * Additional options can be passed in a SendOptions struct.
     */
    void sendTo(Packet *msg, L3Address destAddr, int destPort);

    /**
     * Sends a data packet to the address and port specified previously
     * in a connect() call.
     */
    virtual void send(Packet *msg) override;

    /**
     * Unbinds the socket. Once closed, a closed socket may be bound to another
     * (or the same) port, and reused.
     */
    virtual void close() override;
    //@}

    virtual void destroy() override;

    /** @name Handling of messages arriving from UDP */
    //@{
    /**
     * Returns true if the message belongs to this socket instance (message
     * has a UdpControlInfo as getControlInfo(), and the socketId in it matches
     * that of the socket.)
     */
    virtual bool belongsToSocket(cMessage *msg) const override;

    /**
     * Sets a callback object, to be used with processMessage().
     * This callback object may be your simple module itself (if it
     * multiply inherits from ICallback too, that is you
     * declared it as
     * <pre>
     * class MyAppModule : public cSimpleModule, public UdpSocket::ICallback
     * </pre>
     * and redefined the necessary virtual functions; or you may use
     * dedicated class (and objects) for this purpose.
     *
     * UdpSocket doesn't delete the callback object in the destructor
     * or on any other occasion.
     */
    void setCallback(ICallback *cb);

    virtual void processMessage(cMessage *msg) override;

    virtual bool isOpen() const override { return sockState != CLOSED; }

    /**
     * Utility function: returns a line of information about a packet received via UDP.
     */
    static std::string getReceivedPacketInfo(Packet *pk);
    //@}
};

} // namespace inet

#endif


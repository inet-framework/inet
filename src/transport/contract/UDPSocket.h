//
// Copyright (C) 2005,2011 Andras Varga
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

#include "INETDefs.h"
#include "IPvXAddress.h"

class UDPDataIndication;

/**
 * UDPSocket is a convenience class, to make it easier to send and receive
 * UDP packets from your application models. You'd have one (or more)
 * UDPSocket object(s) in your application simple module class, and call
 * its member functions (bind(), connect(), sendTo(), etc.) to create and
 * configure a socket, and to send datagrams.
 *
 * UDPSocket chooses and remembers the sockId for you, assembles and sends command
 * packets such as UDP_C_BIND to UDP, and can also help you deal with packets and
 * notification messages arriving from UDP.
 *
 * Here is a code fragment that creates an UDP socket and sends a 1K packet
 * over it (the code can be placed in your handleMessage() or activity()):
 *
 * <pre>
 *   UDPSocket socket;
 *   socket.setOutputGate(gate("udpOut"));
 *   socket.connect(IPvXAddress("10.0.0.2"), 2000);
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
 * and casting the control info to UDPDataIndication or UDPErrorIndication.
 * USPSocket provides some help for this with the belongsToSocket() and
 * belongsToAnyUDPSocket() methods.
 */
class INET_API UDPSocket
{
  protected:
    int sockId;
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
     * Generates a new socket id.
     */
    static int generateSocketId();

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
     * Set the TTL (IPv6: Hop Limit) field on sent packets.
     */
    void setTimeToLive(int ttl);

    /**
     * Sets the IPv4 Type of Service / IPv6 Traffic Class fields of packets
     * sent from the UDP socket.
     */
    void setTypeOfService(unsigned char tos);

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
     * Adds the socket to the given multicast group, that is, UDP packets
     * arriving to the given multicast address will be passed up to the socket.
     * One can also optionally specify the output interface for packets sent to
     * that address.
     */
    void joinMulticastGroup(const IPvXAddress& multicastAddr, int interfaceId=-1);

    /**
     * Joins the socket to each multicast group that are registered with
     * any of the interfaces.
     */
    void joinLocalMulticastGroups();

    /**
     * Causes the socket to leave the given multicast group, i.e. UDP packets
     * arriving to the given multicast address will no longer passed up to the socket.
     */
    void leaveMulticastGroup(const IPvXAddress& multicastAddr);

    /**
     * Causes the socket to leave each multicast groups that are registered with
     * any of the interfaces.
     */
    void leaveLocalMulticastGroups();

    /**
     * Sends a data packet to the given address and port.
     */
    void sendTo(cPacket *msg, IPvXAddress destAddr, int destPort);

    /**
     * Sends a data packet to the given address and port using the provided
     * interface.
     */
    void sendTo(cPacket *msg, IPvXAddress destAddr, int destPort, int outInterface);

    /**
     * Sends a data packet to the address and port specified previously
     * in a connect() call.
     */
    void send(cPacket *msg);

    /**
     * Unbinds the socket. Once closed, a closed socket may be bound to another
     * (or the same) port, and reused.
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
     * Utility function: returns a line of information about a packet received via UDP.
     */
    static std::string getReceivedPacketInfo(cPacket *pk);
    //@}
};

#endif


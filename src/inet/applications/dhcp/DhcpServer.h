//
// Copyright (C) 2008 Juan-Carlos Maureira
// Copyright (C) INRIA
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_DHCPSERVER_H
#define __INET_DHCPSERVER_H

#include <map>

#include "inet/applications/base/ApplicationBase.h"
#include "inet/applications/dhcp/DhcpLease.h"
#include "inet/applications/dhcp/DhcpMessage_m.h"
#include "inet/networklayer/arp/ipv4/Arp.h"
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {

/**
 * Implements a DHCP server. See NED file for more details.
 */
class INET_API DhcpServer : public ApplicationBase, public cListener, public UdpSocket::ICallback
{
  protected:
    typedef std::map<Ipv4Address, DhcpLease> DhcpLeased;
    enum TimerType {
        START_DHCP
    };
    DhcpLeased leased; // lookup table for lease infos

    int numSent = 0; // num of sent UDP packets
    int numReceived = 0; // num of received UDP packets
    int serverPort = -1; // server port
    int clientPort = -1; // client port

    /* Set by management, see DhcpServer NED file. */
    unsigned int maxNumOfClients = 0;
    unsigned int leaseTime = 0;
    Ipv4Address subnetMask;
    Ipv4Address gateway;
    Ipv4Address ipAddressStart;

    NetworkInterface *ie = nullptr; // interface to serve DHCP requests on
    UdpSocket socket;
    simtime_t startTime; // application start time
    cMessage *startTimer = nullptr; // self message to start DHCP server

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    /*
     * Opens a UDP socket for client-server communication.
     */
    virtual void openSocket();

    /*
     * Performs a database lookup by MAC address for lease information.
     */
    virtual DhcpLease *getLeaseByMac(MacAddress mac);

    /*
     * Gets the next available lease to be assigned.
     */
    virtual DhcpLease *getAvailableLease(Ipv4Address requestedAddress, const MacAddress& clientMAC);

    /*
     * Implements the server's state machine.
     */
    virtual void processDhcpMessage(Packet *packet);

    /*
     * Send DHCPOFFER message to client in response to DHCPDISCOVER with offer of configuration
     * parameters.
     */
    virtual void sendOffer(DhcpLease *lease, const Ptr<const DhcpMessage>& dhcpMsg);

    /*
     * Send DHCPACK message to client with configuration parameters, including committed network address.
     */
    virtual void sendAck(DhcpLease *lease, const Ptr<const DhcpMessage>& dhcpMsg);

    /*
     * Send DHCPNAK message to client indicating client's notion of network address is incorrect
     * (e.g., client has moved to new subnet) or client's lease as expired.
     */
    virtual void sendNak(const Ptr<const DhcpMessage>& dhcpMsg);

    virtual void handleSelfMessages(cMessage *msg);
    virtual NetworkInterface *chooseInterface();
    virtual void sendToUDP(Packet *msg, int srcPort, const L3Address& destAddr, int destPort);

    // UdpSocket::ICallback methods
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

    /*
     * Signal handler for cObject, override cListener function.
     */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    // Lifecycle methods
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  public:
    DhcpServer();
    virtual ~DhcpServer();
};

} // namespace inet

#endif


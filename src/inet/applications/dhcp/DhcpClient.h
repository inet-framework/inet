//
// Copyright (C) 2008 Juan-Carlos Maureira
// Copyright (C) INRIA
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_DHCPCLIENT_H
#define __INET_DHCPCLIENT_H

#include "inet/applications/base/ApplicationBase.h"
#include "inet/applications/dhcp/DhcpLease.h"
#include "inet/applications/dhcp/DhcpMessage_m.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/networklayer/ipv4/Ipv4RoutingTable.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {

/**
 * Implements a DHCP client. See NED file for more details.
 */
class INET_API DhcpClient : public ApplicationBase, public cListener, public UdpSocket::ICallback
{
  protected:

    // DHCP client states (RFC 2131, Figure 5: state transition diagram)
    enum ClientState {
        IDLE, INIT, INIT_REBOOT, REBOOTING, SELECTING, REQUESTING, BOUND, RENEWING, REBINDING
    };

    // parameters
    int serverPort = -1;
    int clientPort = -1;
    UdpSocket socket; // UDP socket for client-server communication
    simtime_t startTime; // application start time
    MacAddress macAddress; // client's MAC address
    cModule *host = nullptr; // containing host module (@networkNode)
    NetworkInterface *ie = nullptr; // interface to configure
    ModuleRefByPar<IIpv4RoutingTable> irt; // routing table to update

    // state
    cMessage *timerT1 = nullptr; // time at which the client enters the RENEWING state
    cMessage *timerT2 = nullptr; // time at which the client enters the REBINDING state
    cMessage *timerTo = nullptr; // response timeout: WAIT_ACK, WAIT_OFFER
    cMessage *leaseTimer = nullptr; // length of time the lease is valid
    cMessage *startTimer = nullptr; // self message to start DHCP initialization
    ClientState clientState = INIT; // current state
    unsigned int xid = 0; // transaction id; to associate messages and responses between a client and a server
    DhcpLease *lease = nullptr; // leased IP information
    Ipv4Route *route = nullptr; // last added route

    // statistics
    int numSent = 0; // number of sent DHCP messages
    int numReceived = 0; // number of received DHCP messages
    int responseTimeout = 0; // timeout waiting for DHCPACKs, DHCPOFFERs

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void scheduleTimerTO(DhcpTimerType type);
    virtual void scheduleTimerT1();
    virtual void scheduleTimerT2();
    static const char *getStateName(ClientState state);
    const char *getAndCheckMessageTypeName(DhcpMessageType type);
    virtual void refreshDisplay() const override;

    /*
     * Opens a UDP socket for client-server communication.
     */
    virtual void openSocket();

    /*
     * Handles incoming DHCP messages, and implements the
     * state-transition diagram for DHCP clients.
     */
    virtual void handleDhcpMessage(Packet *packet);

    /*
     * Performs state changes and requests according to the timer expiration events
     * (e. g. retransmits DHCPREQUEST after the WAIT_ACK timeout expires).
     */
    virtual void handleTimer(cMessage *msg);

    /*
     * Signal handler for cObject, override cListener function.
     */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    /*
     * Performs UDP transmission.
     */
    virtual void sendToUdp(Packet *msg, int srcPort, const L3Address& destAddr, int destPort);

    /*
     * Client broadcast to locate available servers.
     */
    virtual void sendDiscover();

    /*
     * Client message to servers either (a) requesting offered parameters
     * from one server and implicitly declining offers from all others,
     * (b) confirming correctness of previously allocated address after,
     * e.g., system reboot, or (c) extending the lease on a particular
     * network address.
     */
    virtual void sendRequest();

    /*
     * Client to server indicating network address is already in use.
     */
    virtual void sendDecline(Ipv4Address declinedIp);

    /*
     * Records configuration parameters from a DHCPACK message.
     */
    virtual void recordLease(const Ptr<const DhcpMessage>& dhcpACK);

    /*
     * Records minimal configuration parameters from a DHCPOFFER message.
     */
    virtual void recordOffer(const Ptr<const DhcpMessage>& dhcpOffer);

    /*
     * Assigns the IP address to the interface.
     */
    virtual void bindLease();

    /*
     * Removes the configured IP address (e. g. when clients get a DHCPNAK message in REBINDING or RENEWING states
     * or the lease time expires).
     */
    virtual void unbindLease();

    /*
     * Starts the DHCP configuration process with sending a DHCPDISCOVER.
     */
    virtual void initClient();

    /*
     * Starts the DHCP configuration process with known network address.
     */
    virtual void initRebootedClient();

    /*
     * Handles DHCPACK in any state. Note that, handleDHCPACK() doesn't handle DHCPACK messages
     * in response to DHCPINFORM messages.
     */
    virtual void handleDhcpAck(const Ptr<const DhcpMessage>& msg);

    /*
     * Selects the first non-loopback interface
     */
    virtual NetworkInterface *chooseInterface();

    // UdpSocket::ICallback methods
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

    // Lifecycle methods
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  public:
    DhcpClient() {}
    virtual ~DhcpClient();
};

} // namespace inet

#endif


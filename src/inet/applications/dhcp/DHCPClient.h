//
// Copyright (C) 2008 Juan-Carlos Maureira
// Copyright (C) INRIA
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_DHCPCLIENT_H
#define __INET_DHCPCLIENT_H

#include <vector>
#include "inet/linklayer/common/MACAddress.h"
#include "inet/applications/dhcp/DHCPMessage_m.h"
#include "inet/applications/dhcp/DHCPLease.h"
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/networklayer/ipv4/IPv4RoutingTable.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"
#include "inet/common/lifecycle/ILifecycle.h"

namespace inet {

/**
 * Implements a DHCP client. See NED file for more details.
 */
class INET_API DHCPClient : public cSimpleModule, public cListener, public ILifecycle
{
  protected:
    // DHCP timer types (RFC 2131 4.4.5)
    enum TimerType {
        WAIT_OFFER, WAIT_ACK, T1, T2, LEASE_TIMEOUT, START_DHCP
    };

    // DHCP client states (RFC 2131, Figure 5: state transition diagram)
    enum ClientState {
        IDLE, INIT, INIT_REBOOT, REBOOTING, SELECTING, REQUESTING, BOUND, RENEWING, REBINDING
    };

    // parameters
    int serverPort = -1;
    int clientPort = -1;
    UDPSocket socket;    // UDP socket for client-server communication
    simtime_t startTime;    // application start time
    MACAddress macAddress;    // client's MAC address
    cModule *host = nullptr;    // containing host module (@networkNode)
    InterfaceEntry *ie = nullptr;    // interface to configure
    IIPv4RoutingTable *irt = nullptr;    // routing table to update

    // state
    cMessage *timerT1 = nullptr;    // time at which the client enters the RENEWING state
    cMessage *timerT2 = nullptr;    // time at which the client enters the REBINDING state
    cMessage *timerTo = nullptr;    // response timeout: WAIT_ACK, WAIT_OFFER
    cMessage *leaseTimer = nullptr;    // length of time the lease is valid
    cMessage *startTimer = nullptr;    // self message to start DHCP initialization
    bool isOperational = false;    // lifecycle
    ClientState clientState = INIT;    // current state
    unsigned int xid = 0;    // transaction id; to associate messages and responses between a client and a server
    DHCPLease *lease = nullptr;    // leased IP information
    IPv4Route *route = nullptr;    // last added route

    // statistics
    int numSent = 0;    // number of sent DHCP messages
    int numReceived = 0;    // number of received DHCP messages
    int responseTimeout = 0;    // timeout waiting for DHCPACKs, DHCPOFFERs

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void scheduleTimerTO(TimerType type);
    virtual void scheduleTimerT1();
    virtual void scheduleTimerT2();
    static const char *getStateName(ClientState state);
    const char *getAndCheckMessageTypeName(DHCPMessageType type);
    virtual void updateDisplayString();

    /*
     * Opens a UDP socket for client-server communication.
     */
    virtual void openSocket();

    /*
     * Handles incoming DHCP messages, and implements the
     * state-transition diagram for DHCP clients.
     */
    virtual void handleDHCPMessage(DHCPMessage *msg);

    /*
     * Performs state changes and requests according to the timer expiration events
     * (e. g. retransmits DHCPREQUEST after the WAIT_ACK timeout expires).
     */
    virtual void handleTimer(cMessage *msg);

    /*
     * Signal handler for cObject, override cListener function.
     */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG) override;

    /*
     * Performs UDP transmission.
     */
    virtual void sendToUDP(cPacket *msg, int srcPort, const L3Address& destAddr, int destPort);

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
    virtual void sendDecline(IPv4Address declinedIp);

    /*
     * Records configuration parameters from a DHCPACK message.
     */
    virtual void recordLease(DHCPMessage *dhcpACK);

    /*
     * Records minimal configuration parameters from a DHCPOFFER message.
     */
    virtual void recordOffer(DHCPMessage *dhcpOffer);

    /*
     * Assigns the IP address to the interface.
     */
    virtual void bindLease();

    /*
     * Removes the configured IP address (e. g. when clients get a DHCPNAK message in REBINDING or RENEWING states
     * or the lease time expires).
     */
    virtual void unboundLease();

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
    virtual void handleDHCPACK(DHCPMessage *msg);

    /*
     * Selects the first non-loopback interface
     */
    virtual InterfaceEntry *chooseInterface();

    // Lifecycle methods
    virtual void startApp();
    virtual void stopApp();
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

  public:
    DHCPClient() {}
    virtual ~DHCPClient();
};

} // namespace inet

#endif // ifndef __INET_DHCPCLIENT_H


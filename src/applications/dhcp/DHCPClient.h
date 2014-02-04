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

#ifndef INET_DHCPCLIENT_H__
#define INET_DHCPCLIENT_H__

#include <vector>
#include "NotificationBoard.h"
#include "MACAddress.h"
#include "DHCPMessage_m.h"
#include "DHCPLease.h"
#include "InterfaceTable.h"
#include "RoutingTable.h"
#include "UDPSocket.h"
#include "INotifiable.h"
#include "ILifecycle.h"

/**
 * Implements a DHCP client. See NED file for more details.
 */
class INET_API DHCPClient : public cSimpleModule, public INotifiable, public ILifecycle
{
    protected:
        int serverPort;
        int clientPort;
        UDPSocket socket; // UDP socket for client-server communication
        bool isOperational; // lifecycle
        cMessage* timerT1; // time at which the client enters the RENEWING state
        cMessage* timerT2; // time at which the client enters the REBINDING state
        cMessage* timerTo; // response timeout: WAIT_ACK, WAIT_OFFER
        cMessage* leaseTimer; // length of time the lease is valid
        cMessage* startTimer; // self message to start DHCP initialization

        // DHCP timer types (RFC 2131 4.4.5)
        enum TimerType
        {
            WAIT_OFFER, WAIT_ACK, T1, T2, LEASE_TIMEOUT, START_DHCP
        };

        // DHCP client states (RFC 2131, Figure 5: state transition diagram)
        enum ClientState
        {
            IDLE, INIT, INIT_REBOOT, REBOOTING, SELECTING, REQUESTING, BOUND, RENEWING, REBINDING
        };

        std::string hostName;
        int numSent; // number of sent DHCP messages
        int numReceived; // number of received DHCP messages
        int responseTimeout; // timeout waiting for DHCPACKs, DHCPOFFERs
        unsigned int xid; // transaction id; to associate messages and responses between a client and a server
        ClientState clientState; // current state
        simtime_t startTime; // application start time

        MACAddress macAddress; // client's MAC address
        NotificationBoard * nb; // notification board
        InterfaceEntry *ie; // interface to configure
        IRoutingTable *irt; // routing table to update
        DHCPLease *lease; // leased IP information
        IPv4Route *route; // last added route
    protected:
        // Simulation methods.
        virtual int numInitStages() const { return 4; }
        virtual void initialize(int stage);
        virtual void finish();
        virtual void handleMessage(cMessage * msg);

        /*
         * Opens a UDP socket for client-server communication.
         */
        virtual void openSocket();

        /*
         * Handles incoming DHCP messages, and implements the
         * state-transition diagram for DHCP clients.
         */
        virtual void handleDHCPMessage(DHCPMessage * msg);

        /*
         * Performs state changes and requests according to the timer expiration events
         * (e. g. retransmits DHCPREQUEST after the WAIT_ACK timeout expires).
         */
        virtual void handleTimer(cMessage * msg);


        virtual void receiveChangeNotification(int category, const cPolymorphic * details);

        /*
         * Performs UDP transmission.
         */
        virtual void sendToUDP(cPacket * msg, int srcPort, const IPvXAddress& destAddr, int destPort);

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
        virtual void recordLease(DHCPMessage * dhcpACK);

        /*
         * Records minimal configuration parameters from a DHCPOFFER message.
         */
        virtual void recordOffer(DHCPMessage * dhcpOffer);

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
        virtual void handleDHCPACK(DHCPMessage * msg);

        virtual InterfaceEntry *chooseInterface();
        virtual void scheduleTimerTO(TimerType type);
        virtual void scheduleTimerT1();
        virtual void scheduleTimerT2();
        static const char *getStateName(ClientState state);
        const char *getAndCheckMessageTypeName(DHCPMessageType type);
        virtual void updateDisplayString();
        virtual void startApp();
        virtual void stopApp();

    public:
        DHCPClient();
        virtual ~DHCPClient();

        virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);
};

#endif


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

#ifndef INET_DHCPSERVER_H__
#define INET_DHCPSERVER_H__

#include <vector>
#include <map>

#include "INETDefs.h"

#include "DHCPMessage_m.h"
#include "DHCPLease.h"
#include "InterfaceTable.h"
#include "ARP.h"
#include "UDPSocket.h"

class NotificationBoard;

/**
 * Implements a DHCP server. See NED file for more details.
 */
class INET_API DHCPServer : public cSimpleModule, public INotifiable, public ILifecycle
{
    protected:
        typedef std::map<IPv4Address, DHCPLease> DHCPLeased;
        enum TimerType
        {
            START_DHCP
        };
        DHCPLeased leased; // lookup table for lease infos

        bool isOperational; // lifecycle
        int numSent; // num of sent UDP packets
        int numReceived; // num of received UDP packets
        int serverPort; // server port
        int clientPort; // client port

        /* Set by management, see DHCPServer NED file. */
        unsigned int maxNumOfClients;
        unsigned int leaseTime;
        IPv4Address subnetMask;
        IPv4Address gateway;
        IPv4Address ipAddressStart;

        InterfaceEntry *ie; // interface to serve DHCP requests on
        NotificationBoard *nb;
        UDPSocket socket;
        simtime_t startTime; // application start time
        cMessage *startTimer; // self message to start DHCP server

    protected:
        virtual int numInitStages() const { return 4; }
        virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg);

        /*
         * Opens a UDP socket for client-server communication.
         */
        virtual void openSocket();

        /*
         * Performs a database lookup by MAC address for lease information.
         */
        virtual DHCPLease* getLeaseByMac(MACAddress mac);

        /*
         * Gets the next available lease to be assigned.
         */
        virtual DHCPLease* getAvailableLease(IPv4Address requestedAddress, MACAddress& clientMAC);

        /*
         * Implements the server's state machine.
         */
        virtual void processDHCPMessage(DHCPMessage *packet);

        /*
         * Send DHCPOFFER message to client in response to DHCPDISCOVER with offer of configuration
         * parameters.
         */
        virtual void sendOffer(DHCPLease* lease);

        /*
         * Send DHCPACK message to client with configuration parameters, including committed network address.
         */
        virtual void sendACK(DHCPLease* lease, DHCPMessage * packet);

        /*
         * Send DHCPNAK message to client indicating client's notion of network address is incorrect
         * (e.g., client has moved to new subnet) or client's lease as expired.
         */
        virtual void sendNAK(DHCPMessage* msg);

        virtual void handleSelfMessages(cMessage * msg);
        virtual InterfaceEntry *chooseInterface();
        virtual void sendToUDP(cPacket *msg, int srcPort, const IPvXAddress& destAddr, int destPort);
        virtual void receiveChangeNotification(int category, const cPolymorphic *details);
        virtual void startApp();
        virtual void stopApp();

    public:
        DHCPServer();
        virtual ~DHCPServer();

        /*
         * For lifecycle management.
         */
        virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);
};

#endif


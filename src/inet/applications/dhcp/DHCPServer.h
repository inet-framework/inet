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

#ifndef __INET_DHCPSERVER_H
#define __INET_DHCPSERVER_H

#include <vector>
#include <map>

#include "inet/common/INETDefs.h"

#include "inet/applications/dhcp/DHCPMessage_m.h"
#include "inet/applications/dhcp/DHCPLease.h"
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/networklayer/arp/ipv4/ARP.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"

namespace inet {

/**
 * Implements a DHCP server. See NED file for more details.
 */
class INET_API DHCPServer : public cSimpleModule, public cListener, public ILifecycle
{
  protected:
    typedef std::map<IPv4Address, DHCPLease> DHCPLeased;
    enum TimerType {
        START_DHCP
    };
    DHCPLeased leased;    // lookup table for lease infos

    bool isOperational = false;    // lifecycle
    int numSent = 0;    // num of sent UDP packets
    int numReceived = 0;    // num of received UDP packets
    int serverPort = -1;    // server port
    int clientPort = -1;    // client port

    /* Set by management, see DHCPServer NED file. */
    unsigned int maxNumOfClients = 0;
    unsigned int leaseTime = 0;
    IPv4Address subnetMask;
    IPv4Address gateway;
    IPv4Address ipAddressStart;

    InterfaceEntry *ie = nullptr;    // interface to serve DHCP requests on
    UDPSocket socket;
    simtime_t startTime;    // application start time
    cMessage *startTimer = nullptr;    // self message to start DHCP server

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

    /*
     * Opens a UDP socket for client-server communication.
     */
    virtual void openSocket();

    /*
     * Performs a database lookup by MAC address for lease information.
     */
    virtual DHCPLease *getLeaseByMac(MACAddress mac);

    /*
     * Gets the next available lease to be assigned.
     */
    virtual DHCPLease *getAvailableLease(IPv4Address requestedAddress, MACAddress& clientMAC);

    /*
     * Implements the server's state machine.
     */
    virtual void processDHCPMessage(DHCPMessage *packet);

    /*
     * Send DHCPOFFER message to client in response to DHCPDISCOVER with offer of configuration
     * parameters.
     */
    virtual void sendOffer(DHCPLease *lease);

    /*
     * Send DHCPACK message to client with configuration parameters, including committed network address.
     */
    virtual void sendACK(DHCPLease *lease, DHCPMessage *packet);

    /*
     * Send DHCPNAK message to client indicating client's notion of network address is incorrect
     * (e.g., client has moved to new subnet) or client's lease as expired.
     */
    virtual void sendNAK(DHCPMessage *msg);

    virtual void handleSelfMessages(cMessage *msg);
    virtual InterfaceEntry *chooseInterface();
    virtual void sendToUDP(cPacket *msg, int srcPort, const L3Address& destAddr, int destPort);

    /*
     * Signal handler for cObject, override cListener function.
     */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG) override;

    virtual void startApp();
    virtual void stopApp();
    /*
     * For lifecycle management.
     */
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

  public:
    DHCPServer();
    virtual ~DHCPServer();
};

} // namespace inet

#endif // ifndef __INET_DHCPSERVER_H


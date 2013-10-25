//
// Copyright (C) 2008 Juan-Carlos Maureira
// Copyright (C) INRIA
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

#ifndef __DHCPSERVER_H__
#define __DHCPSERVER_H__

#include <vector>
#include <map>
#include "INETDefs.h"
#include "DHCP_m.h"
#include "DHCPOptions.h"
#include "DHCPLease.h"
#include "InterfaceTable.h"
#include "ARP.h"
#include "UDPSocket.h"


class INET_API DHCPServer : public cSimpleModule, public cListener, public ILifecycle
{
    protected:
        // Transmission Timer
        enum TIMER_TYPE
        {
            PROC_DELAY
        };

        // list of leased ip
        typedef std::map<IPv4Address, DHCPLease> DHCPLeased;
        DHCPLeased leased;

        std::vector<cMessage *> messagesBeingProcessed;

        int numSent;
        int numReceived;

        int bootps_port; // server
        int bootpc_port; // client

        simtime_t proc_delay; // process delay

        InterfaceEntry* ie; // interface to listen
        UDPSocket socket;

    protected:
        virtual int numInitStages() const { return NUM_INIT_STAGES; }
        virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg);
        virtual void handleIncomingPacket(DHCPMessage *pkt);
        virtual void openSocket();

        virtual void cancelMessagesBeingProcessed();
        // search for a mac into the leased ip
        virtual DHCPLease* getLeaseByMac(MACAddress mac);
        // get the next available lease to be assigned
        virtual DHCPLease* getAvailableLease();
        virtual void handleTimer(cMessage *msg);
        virtual void processPacket(DHCPMessage *packet);
        virtual void sendOffer(DHCPLease* lease);
        virtual void sendACK(DHCPLease* lease);
        virtual void sendToUDP(cPacket *msg, int srcPort, const Address& destAddr, int destPort);

    public:
        DHCPServer();
        virtual ~DHCPServer();

        virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);
        virtual void receiveSignal(cComponent *source, simsignal_t category, cObject *details);
};

#endif


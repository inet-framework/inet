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

#ifndef __DHCPCLIENT_H__
#define __DHCPCLIENT_H__

#include <vector>
#include "NotificationBoard.h"
#include "MACAddress.h"
#include "DHCP_m.h"
#include "DHCPLease.h"
#include "InterfaceTable.h"
#include "RoutingTable.h"
#include "UDPSocket.h"
#include "INotifiable.h"

class INET_API DHCPClient : public cSimpleModule, public INotifiable
{
    protected:
        int bootps_port; // server
        int bootpc_port; // client
        UDPSocket socket;

        cMessage* timer_t1;
        cMessage* timer_t2;
        cMessage* timer_to; // response timeout

        enum TIMER_TYPE
        {
            WAIT_OFFER, WAIT_ACK, T1, T2
        }; // we discard the init-reboot and rebooting states.
           // selecting state waits for the first offering and moves on
        enum CLIENT_STATE
        {
            IDLE, INIT, SELECTING, REQUESTING, BOUND, RENEWING, REBINDING
        };

        std::string host_name;

        int numSent;
        int numReceived;

        int retry_count;
        int retry_max;
        int response_timeout;
        long xid; // transaction id;
        CLIENT_STATE client_state;

        MACAddress client_mac_address; // client_mac_address

        NotificationBoard* nb; // Notification board
        InterfaceEntry* ie; // interface to configure
        IRoutingTable* irt; // Routing table to update

        DHCPLease* lease; // leased ip information

    protected:
        // simulation method
        virtual int numInitStages() const
        {
            return 4;
        }
        virtual void initialize(int stage);
        virtual void finish();
        virtual void handleMessage(cMessage *msg);
        virtual void handleTimer(cMessage *msg);
        virtual void handleDHCPMessage(DHCPMessage* msg);
        virtual void receiveChangeNotification(int category, const cPolymorphic *details);
        virtual cModule* getContainingNode();
        virtual void sendToUDP(cPacket *msg, int srcPort, const IPvXAddress& destAddr, int destPort);

        // internal client methods
        virtual void changeFSMState(CLIENT_STATE new_state);
        virtual void cancelTimer_T1();
        virtual void cancelTimer_T2();
        virtual void cancelTimer_TO();
        virtual void sendDiscover();
        virtual void sendRequest();

        virtual void scheduleTimer_TO(TIMER_TYPE type);
        virtual void scheduleTimer_T1();
        virtual void scheduleTimer_T2();

    private:
        // utils methods
        virtual void cancelTimer(cMessage* timer);

    public:
        DHCPClient();
        ~DHCPClient();
};

#endif


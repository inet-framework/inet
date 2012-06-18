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

#include "DHCPClient.h"
#include "NotifierConsts.h"
#include "InterfaceTableAccess.h"
#include "RoutingTableAccess.h"
#include "IPv4InterfaceData.h"

Define_Module(DHCPClient);

void DHCPClient::initialize(int stage)
{
    if (stage != 3)
    {
        this->timer_t1 = NULL;
        this->timer_t2 = NULL;
        this->timer_to = NULL;
        return;
    }

    numSent = 0;
    numReceived = 0;
    retry_count = 0;
    xid = 0;

    retry_max = 10; // Resent attempts
    response_timeout = 1; // response timeout in seconds;

    WATCH(numSent);
    WATCH(numReceived);
    WATCH(retry_count);
    WATCH(client_state);
    WATCH(xid);

    // DHCP UDP ports
    bootpc_port = 68; // client
    bootps_port = 67; // server

    // get the hostname
    cModule* host = findHost();
    if (host != NULL)
    {
        this->host_name = host->getName();
    }

    nb = NotificationBoardAccess().get();
    if (nb == NULL)
    {
        error("Notification board not found. DHCP Client needs a notification board to know when the link is done.");
    }

    // for a wireless interface subscribe the association event to start the DHCP protocol
    nb->subscribe(this, NF_L2_ASSOCIATED);

    // Get the interface to configure
    IInterfaceTable* ift = InterfaceTableAccess().get();
    this->ie = ift->getInterfaceByName(this->par("iface"));

    if (this->ie == NULL)
    {
        error("DHCP Interface does not exist. aborting.");
        return;
    }

    // get the routing table to update and subscribe it to the blackboard
    this->irt = RoutingTableAccess().get();

    // grab the interface mac address
    this->client_mac_address = ie->getMacAddress();

    // bind the client to the udp port
    socket.setOutputGate(gate("udpOut"));
    socket.bind(bootpc_port);
    socket.setBroadcast(true);
    ev << "DHCP Client bond to port " << bootpc_port << " at " << ie->getName() <<  endl;

    // set client to idle state
    this->client_state = IDLE;
}

void DHCPClient::changeFSMState(CLIENT_STATE new_state)
{
    this->client_state = new_state;
    if (new_state == INIT)
    {

        this->cancelTimer_T1();
        this->cancelTimer_T2();
        this->cancelTimer_TO();

        this->changeFSMState(SELECTING);
    }

    if (new_state == SELECTING)
    {
        // the selected lease is in this->lease
        this->sendDiscover();
        this->scheduleTimer_TO(WAIT_OFFER);
    }

    if (new_state == REQUESTING)
    {
        // the selected lease is in this->lease
        this->sendRequest();
        this->scheduleTimer_TO(WAIT_ACK);
    }

    if (new_state == BOUND)
    {
        this->cancelTimer_TO();
        this->scheduleTimer_T1();
        this->scheduleTimer_T2();

        // Assign the IP to the interface
        this->ie->ipv4Data()->setIPAddress(this->lease->ip);
        this->ie->ipv4Data()->setNetmask(this->lease->netmask);

        std::string banner = "Got IP " + this->lease->ip.str();
        this->findHost()->bubble(banner.c_str());

        EV
                << "Configuring interface : " << this->ie->getName() << " ip:" << this->lease->ip << "/"
                        << this->lease->netmask << " leased time: " << lease->lease_time << " (segs)" << endl;
        std::cout << "host " << this->host_name << " got ip" << endl;

        IPv4Route *iroute = NULL;
        for (int i=0;i<this->irt->getNumRoutes();i++)
        {
            IPv4Route * e = this->irt->getRoute(i);
            if (routeMatches(e, IPv4Address(), IPv4Address(), lease->gateway, 0, (char*) (this->ie->getName())))
            {
                iroute =  e;
                break;
            }
        }
        if (iroute == NULL)
        {
            // create gateway route
            IPv4Route *e = new IPv4Route();
            e->setDestination(IPv4Address());
            e->setNetmask(IPv4Address());
            e->setGateway(lease->gateway);
            e->setInterface(this->ie);
            e->setSource(IPv4Route::MANUAL);
            this->irt->addRoute(e);
        }
        // update the routing table
        this->nb->fireChangeNotification(NF_INTERFACE_IPv4CONFIG_CHANGED, NULL);
        EV << "publishing the configuration change into the blackboard" << endl;
    }

    if (new_state == RENEWING)
    {
        // asking for lease renewal
        this->sendRequest();
        this->scheduleTimer_TO(WAIT_ACK);
    }

    if (new_state == REBINDING)
    {
        // asking for lease rebinding
        this->cancelTimer_T1();
        this->cancelTimer_T2();
        this->cancelTimer_TO();

        this->sendRequest();
        this->scheduleTimer_TO(WAIT_ACK);
    }

}

void DHCPClient::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        this->handleTimer(msg);
    }
    else if (msg->arrivedOn("udpIn"))
    {
        // check if the message is DHCPMessage
        if (dynamic_cast<DHCPMessage*>(msg))
        {
            this->handleDHCPMessage(check_and_cast<DHCPMessage*>(msg));
        }
        else
        {
            EV << "unknown packet, discarding it" << endl;
        }
    }
    // delete the msg
    delete msg;
}

void DHCPClient::handleTimer(cMessage* msg)
{
    int category = msg->getKind();

    if (category == WAIT_OFFER)
    {
        if (retry_count < retry_max)
        {
            this->changeFSMState(SELECTING);
            retry_count++;
        }
        else
        {
            ev << "No DHCP offer. restarting. " << endl;
            retry_count = 0;
            this->changeFSMState(INIT);
        }
    }

    if (category == WAIT_ACK)
    {
        if (retry_count < retry_max)
        {
            // trigger the current state one more time
            retry_count++;
        }
        else
        {
            ev << "No DHCP ACK. restarting. " << endl;
            retry_count = 0;
        }
        if (this->client_state == REQUESTING)
        {
            this->changeFSMState(REQUESTING);
        }
        if (this->client_state == RENEWING)
        {
            this->changeFSMState(RENEWING);
        }
        if (this->client_state == REBINDING)
        {
            this->changeFSMState(REBINDING);
        }
    }

    if (category == T1)
    {
        ev << "T1 reached. starting RENEWING state " << endl;
        this->changeFSMState(RENEWING);
    }

    if (category == T2 && this->client_state == RENEWING)
    {
        ev << "T2 reached. starting REBINDING state " << endl;
        this->changeFSMState(REBINDING);
    }

}

void DHCPClient::handleDHCPMessage(DHCPMessage* msg)
{
    EV << "arrived DHCP message:" << msg << endl;

    // check the packet type (reply) and transaction id
    if (msg->getOp() == BOOTREPLY && msg->getXid() == this->xid)
    {
        // bootreply  is 4 us.
        if (msg->getOptions().get(DHCP_MSG_TYPE) == DHCPOFFER && this->client_state == SELECTING)
        {

            // the offering to our discover arrived
            if (!msg->getYiaddr().isUnspecified())
            {
                IPv4Address ip = msg->getYiaddr();
                EV << "DHCPOFFER arrived" << endl;

                Byte server_id_b = msg->getOptions().get(SERVER_ID);
                IPv4Address server_id;
                if (server_id_b.stringValue() != "")
                {
                    server_id = IPv4Address(server_id_b.stringValue().c_str());
                }

                // minimal information to configure the interface
                if (!ip.isUnspecified())
                {
                    // create the lease to request
                    this->lease = new DHCPLease();
                    this->lease->ip = ip;
                    this->lease->mac = this->client_mac_address;
                    this->lease->server_id = server_id;
                }
                // start requesting
                this->changeFSMState(REQUESTING);

            }
            else
            {
                EV << "DHCPOFFER arrived, but no ip address has been offered. discarding it and remains in SELECTING." << endl;
            }
        }

        // check if the msg is DHCP ACK and we are waiting the ACK
        if (msg->getOptions().get(DHCP_MSG_TYPE) == DHCPACK)
        {

            if (!msg->getYiaddr().isUnspecified())
            {
                IPv4Address ip = msg->getYiaddr();
                EV << "DHCPACK arrived" << endl << "IP: " << ip << endl;

                // extract the server_id
                Byte server_id_b = msg->getOptions().get(SERVER_ID);
                IPv4Address server_id;
                if (server_id_b.stringValue() != "")
                {
                    server_id = IPv4Address(server_id_b.stringValue().c_str());
                }

                Byte netmask_b(msg->getOptions().get(SUBNET_MASK));
                Byte gateway_b(msg->getOptions().get(ROUTER));
                Byte dns_b(msg->getOptions().get(ROUTER));
                Byte ntp_b(msg->getOptions().get(NTP_SRV));

                IPv4Address netmask;
                IPv4Address gateway;
                IPv4Address dns;
                IPv4Address ntp;

                if (netmask_b.intValue() > 0)
                {
                    this->lease->netmask = IPv4Address(netmask_b.stringValue().c_str());
                }

                if (gateway_b.intValue() > 0)
                {
                    this->lease->gateway = IPv4Address(gateway_b.stringValue().c_str());
                }
                if (dns_b.intValue() > 0)
                {
                    this->lease->dns = IPv4Address(dns_b.stringValue().c_str());
                }
                if (ntp_b.intValue() > 0)
                {
                    this->lease->ntp = IPv4Address(ntp_b.stringValue().c_str());
                }

                this->lease->lease_time = msg->getOptions().get(LEASE_TIME);
                this->lease->renewal_time = msg->getOptions().get(RENEWAL_TIME);
                this->lease->rebind_time = msg->getOptions().get(REBIND_TIME);

                // starting BOUND
                this->changeFSMState(BOUND);
            }
            else
            {
                EV << "DHCPACK arrived, but no ip confirmed." << endl;
            }
        }
    }
    else
    {
        EV << "dhcp message is not 4 us. discarding it" << endl;
    }
}

void DHCPClient::receiveChangeNotification(int category, const cPolymorphic *details)
{
    Enter_Method_Silent();
    printNotificationBanner(category, details);

    // host associated. Link is up. Change the state to init.
    if (category == NF_L2_ASSOCIATED)
    {
        InterfaceEntry * ie = NULL;
        if (details)
        {
            cPolymorphic *detailsAux = const_cast<cPolymorphic*>(details);
            ie = dynamic_cast<InterfaceEntry*>(detailsAux);
        }
        if (!ie || (ie && (ie == this->ie)))
        {
            EV << "Interface Associated, starting DHCP" << endl;
            this->changeFSMState(INIT);
        }
    }
}

void DHCPClient::sendRequest()
{

    // setting the xid
    this->xid = intuniform(0, RAND_MAX); //random();

    DHCPMessage* request = new DHCPMessage("DHCPREQUEST");
    request->setOp(BOOTREQUEST);
    request->setByteLength(280); // DHCP request packet size;
    request->setHtype(1); // Ethernet
    request->setHlen(6); // Hardware Address lenght (6 octets)
    request->setHops(0);
    request->setXid(this->xid); // transacction id;
    request->setSecs(0); // 0 seconds from transaction started.
    request->setFlags(0); // 0 = Unicast
    request->setYiaddr(IPv4Address("0.0.0.0")); // NO your IP addr.
    request->setGiaddr(IPv4Address("0.0.0.0")); // NO DHCP Gateway Agents
    request->setChaddr(this->client_mac_address); // my mac address;
    request->setSname(""); // no server name given
    request->setFile(""); // no file given
    request->getOptions().set(DHCP_MSG_TYPE, DHCPREQUEST);
    request->getOptions().set(CLIENT_ID, "Ethernet:" + this->client_mac_address.str());

    // set the parameters to request
    request->getOptions().add(PARAM_LIST, SUBNET_MASK);
    request->getOptions().add(PARAM_LIST, ROUTER);
    request->getOptions().add(PARAM_LIST, DNS);
    request->getOptions().add(PARAM_LIST, NTP_SRV);

    if (this->client_state == REQUESTING)
    {
        // the request is in response of a offer
        request->getOptions().set(SERVER_ID, this->lease->server_id.str());
        request->getOptions().set(REQUESTED_IP, this->lease->ip.str());
        request->setCiaddr(IPv4Address("0.0.0.0")); // NO client IP addr.
        EV << "Sending DHCPREQUEST asking for IP " << this->lease->ip << " via broadcast" << endl;
        sendToUDP(request, this->bootpc_port, IPv4Address::ALLONES_ADDRESS, this->bootps_port);
    }
    else if (this->client_state == RENEWING)
    {
        // the request is for extending the lease
        request->setCiaddr(this->lease->ip); // the client IP
        EV
                << "Sending DHCPREQUEST extending lease for IP " << this->lease->ip << " via unicast to "
                        << this->lease->server_id << endl;
        sendToUDP(request, this->bootpc_port, this->lease->server_id, this->bootps_port);
    }
    else if (this->client_state == REBINDING)
    {
        // the request is for extending the lease
        request->setCiaddr(this->lease->ip); // the client IP
        EV << "Sending DHCPREQUEST renewing the IP " << this->lease->ip << " via broadcast " << endl;
        sendToUDP(request, this->bootpc_port, IPv4Address::ALLONES_ADDRESS, this->bootps_port);
    }
}

void DHCPClient::sendDiscover()
{

    // setting the xid
    this->xid = intuniform(0, RAND_MAX);

    DHCPMessage* discover = new DHCPMessage("DHCPDISCOVER");
    discover->setOp(BOOTREQUEST);
    discover->setByteLength(280); // DHCP Discover packet size;
    discover->setHtype(1); // Ethernet
    discover->setHlen(6); // Hardware Address lenght (6 octets)
    discover->setHops(0);
    discover->setXid(this->xid); // transacction id;
    discover->setSecs(0); // 0 seconds from transaction started.
    discover->setFlags(0); // 0 = Unicast
    discover->setCiaddr(IPv4Address("0.0.0.0")); // NO client IP addr.
    discover->setYiaddr(IPv4Address("0.0.0.0")); // NO your IP addr.
    discover->setGiaddr(IPv4Address("0.0.0.0")); // NO DHCP Gateway Agents
    discover->setChaddr(this->client_mac_address); // my mac address;
    discover->setSname(""); // no server name given
    discover->setFile(""); // no file given
    discover->getOptions().set(DHCP_MSG_TYPE, DHCPDISCOVER);
    discover->getOptions().set(CLIENT_ID, "Ethernet:" + this->client_mac_address.str());
    discover->getOptions().set(REQUESTED_IP, "0.0.0.0");

    // set the parameters to request
    discover->getOptions().add(PARAM_LIST, SUBNET_MASK);
    discover->getOptions().add(PARAM_LIST, ROUTER);
    discover->getOptions().add(PARAM_LIST, DNS);
    discover->getOptions().add(PARAM_LIST, NTP_SRV);

    ev << "Sending DHCPDISCOVER" << endl;
    sendToUDP(discover, this->bootpc_port, IPv4Address::ALLONES_ADDRESS, this->bootps_port);
}

void DHCPClient::cancelTimer(cMessage* timer)
{
    // check if the timer exist
    if (timer != NULL)
    {
        cancelEvent(timer);
        delete timer;
    }
}

void DHCPClient::cancelTimer_T1()
{
    this->cancelTimer(this->timer_t1);
    this->timer_t1 = NULL;
}
void DHCPClient::cancelTimer_T2()
{
    this->cancelTimer(this->timer_t2);
    this->timer_t2 = NULL;
}
void DHCPClient::cancelTimer_TO()
{
    this->cancelTimer(this->timer_to);
    this->timer_to = NULL;

}

void DHCPClient::scheduleTimer_TO(TIMER_TYPE type)
{
    // cancel the previous TO
    this->cancelTimer_TO();
    this->timer_to = new cMessage("DHCP timeout", type);
    scheduleAt(simTime() + this->response_timeout, this->timer_to);
}

void DHCPClient::scheduleTimer_T1()
{
    // cancel the previous T1
    this->cancelTimer_T1();
    this->timer_t1 = new cMessage("DHCP T1", T1);
    scheduleAt(simTime() + (this->lease->renewal_time), this->timer_t1); // RFC 2131 4.4.5
}
void DHCPClient::scheduleTimer_T2()
{
    // cancel the previous T2
    this->cancelTimer_T2();
    this->timer_t2 = new cMessage("DHCP T2", T2);
    scheduleAt(simTime() + (this->lease->rebind_time), this->timer_t2); // RFC 2131 4.4.5
}

cModule *DHCPClient::findHost(void) const
{
    cModule *mod;
    for (mod = getParentModule(); mod != 0; mod = mod->getParentModule())
        if (mod->getSubmodule("notificationBoard"))
            break;

    if (!mod)
        error("findHost(): host module not found (it should have a submodule named notificationBoard)");

    return mod;
}

// Overwrite the sendToUDP in order to add the interface to use to allow the packet be routed by the IP stack
void DHCPClient::sendToUDP(cPacket *msg, int srcPort, const IPvXAddress& destAddr, int destPort)
{

    EV << "Sending packet: ";
    // printPacket(msg);

   // emit(sentPkSignal, msg);
    socket.sendTo(msg, destAddr, destPort, this->ie->getInterfaceId());
}

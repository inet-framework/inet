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


#include "UDPControlInfo_m.h"
#include "IPvXAddressResolver.h"
#include "InterfaceTableAccess.h"
#include "InterfaceTable.h"
#include "IPv4InterfaceData.h"
#include "DHCPServer.h"

#include "DHCP_m.h"

Define_Module(DHCPServer);

void DHCPServer::initialize(int stage)
{
    if (stage == 1)
    {
        numSent = 0;
        numReceived = 0;
        WATCH(numSent);
        WATCH(numReceived);
        WATCH_MAP(leased);

        // DHCP UDP ports
        this->bootpc_port = 68; // client
        this->bootps_port = 67; // server

        // process delay
        this->proc_delay = 0.001; // 100ms

        IInterfaceTable* ift = InterfaceTableAccess().get();

        this->ie = ift->getInterfaceByName(par("iface"));
    }

    if (stage == 2)
    {
        if (this->ie != NULL)
        {
            // bind the client to the udp port
            // bindToPort(bootps_port);
            socket.setOutputGate(gate("udpOut"));
            socket.bind(bootps_port);
            socket.setBroadcast(true);
            ev << "DHCP Server bond to port " << bootps_port << " at " << ie <<  endl;
        }
        else
        {
            error("Interface to listen does not exist. aborting");
            return;
        }
    }
}

void DHCPServer::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        this->handleTimer(msg);
    }
    else
    {
        this->handleIncommingPacket((cPacket*) msg);
    }
}

void DHCPServer::handleIncommingPacket(cPacket *pkt)
{
    // schedule the packet processing
    cMessage* proc_delay_timer = new cMessage("PROC_DELAY", PROC_DELAY);
    proc_delay_timer->addPar("incomming_packet") = pkt;
    scheduleAt(simTime() + this->proc_delay, proc_delay_timer);
    std::cout << "scheduling process" << endl;
}

void DHCPServer::handleTimer(cMessage *msg)
{
    if (msg->getKind() == PROC_DELAY)
    {
        cPacket* pkt = check_and_cast<cPacket*>(msg->par("incomming_packet"));
        cMsgPar* par = (cMsgPar*) msg->removeObject("incomming_packet");
        delete par;
        delete msg;
        if (pkt != NULL)
        {
            this->processPacket(pkt);
            return;
        }
        else
        {
            EV << "incomming packet null, discarding it" << endl;
        }
    }
    else
    {
        EV << "Unknown Timer, discarding it" << endl;
    }
    delete (msg);
}

void DHCPServer::processPacket(cPacket *msg)
{
    EV << "Received packet: ";

    DHCPMessage* packet = check_and_cast<DHCPMessage*>(msg);

    // check the op_code
    if (packet->getOp() == BOOTREQUEST)
    {

        if (packet->getOptions().get(DHCP_MSG_TYPE) == DHCPDISCOVER)
        {
            EV << "DHCPDISCOVER arrived. handling it." << endl;

            DHCPLease* lease = this->getLeaseByMac(packet->getChaddr());
            if (lease == NULL)
            {
                // mac not registered. create offering a new lease to the client
                lease = this->getAvailableLease();
                if (lease != NULL)
                {
                    lease->mac = packet->getChaddr();
                    lease->xid = packet->getXid();
                    lease->parameter_request_list = packet->getOptions().get(PARAM_LIST);
                    lease->leased = true;
                    this->sendOffer(lease);
                }
                else
                {
                    EV << "no lease available. Ignoring discover" << endl;
                }
            }
            else
            {
                // mac already exist. offering the same lease
                // FIXME: if the xid change? what to do?
                lease->xid = packet->getXid();
                lease->parameter_request_list = packet->getOptions().get(PARAM_LIST);
                this->sendOffer(lease);
            }

        }
        else if (packet->getOptions().get(DHCP_MSG_TYPE) == DHCPREQUEST)
        {
            EV << "DHCPREQUEST arrived. handling it." << endl;
            // check if the request was in response of a offering
            if (packet->getOptions().get(SERVER_ID) == this->ie->ipv4Data()->getIPAddress().str())
            {
                // the REQUEST is in response to an offering
                DHCPLease* lease = this->getLeaseByMac(packet->getChaddr());
                if (lease != NULL)
                {
                    EV << "Requesting offered. From now " << lease->ip << " is leased to " << lease->mac << endl;
                    lease->xid = packet->getXid();
                    lease->lease_time = par("lease_time");
                    // TODO: lease the ip
                    this->sendACK(lease);

                    // TODO: update the display string to inform how many clients are assigned
                }
                else
                {
                    // FIXME: lease does not exists.
                }
            }
            else
            {
                // the request is to extend, renew or rebind a lease
                DHCPLease* lease = this->getLeaseByMac(packet->getChaddr()); // FIXME: we should find the lease by ip, not by mac
                if (lease != NULL)
                {
                    EV << "Request for renewal/rebind. extending lease " << lease->ip << " to " << lease->mac << endl;
                    lease->xid = packet->getXid();
                    lease->lease_time = par("lease_time");
                    lease->leased = true;
                    this->sendACK(lease);
                }
                else
                {
                    // FIXME: lease not any more available.
                }
            }
        }
        else
        {
            EV << "BOOTP arrived, but DHCP type unknown. dropping it" << endl;
        }
    }
    else if (packet->getOp() == BOOTREPLY)
    {
        EV << "a BOOTREPLY msg arrived. there is another DHCP server playing around?. dropping the packet" << endl;
    }
    else
    {
        EV << "unknown message. dropping it" << endl;
    }

    EV << "deleting " << msg << endl;
    delete msg;

    numReceived++;
}

void DHCPServer::sendACK(DHCPLease* lease)
{
    EV << "Sending the ACK to " << lease->mac << endl;

    DHCPMessage* ack = new DHCPMessage("DHCPACK");
    ack->setOp(BOOTREPLY);
    //ack->setByteLength(308); // DHCP ACK packet size
    ack->setHtype(1); // Ethernet
    ack->setHlen(6); // Hardware Address lenght (6 octets)
    ack->setHops(0);
    ack->setXid(lease->xid); // transacction id;
    ack->setSecs(0); // 0 seconds from transaction started.
    ack->setFlags(0); // 0 = Unicast
    ack->setCiaddr(lease->ip); // client IP addr.
    ack->setYiaddr(lease->ip); // clinet IP addr.
    ack->setGiaddr(IPv4Address("0.0.0.0")); // Next server ip

    ack->setChaddr(lease->mac); // client mac address;
    ack->setSname(""); // no server name given
    ack->setFile(""); // no file given
    ack->getOptions().set(DHCP_MSG_TYPE, DHCPACK);

    // add the lease options
    long lease_time = this->par("lease_time");
    ack->getOptions().set(SUBNET_MASK, lease->netmask.str());
    ack->getOptions().set(RENEWAL_TIME, lease_time * 0.5); // RFC 4.4.5
    ack->getOptions().set(REBIND_TIME, lease_time * 0.875); // RFC 4.4.5
    ack->getOptions().set(LEASE_TIME, lease_time);
    ack->getOptions().set(ROUTER, lease->gateway.str());
    ack->getOptions().set(DNS, lease->dns.str());
    ack->getOptions().set(LEASE_TIME, lease_time);

    // add the server_id as the RFC says
    ack->getOptions().set(SERVER_ID, this->ie->ipv4Data()->getIPAddress().str());

    // register the lease time
    lease->lease_time = simTime();

    sendToUDP(ack, this->bootps_port, lease->ip.getBroadcastAddress(lease->netmask), this->bootpc_port);
}

void DHCPServer::sendOffer(DHCPLease* lease)
{

    EV << "Offering " << *lease << endl;

    DHCPMessage* offer = new DHCPMessage("DHCPOFFER");
    offer->setOp(BOOTREPLY);
    //offer->setByteLength(308); // DHCP OFFER packet size
    offer->setHtype(1); // Ethernet
    offer->setHlen(6); // Hardware Address lenght (6 octets)
    offer->setHops(0);
    offer->setXid(lease->xid); // transacction id;
    offer->setSecs(0); // 0 seconds from transaction started.
    offer->setFlags(0); // 0 = Unicast
    offer->setCiaddr(IPv4Address("0.0.0.0")); // NO client IP addr.

    offer->setYiaddr(lease->ip); // ip offered.
    offer->setGiaddr(lease->gateway); // next server ip

    offer->setChaddr(lease->mac); // client mac address;
    offer->setSname(""); // no server name given
    offer->setFile(""); // no file given
    offer->getOptions().set(DHCP_MSG_TYPE, DHCPOFFER);

    // add the offer options
    long lease_time = this->par("lease_time");
    offer->getOptions().set(SUBNET_MASK, lease->netmask.str());
    offer->getOptions().set(RENEWAL_TIME, lease_time * 0.5); // RFC 4.4.5
    offer->getOptions().set(REBIND_TIME, lease_time * 0.875); // RFC 4.4.5
    offer->getOptions().set(LEASE_TIME, lease_time);
    offer->getOptions().set(DNS, lease->dns.str());
    offer->getOptions().set(ROUTER, lease->gateway.str());

    // add the server_id as the RFC says
    offer->getOptions().set(SERVER_ID, this->ie->ipv4Data()->getIPAddress().str());

    // register the offering time
    lease->lease_time = simTime();

    sendToUDP(offer, 67, lease->ip.getBroadcastAddress(lease->netmask), 68);
}

DHCPLease* DHCPServer::getLeaseByMac(MACAddress mac)
{
    for (DHCPLeased::iterator it = this->leased.begin(); it != this->leased.end(); it++)
    {
        // lease exist
        if (it->second.mac == mac)
        {
            EV << "found lease for mac " << mac << endl;
            return (&(it->second));
        }
    }
    EV << "lease not found for mac " << mac << endl;
    // lease does not exist
    return (NULL);
}

DHCPLease* DHCPServer::getAvailableLease()
{
    IPv4Address network(par("net").stringValue());
    IPv4Address netmask(par("mask").stringValue());
    IPv4Address gateway(par("gateway").stringValue());
    IPv4Address begin(par("ip_begin").stringValue());

    int num_cli = par("client_num");

    int begin_addr_int = begin.getInt();
    for (int i = 0; i < num_cli; i++)
    {
        IPv4Address ip(begin_addr_int + i);
        if (this->leased.find(ip) != this->leased.end())
        {
            // lease exists
            if (!this->leased[ip].leased)
            {
                // lease expired
                return (&(this->leased[ip]));
            }
        }
        else
        {
            // lease does not exist, create it
            this->leased[ip] = DHCPLease();
            this->leased[ip].ip = ip;
            this->leased[ip].gateway = gateway;
            this->leased[ip].netmask = netmask;
            this->leased[ip].network = network;
            return (&(this->leased[ip]));
        }
    }
    // no lease available
    return (NULL);
}

void DHCPServer::sendToUDP(cPacket *msg, int srcPort, const IPvXAddress& destAddr, int destPort)
{
    // Overwrite the sendToUDP in order to add the interface to use to allow the packet be routed by the IP stack
    msg->setKind(UDP_C_DATA);
    EV << "Sending packet: ";
    // printPacket(msg);

    //send(msg, "udpOut");
    socket.sendTo(msg, destAddr, destPort, this->ie->getInterfaceId());
}

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

#include <algorithm>
#include "UDPControlInfo_m.h"
#include "IPvXAddressResolver.h"
#include "InterfaceTable.h"
#include "IPv4InterfaceData.h"
#include "DHCPServer.h"
#include "NodeOperations.h"
#include "NodeStatus.h"
#include "NotificationBoard.h"
#include "NotifierConsts.h"

Define_Module(DHCPServer);


DHCPServer::DHCPServer()
{
    ie = NULL;
}

DHCPServer::~DHCPServer()
{
}

void DHCPServer::initialize(int stage)
{
    if (stage == 1)
    {
        numSent = 0;
        numReceived = 0;
        WATCH(numSent);
        WATCH(numReceived);
        WATCH_MAP(leased);

        subnetMask = IPv4Address(par("subnetMask").stringValue());
        gateway = IPv4Address(par("gateway").stringValue());
        ipAddressStart = IPv4Address(par("ipAddressStart").stringValue());
        maxNumOfClients = par("maxNumClients");
        leaseTime = par("leaseTime");

        // DHCP UDP ports
        clientPort = 68; // client
        serverPort = 67; // server

        IInterfaceTable * ift = check_and_cast<IInterfaceTable*>(getModuleByPath(par("interfaceTablePath")));
        ie = ift->getInterfaceByName(par("interface"));

        nb = check_and_cast<NotificationBoard*>(getModuleByPath(par("notificationBoardPath")));
        nb->subscribe(this, NF_INTERFACE_CREATED);
        nb->subscribe(this, NF_INTERFACE_DELETED);

        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;

        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state!");
    }
    if (stage == 2)
    {
        if (ie != NULL)
            openSocket();
    }
}

void DHCPServer::openSocket()
{
    if (!ie)
        error("Interface to listen does not exist. Aborting!");

    socket.setOutputGate(gate("udpOut"));
    socket.bind(serverPort);
    socket.setBroadcast(true);
    EV_INFO << "DHCP server bound to port " << serverPort << " on " << ie->getName() << "." << endl;
}

void DHCPServer::receiveChangeNotification(int category, const cPolymorphic *details)
{
    Enter_Method_Silent();

    InterfaceEntry * nie;
    switch (category)
    {
        case NF_INTERFACE_CREATED:
            nie = const_cast<InterfaceEntry *>(check_and_cast<const InterfaceEntry*>(details));
            if (!ie && !strcmp(nie->getName(), par("interface").stringValue()))
                ie = nie;
            break;

        case NF_INTERFACE_DELETED:
            nie = const_cast<InterfaceEntry *>(check_and_cast<const InterfaceEntry*>(details));
            if (ie == nie)
                ie = NULL;
            break;

        default:
            throw cRuntimeError("Unaccepted notification category: %d", category);
    }
}

void DHCPServer::handleMessage(cMessage *msg)
{

    DHCPMessage * dhcpPacket = dynamic_cast<DHCPMessage*>(msg);
    if (dhcpPacket)
        processPacket(dhcpPacket);
    else
    {
        EV_WARN << "Unknown packet, discarding it." << endl;
        delete msg;
    }
}


void DHCPServer::processPacket(DHCPMessage *packet)
{
    // check the OP code
    if (packet->getOp() == BOOTREQUEST)
    {

        if (packet->getOptions().getMessageType() == DHCPDISCOVER) // RFC 2131, 4.3.1
        {
            EV_INFO << "DHCPDISCOVER arrived. Handling it." << endl;

            DHCPLease* lease = getLeaseByMac(packet->getChaddr());
            if (!lease)
            {
                // MAC not registered, create offering a new lease to the client
                lease = getAvailableLease(packet->getOptions().getRequestedIp(),packet->getChaddr());
                if (lease != NULL)
                {
                    // std::cout << "MAC: " << packet->getChaddr() << " ----> IP: " << lease->ip << endl;
                    lease->mac = packet->getChaddr();
                    lease->xid = packet->getXid();
                    //lease->parameterRequestList = packet->getOptions().get(PARAM_LIST); TODO: !!
                    lease->leased = true; // TODO
                    sendOffer(lease);
                }
                else
                    EV_ERROR << "No lease available. Ignoring discover." << endl;
            }
            else
            {
                // MAC already exist, offering the same lease
                lease->xid = packet->getXid();
                //lease->parameterRequestList = packet->getOptions().get(PARAM_LIST); // TODO: !!
                sendOffer(lease);
            }

        }
        else if (packet->getOptions().getMessageType() == DHCPREQUEST) // RFC 2131, 4.3.2
        {
            EV_INFO << "DHCPREQUEST arrived. Handling it." << endl;

            // check if the request was in response of an offering
            if (packet->getOptions().getServerIdentifier() == ie->ipv4Data()->getIPAddress())
            {
                // the REQUEST is in response to an offering (because SERVER_ID is filled)
                // otherwise the msg is a request to extend an existing lease (e. g. INIT-REBOOT)

                DHCPLease* lease = getLeaseByMac(packet->getChaddr());
                if (lease != NULL)
                {
                    if (lease->ip != packet->getOptions().getRequestedIp())
                    {
                        EV_ERROR << "The 'requested IP address' must be filled in with the 'yiaddr' value from the chosen DHCPOFFER." << endl;
                        sendNAK(packet);
                    }
                    else
                    {
                        EV_INFO << "From now " << lease->ip << " is leased to " << lease->mac << "." << endl;
                        lease->xid = packet->getXid();
                        lease->leaseTime = leaseTime;
                        lease->leased = true;

                        // TODO: final check before ACK (it is not necessary but recommended)
                        sendACK(lease,packet);

                        // TODO: update the display string to inform how many clients are assigned
                    }
                }
                else
                {
                    EV_ERROR << "There is no available lease for " << packet->getChaddr() << ". Probably, the client missed to send DHCPDISCOVER before DHCPREQUEST." << endl;
                    sendNAK(packet);
                }
            }
            else
            {
                if (packet->getCiaddr().isUnspecified())  // init-reboot
                {
                    // std::cout << "init-reboot" << endl;
                    IPv4Address requestedAddress = packet->getOptions().getRequestedIp();
                    DHCPLeased::iterator it = leased.find(requestedAddress);
                    if (it == leased.end())
                    {
                        // if DHCP server has no record of the requested IP, then it must remain silent
                        // and may output a warning to the network admin
                        EV_WARN << "DHCP server has no record of IP " << requestedAddress << "." << endl;
                    }
                    else if (IPv4Address::maskedAddrAreEqual(requestedAddress,it->second.ip,subnetMask)) // on the same network
                    {
                        DHCPLease * lease = &it->second;
                        EV_INFO << "Initialization with known IP address (INIT-REBOOT) " << lease->ip <<  " on " << lease->mac <<  " was successful." << endl;
                        lease->xid = packet->getXid();
                        lease->leaseTime = leaseTime;
                        lease->leased = true;

                        // TODO: final check before ACK (it is not necessary but recommended)
                        sendACK(lease,packet);
                    }
                    else
                    {
                        EV_ERROR << "The requested IP address is incorrect, or is on the wrong network." << endl;
                        sendNAK(packet);
                    }
                }
                else // renewing or rebinding: in this case ciaddr must be filled in with client's IP address
                {
                    DHCPLeased::iterator it = leased.find(packet->getCiaddr());
                    DHCPLease * lease = &it->second;
                    if (it != leased.end())
                    {
                        EV_INFO << "Request for renewal/rebinding IP " << lease->ip << " to " << lease->mac << "." << endl;
                        lease->xid = packet->getXid();
                        lease->leaseTime = leaseTime;
                        lease->leased = true;

                        // unicast ACK to ciaddr
                        sendACK(lease,packet);
                    }
                    else
                    {
                        EV_ERROR << "Renewal/rebinding process failed: requested IP address " << packet->getCiaddr() << " not found in the server's database!" << endl;
                        sendNAK(packet);
                    }
                }
            }
        }
        else
            EV_WARN << "BOOTREQUEST arrived, but DHCP message type is unknown. Dropping it." << endl;
    }
    else
    {
        EV_WARN << "Message opcode is unknown. This DHCP server only handles BOOTREQUEST messages. Dropping it." << endl;
    }

    EV_DEBUG << "Deleting " << packet << "." << endl;
    delete packet;

    numReceived++;
}

void DHCPServer::sendNAK(DHCPMessage* msg)
{
    //EV_INFO << "Sending NAK to " << lease->mac << "." << endl;
    DHCPMessage * nak = new DHCPMessage("DHCPNAK");
    nak->setOp(BOOTREPLY);
    nak->setByteLength(308); // DHCPNAK packet size
    nak->setHtype(1); // ethernet
    nak->setHlen(6); // hardware address length (6 octets)
    nak->setHops(0);
    nak->setXid(msg->getXid()); // transaction id from client
    nak->setSecs(0); // 0 seconds from transaction started.
    nak->setFlags(msg->getFlags()); // 0 = unicast
    nak->setGiaddr(msg->getGiaddr()); // next server ip
    nak->setChaddr(msg->getChaddr());
    nak->getOptions().setServerIdentifier(ie->ipv4Data()->getIPAddress());

    sendToUDP(nak, serverPort, IPv4Address::ALLONES_ADDRESS, clientPort);
}

void DHCPServer::sendACK(DHCPLease* lease, DHCPMessage * packet)
{
    EV_INFO << "Sending the ACK to " << lease->mac << "." << endl;

    DHCPMessage* ack = new DHCPMessage("DHCPACK");
    ack->setOp(BOOTREPLY);
    ack->setByteLength(308); // DHCP ACK packet size
    ack->setHtype(1);// ethernet
    ack->setHlen(6);// hardware Address lenght (6 octets)
    ack->setHops(0);
    ack->setXid(lease->xid);// transaction id;
    ack->setSecs(0);// 0 seconds from transaction started
    ack->setFlags(0);// 0 = unicast
    ack->setCiaddr(lease->ip);// client IP addr.
    ack->setYiaddr(lease->ip);// clinet IP addr.

    ack->setChaddr(lease->mac); // client MAC address
    ack->setSname(""); // no server name given
    ack->setFile(""); // no file given
    ack->getOptions().setMessageType(DHCPACK);

    // add the lease options
    ack->getOptions().setSubnetMask(lease->subnetMask);
    ack->getOptions().setRenewalTime(leaseTime * 0.5); // RFC 4.4.5
    ack->getOptions().setRebindingTime(leaseTime * 0.875);
    ack->getOptions().setLeaseTime(leaseTime);
    ack->getOptions().setRouterArraySize(1);
    ack->getOptions().setRouter(0,lease->gateway);
    ack->getOptions().setDnsArraySize(1);
    ack->getOptions().setDns(0,lease->dns);

    // add the server_id as the RFC says
    ack->getOptions().setServerIdentifier(ie->ipv4Data()->getIPAddress());

    // register the lease time
    lease->leaseTime = simTime();

    if (packet->getGiaddr().isUnspecified() && !packet->getCiaddr().isUnspecified())
        sendToUDP(ack, serverPort, packet->getCiaddr(), clientPort);
    else
        sendToUDP(ack, serverPort, lease->ip.makeBroadcastAddress(lease->ip.getNetworkMask()), clientPort);
}

void DHCPServer::sendOffer(DHCPLease* lease)
{

    EV_INFO << "Offering " << *lease << endl;

    DHCPMessage * offer = new DHCPMessage("DHCPOFFER");
    offer->setOp(BOOTREPLY);
    offer->setByteLength(308); // DHCP OFFER packet size
    offer->setHtype(1); // ethernet
    offer->setHlen(6); // hardware Address lenght (6 octets)
    offer->setHops(0);
    offer->setXid(lease->xid); // transacction id;
    offer->setSecs(0); // 0 seconds from transaction started.
    offer->setFlags(0); // 0 = unicast

    offer->setYiaddr(lease->ip); // ip offered.
    offer->setGiaddr(lease->gateway); // next server ip

    offer->setChaddr(lease->mac);// client mac address;
    offer->setSname(""); // no server name given
    offer->setFile(""); // no file given
    offer->getOptions().setMessageType(DHCPOFFER);

    // add the offer options
    offer->getOptions().setSubnetMask(lease->subnetMask);
    offer->getOptions().setRenewalTime(leaseTime * 0.5); // RFC 4.4.5
    offer->getOptions().setRebindingTime(leaseTime * 0.875);
    offer->getOptions().setLeaseTime(leaseTime);
    offer->getOptions().setRouterArraySize(1);
    offer->getOptions().setRouter(0,lease->gateway);
    offer->getOptions().setDnsArraySize(1);
    offer->getOptions().setDns(0,lease->dns);

    // add the server_id as the RFC says
    offer->getOptions().setServerIdentifier(ie->ipv4Data()->getIPAddress());

    // register the offering time // todo: ?
    lease->leaseTime = simTime();

    sendToUDP(offer, 67, lease->ip.makeBroadcastAddress(lease->ip.getNetworkMask()), 68);
}

DHCPLease* DHCPServer::getLeaseByMac(MACAddress mac)
{
    for (DHCPLeased::iterator it = leased.begin(); it != leased.end(); it++)
    {
        // lease exist
        if (it->second.mac == mac)
        {
            EV_DETAIL << "Found lease for MAC " << mac << "." << endl;
            return (&(it->second));
        }
    }
    EV_DETAIL << "Lease not found for MAC " << mac << "." << endl;

    // lease does not exist
    return NULL;
}

DHCPLease* DHCPServer::getAvailableLease(IPv4Address requestedAddress, MACAddress& clientMAC)
{
    int beginAddr = ipAddressStart.getInt(); // the first address that we might use

    // try to allocate the requested address if that address is valid and not already allocated
    if (!requestedAddress.isUnspecified()) // valid
    {
        if (leased.find(requestedAddress) != leased.end() && !leased[requestedAddress].leased) // not already leased (allocated)
            return &leased[requestedAddress];

        // lease does not exist, create it
        leased[requestedAddress] = DHCPLease();
        leased[requestedAddress].ip = requestedAddress;
        leased[requestedAddress].gateway = gateway;
        leased[requestedAddress].subnetMask = subnetMask;

        return &leased[requestedAddress];
    }

    // allocate new address from server's pool of available addresses
    for (unsigned int i = 0; i < maxNumOfClients; i++)
    {
        IPv4Address ip(beginAddr + i);
        if (leased.find(ip) != leased.end())
        {
            // lease exists but not allocated (e.g. expired or released)
            if (!leased[ip].leased)
                return (&(leased[ip]));
        }
        else
        {
            // there is no expired lease so we create a new one
            leased[ip] = DHCPLease();
            leased[ip].ip = ip;
            leased[ip].gateway = gateway;
            leased[ip].subnetMask = subnetMask;
            return (&(leased[ip]));
        }
    }
    // no lease available
    return NULL;
}

void DHCPServer::sendToUDP(cPacket *msg, int srcPort, const IPvXAddress& destAddr, int destPort)
{
    EV_INFO << "Sending packet: " << msg << "." << endl;
    numSent++;
    socket.sendTo(msg, destAddr, destPort, ie->getInterfaceId());
}

bool DHCPServer::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation))
    {
        if (stage == NodeStartOperation::STAGE_APPLICATION_LAYER)
        {
            IInterfaceTable * ift = check_and_cast<IInterfaceTable * >(getModuleByPath(par("interfaceTablePath")));
            ie = ift->getInterfaceByName(par("interface"));
            openSocket();
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation))
    {
        if (stage == NodeShutdownOperation::STAGE_APPLICATION_LAYER)
        {
            leased.clear();
            ie = NULL;
            // socket.close(); TODO:
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation))
    {
        if (stage == NodeCrashOperation::STAGE_CRASH)
        {
            leased.clear();
            ie = NULL;
            // socket???
        }
    }
    else
        throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

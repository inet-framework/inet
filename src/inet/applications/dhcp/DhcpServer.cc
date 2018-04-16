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

#include "inet/applications/dhcp/DhcpServer.h"

#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/Simsignals.h"
#include "inet/linklayer/common/InterfaceTag_m.h"

namespace inet {

Define_Module(DhcpServer);

DhcpServer::DhcpServer()
{
    ie = nullptr;
    startTimer = nullptr;
}

DhcpServer::~DhcpServer()
{
    cancelAndDelete(startTimer);
}

void DhcpServer::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        startTimer = new cMessage("Start DHCP server", START_DHCP);
        startTime = par("startTime");
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        numSent = 0;
        numReceived = 0;
        WATCH(numSent);
        WATCH(numReceived);
        WATCH_MAP(leased);

        // DHCP UDP ports
        clientPort = 68;    // client
        serverPort = 67;    // server

        cModule *host = getContainingNode(this);
        host->subscribe(interfaceDeletedSignal, this);

        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(host->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;

        if (isOperational)
            startApp();
    }
}

void DhcpServer::openSocket()
{
    if (!ie)
        throw cRuntimeError("Interface to listen does not exist. aborting");
    socket.setOutputGate(gate("socketOut"));
    socket.bind(serverPort);
    socket.setBroadcast(true);
    EV_INFO << "DHCP server bound to port " << serverPort << endl;
}

void DhcpServer::receiveSignal(cComponent *source, int signalID, cObject *obj, cObject *details)
{
    Enter_Method_Silent();

    if (signalID == interfaceDeletedSignal) {
        if (isOperational) {
            InterfaceEntry *nie = check_and_cast<InterfaceEntry *>(obj);
            if (ie == nie)
                throw cRuntimeError("Reacting to interface deletions is not implemented in this module");
        }
    }
    else
        throw cRuntimeError("Unexpected signal: %s", getSignalName(signalID));
}

InterfaceEntry *DhcpServer::chooseInterface()
{
    IInterfaceTable *ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    const char *interfaceName = par("interface");
    InterfaceEntry *ie = nullptr;

    if (strlen(interfaceName) > 0) {
        ie = ift->getInterfaceByName(interfaceName);
        if (ie == nullptr)
            throw cRuntimeError("Interface \"%s\" does not exist", interfaceName);
    }
    else {
        // there should be exactly one non-loopback interface that we want to serve DHCP requests on
        for (int i = 0; i < ift->getNumInterfaces(); i++) {
            InterfaceEntry *current = ift->getInterface(i);
            if (!current->isLoopback()) {
                if (ie)
                    throw cRuntimeError("Multiple non-loopback interfaces found, please select explicitly which one you want to serve DHCP requests on");
                ie = current;
            }
        }
        if (!ie)
            throw cRuntimeError("No non-loopback interface found to be configured via DHCP");
    }

    return ie;
}

void DhcpServer::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        handleSelfMessages(msg);
    }
    else {
        auto packet = check_and_cast<Packet *>(msg);
        if (packet->getKind() == UDP_I_DATA)
            processDHCPMessage(packet);
        else {
            // note: unknown packets are likely Icmp errors in response to DHCP messages we sent out; just ignore them
            EV_WARN << "Unknown packet '" << msg->getName() << "', kind = " << packet->getKind() << ", discarding it." << endl;
            delete msg;
        }
    }
}

void DhcpServer::handleSelfMessages(cMessage *msg)
{
    if (msg->getKind() == START_DHCP) {
        openSocket();
    }
    else
        throw cRuntimeError("Unknown selfmessage type!");
}

void DhcpServer::processDHCPMessage(Packet *packet)
{
    ASSERT(isOperational && ie != nullptr);

    const auto& dhcpMsg = packet->peekAtFront<DhcpMessage>();

    // check that the packet arrived on the interface we are supposed to serve
    int inputInterfaceId = packet->getTag<InterfaceInd>()->getInterfaceId();
    if (inputInterfaceId != ie->getInterfaceId()) {
        EV_WARN << "DHCP message arrived on a different interface, dropping\n";
        delete packet;
        return;
    }

    // check the OP code
    if (dhcpMsg->getOp() == BOOTREQUEST) {
        int messageType = dhcpMsg->getOptions().getMessageType();

        if (messageType == DHCPDISCOVER) {    // RFC 2131, 4.3.1
            EV_INFO << "DHCPDISCOVER arrived. Handling it." << endl;

            DhcpLease *lease = getLeaseByMac(dhcpMsg->getChaddr());
            if (!lease) {
                // MAC not registered, create offering a new lease to the client
                lease = getAvailableLease(dhcpMsg->getOptions().getRequestedIp(), dhcpMsg->getChaddr());
                if (lease != nullptr) {
                    // std::cout << "MAC: " << packet->getChaddr() << " ----> IP: " << lease->ip << endl;
                    lease->mac = dhcpMsg->getChaddr();
                    lease->xid = dhcpMsg->getXid();
                    //lease->parameterRequestList = packet->getOptions().get(PARAM_LIST); TODO: !!
                    lease->leased = true;    // TODO
                    sendOffer(lease, dhcpMsg);
                }
                else
                    EV_ERROR << "No lease available. Ignoring discover." << endl;
            }
            else {
                // MAC already exist, offering the same lease
                lease->xid = dhcpMsg->getXid();
                //lease->parameterRequestList = packet->getOptions().get(PARAM_LIST); // TODO: !!
                sendOffer(lease, dhcpMsg);
            }
        }
        else if (messageType == DHCPREQUEST) {    // RFC 2131, 4.3.2
            EV_INFO << "DHCPREQUEST arrived. Handling it." << endl;

            // check if the request was in response of an offering
            if (dhcpMsg->getOptions().getServerIdentifier() == ie->ipv4Data()->getIPAddress()) {
                // the REQUEST is in response to an offering (because SERVER_ID is filled)
                // otherwise the msg is a request to extend an existing lease (e. g. INIT-REBOOT)

                DhcpLease *lease = getLeaseByMac(dhcpMsg->getChaddr());
                if (lease != nullptr) {
                    if (lease->ip != dhcpMsg->getOptions().getRequestedIp()) {
                        EV_ERROR << "The 'requested IP address' must be filled in with the 'yiaddr' value from the chosen DHCPOFFER." << endl;
                        sendNAK(dhcpMsg);
                    }
                    else {
                        EV_INFO << "From now " << lease->ip << " is leased to " << lease->mac << "." << endl;
                        lease->xid = dhcpMsg->getXid();
                        lease->leaseTime = leaseTime;
                        lease->leased = true;

                        // TODO: final check before ACK (it is not necessary but recommended)
                        sendACK(lease, dhcpMsg);

                        // TODO: update the display string to inform how many clients are assigned
                    }
                }
                else {
                    EV_ERROR << "There is no available lease for " << dhcpMsg->getChaddr() << ". Probably, the client missed to send DHCPDISCOVER before DHCPREQUEST." << endl;
                    sendNAK(dhcpMsg);
                }
            }
            else {
                if (dhcpMsg->getCiaddr().isUnspecified()) {    // init-reboot
                    // std::cout << "init-reboot" << endl;
                    Ipv4Address requestedAddress = dhcpMsg->getOptions().getRequestedIp();
                    auto it = leased.find(requestedAddress);
                    if (it == leased.end()) {
                        // if DHCP server has no record of the requested IP, then it must remain silent
                        // and may output a warning to the network admin
                        EV_WARN << "DHCP server has no record of IP " << requestedAddress << "." << endl;
                    }
                    else if (Ipv4Address::maskedAddrAreEqual(requestedAddress, it->second.ip, subnetMask)) {    // on the same network
                        DhcpLease *lease = &it->second;
                        EV_INFO << "Initialization with known IP address (INIT-REBOOT) " << lease->ip << " on " << lease->mac << " was successful." << endl;
                        lease->xid = dhcpMsg->getXid();
                        lease->leaseTime = leaseTime;
                        lease->leased = true;

                        // TODO: final check before ACK (it is not necessary but recommended)
                        sendACK(lease, dhcpMsg);
                    }
                    else {
                        EV_ERROR << "The requested IP address is incorrect, or is on the wrong network." << endl;
                        sendNAK(dhcpMsg);
                    }
                }
                else {    // renewing or rebinding: in this case ciaddr must be filled in with client's IP address
                    auto it = leased.find(dhcpMsg->getCiaddr());
                    DhcpLease *lease = &it->second;
                    if (it != leased.end()) {
                        EV_INFO << "Request for renewal/rebinding IP " << lease->ip << " to " << lease->mac << "." << endl;
                        lease->xid = dhcpMsg->getXid();
                        lease->leaseTime = leaseTime;
                        lease->leased = true;

                        // unicast ACK to ciaddr
                        sendACK(lease, dhcpMsg);
                    }
                    else {
                        EV_ERROR << "Renewal/rebinding process failed: requested IP address " << dhcpMsg->getCiaddr() << " not found in the server's database!" << endl;
                        sendNAK(dhcpMsg);
                    }
                }
            }
        }
        else
            EV_WARN << "BOOTREQUEST arrived, but DHCP message type is unknown. Dropping it." << endl;
    }
    else {
        EV_WARN << "Message opcode is unknown. This DHCP server only handles BOOTREQUEST messages. Dropping it." << endl;
    }

    EV_DEBUG << "Deleting " << packet << "." << endl;
    delete packet;

    numReceived++;
}

void DhcpServer::sendNAK(const Ptr<const DhcpMessage>& msg)
{
    // EV_INFO << "Sending NAK to " << lease->mac << "." << endl;
    Packet *pk = new Packet("DHCPNAK");
    const auto& nak = makeShared<DhcpMessage>();
    nak->setOp(BOOTREPLY);
    nak->setChunkLength(B(308));    // DHCPNAK packet size
    nak->setHtype(1);    // ethernet
    nak->setHlen(6);    // hardware address length (6 octets)
    nak->setHops(0);
    nak->setXid(msg->getXid());    // transaction id from client
    nak->setSecs(0);    // 0 seconds from transaction started.
    nak->setBroadcast(msg->getBroadcast());
    nak->setGiaddr(msg->getGiaddr());    // next server IP
    nak->setChaddr(msg->getChaddr());
    nak->getOptionsForUpdate().setServerIdentifier(ie->ipv4Data()->getIPAddress());
    nak->getOptionsForUpdate().setMessageType(DHCPNAK);

    pk->insertAtBack(nak);
    /* RFC 2131, 4.1
     *
     * In all cases, when 'giaddr' is zero, the server broadcasts any DHCPNAK
     * messages to 0xffffffff.
     */
    Ipv4Address destAddr = Ipv4Address::ALLONES_ADDRESS;
    if (!msg->getGiaddr().isUnspecified())
        destAddr = msg->getGiaddr();
    sendToUDP(pk, serverPort, destAddr, clientPort);
}

void DhcpServer::sendACK(DhcpLease *lease, const Ptr<const DhcpMessage>& packet)
{
    EV_INFO << "Sending the ACK to " << lease->mac << "." << endl;

    Packet *pk = new Packet("DHCPACK");
    const auto& ack = makeShared<DhcpMessage>();
    ack->setOp(BOOTREPLY);
    ack->setChunkLength(B(308));    // DHCP ACK packet size
    ack->setHtype(1);    // ethernet
    ack->setHlen(6);    // hardware address length (6 octets)
    ack->setHops(0);
    ack->setXid(lease->xid);    // transaction id;
    ack->setSecs(0);    // 0 seconds from transaction started
    ack->setBroadcast(false);
    ack->setCiaddr(lease->ip);    // client IP addr.
    ack->setYiaddr(lease->ip);    // client IP addr.

    ack->setChaddr(lease->mac);    // client MAC address
    ack->setSname("");    // no server name given
    ack->setFile("");    // no file given
    ack->getOptionsForUpdate().setMessageType(DHCPACK);

    // add the lease options
    ack->getOptionsForUpdate().setSubnetMask(lease->subnetMask);
    ack->getOptionsForUpdate().setRenewalTime(leaseTime * 0.5);    // RFC 4.4.5
    ack->getOptionsForUpdate().setRebindingTime(leaseTime * 0.875);
    ack->getOptionsForUpdate().setLeaseTime(leaseTime);
    ack->getOptionsForUpdate().setRouterArraySize(1);
    ack->getOptionsForUpdate().setRouter(0, lease->gateway);
    ack->getOptionsForUpdate().setDnsArraySize(1);
    ack->getOptionsForUpdate().setDns(0, lease->dns);

    // add the server ID as the RFC says
    ack->getOptionsForUpdate().setServerIdentifier(ie->ipv4Data()->getIPAddress());
    pk->insertAtBack(ack);

    // register the lease time
    lease->leaseTime = simTime();

    /* RFC 2131, 4.1
     * If the 'giaddr' field in a DHCP message from a client is non-zero,
     * the server sends any return messages to the 'DHCP server' port on the
     * BOOTP relay agent whose address appears in 'giaddr'. If the 'giaddr'
     * field is zero and the 'ciaddr' field is nonzero, then the server
     * unicasts DHCPOFFER and DHCPACK messages to the address in 'ciaddr'.
     * If 'giaddr' is zero and 'ciaddr' is zero, and the broadcast bit is
     * set, then the server broadcasts DHCPOFFER and DHCPACK messages to
     * 0xffffffff. If the broadcast bit is not set and 'giaddr' is zero and
     * 'ciaddr' is zero, then the server unicasts DHCPOFFER and DHCPACK
     * messages to the client's hardware address and 'yiaddr' address.
     */
    Ipv4Address destAddr;
    if (!packet->getGiaddr().isUnspecified())
        destAddr = packet->getGiaddr();
    else if (!packet->getCiaddr().isUnspecified())
        destAddr = packet->getCiaddr();
    else if (packet->getBroadcast())
        destAddr = Ipv4Address::ALLONES_ADDRESS;
    else {
        // TODO should send it to client's hardware address and yiaddr address, but the application can not set the destination MacAddress.
        // destAddr = lease->ip;
        destAddr = Ipv4Address::ALLONES_ADDRESS;
    }

    sendToUDP(pk, serverPort, destAddr, clientPort);
}

void DhcpServer::sendOffer(DhcpLease *lease, const Ptr<const DhcpMessage>& packet)
{
    EV_INFO << "Offering " << *lease << endl;

    Packet *pk = new Packet("DHCPOFFER");
    const auto& offer = makeShared<DhcpMessage>();
    offer->setOp(BOOTREPLY);
    offer->setChunkLength(B(308));    // DHCP OFFER packet size
    offer->setHtype(1);    // ethernet
    offer->setHlen(6);    // hardware address lenght (6 octets)
    offer->setHops(0);
    offer->setXid(lease->xid);    // transaction id
    offer->setSecs(0);    // 0 seconds from transaction started
    offer->setBroadcast(false);    // unicast

    offer->setYiaddr(lease->ip);    // ip offered.
    offer->setGiaddr(lease->gateway);    // next server ip

    offer->setChaddr(lease->mac);    // client mac address
    offer->setSname("");    // no server name given
    offer->setFile("");    // no file given
    offer->getOptionsForUpdate().setMessageType(DHCPOFFER);

    // add the offer options
    offer->getOptionsForUpdate().setSubnetMask(lease->subnetMask);
    offer->getOptionsForUpdate().setRenewalTime(leaseTime * 0.5);    // RFC 4.4.5
    offer->getOptionsForUpdate().setRebindingTime(leaseTime * 0.875);
    offer->getOptionsForUpdate().setLeaseTime(leaseTime);
    offer->getOptionsForUpdate().setRouterArraySize(1);
    offer->getOptionsForUpdate().setRouter(0, lease->gateway);
    offer->getOptionsForUpdate().setDnsArraySize(1);
    offer->getOptionsForUpdate().setDns(0, lease->dns);

    // add the server_id as the RFC says
    offer->getOptionsForUpdate().setServerIdentifier(ie->ipv4Data()->getIPAddress());

    // register the offering time // todo: ?
    lease->leaseTime = simTime();
    pk->insertAtBack(offer);

    /* RFC 2131, 4.1
     * If the 'giaddr' field in a DHCP message from a client is non-zero,
     * the server sends any return messages to the 'DHCP server' port on the
     * BOOTP relay agent whose address appears in 'giaddr'. If the 'giaddr'
     * field is zero and the 'ciaddr' field is nonzero, then the server
     * unicasts DHCPOFFER and DHCPACK messages to the address in 'ciaddr'.
     * If 'giaddr' is zero and 'ciaddr' is zero, and the broadcast bit is
     * set, then the server broadcasts DHCPOFFER and DHCPACK messages to
     * 0xffffffff. If the broadcast bit is not set and 'giaddr' is zero and
     * 'ciaddr' is zero, then the server unicasts DHCPOFFER and DHCPACK
     * messages to the client's hardware address and 'yiaddr' address.
     */
    Ipv4Address destAddr;
    if (!packet->getGiaddr().isUnspecified())
        destAddr = packet->getGiaddr();
    else if (!packet->getCiaddr().isUnspecified())
        destAddr = packet->getCiaddr();
    else if (packet->getBroadcast())
        destAddr = Ipv4Address::ALLONES_ADDRESS;
    else {
        // TODO should send it to client's hardware address and yiaddr address, but the application can not set the destination MacAddress.
        // destAddr = lease->ip;
        destAddr = Ipv4Address::ALLONES_ADDRESS;
    }

    sendToUDP(pk, serverPort, destAddr, clientPort);
}

DhcpLease *DhcpServer::getLeaseByMac(MacAddress mac)
{
    for (auto & elem : leased) {
        // lease exist
        if (elem.second.mac == mac) {
            EV_DETAIL << "Found lease for MAC " << mac << "." << endl;
            return &(elem.second);
        }
    }
    EV_DETAIL << "Lease not found for MAC " << mac << "." << endl;

    // lease does not exist
    return nullptr;
}

DhcpLease *DhcpServer::getAvailableLease(Ipv4Address requestedAddress, const MacAddress& clientMAC)
{
    int beginAddr = ipAddressStart.getInt();    // the first address that we might use

    // try to allocate the requested address if that address is valid and not already allocated
    if (!requestedAddress.isUnspecified()) {    // valid
        if (leased.find(requestedAddress) != leased.end() && !leased[requestedAddress].leased) // not already leased (allocated)
            return &leased[requestedAddress];

        // lease does not exist, create it
        leased[requestedAddress] = DhcpLease();
        leased[requestedAddress].ip = requestedAddress;
        leased[requestedAddress].gateway = gateway;
        leased[requestedAddress].subnetMask = subnetMask;

        return &leased[requestedAddress];
    }

    // allocate new address from server's pool of available addresses
    for (unsigned int i = 0; i < maxNumOfClients; i++) {
        Ipv4Address ip(beginAddr + i);
        if (leased.find(ip) != leased.end()) {
            // lease exists but not allocated (e.g. expired or released)
            if (!leased[ip].leased)
                return &(leased[ip]);
        }
        else {
            // there is no expired lease so we create a new one
            leased[ip] = DhcpLease();
            leased[ip].ip = ip;
            leased[ip].gateway = gateway;
            leased[ip].subnetMask = subnetMask;
            return &(leased[ip]);
        }
    }
    // no lease available
    return nullptr;
}

void DhcpServer::sendToUDP(Packet *msg, int srcPort, const L3Address& destAddr, int destPort)
{
    EV_INFO << "Sending packet: " << msg << "." << endl;
    numSent++;
    msg->addTagIfAbsent<InterfaceReq>()->setInterfaceId(ie->getInterfaceId());
    socket.sendTo(msg, destAddr, destPort);
}

void DhcpServer::startApp()
{
    maxNumOfClients = par("maxNumClients");
    leaseTime = par("leaseTime");

    simtime_t start = std::max(startTime, simTime());
    ie = chooseInterface();
    Ipv4InterfaceData *ipv4data = ie->ipv4Data();
    if (ipv4data == nullptr)
        throw cRuntimeError("interface %s is not configured for IPv4", ie->getFullName());
    const char *gatewayStr = par("gateway");
    gateway = *gatewayStr ? L3AddressResolver().resolve(gatewayStr, L3AddressResolver::ADDR_IPv4).toIpv4() : ipv4data->getIPAddress();
    subnetMask = ipv4data->getNetmask();
    long numReservedAddresses = par("numReservedAddresses");
    uint32_t networkStartAddress = ipv4data->getIPAddress().getInt() & ipv4data->getNetmask().getInt();
    ipAddressStart = Ipv4Address(networkStartAddress + numReservedAddresses);
    if (!Ipv4Address::maskedAddrAreEqual(ipv4data->getIPAddress(), ipAddressStart, subnetMask))
        throw cRuntimeError("The numReservedAddresses parameter larger than address range");
    if (!Ipv4Address::maskedAddrAreEqual(ipv4data->getIPAddress(), Ipv4Address(ipAddressStart.getInt() + maxNumOfClients - 1), subnetMask))
        throw cRuntimeError("Not enough IP addresses in subnet for %d clients", maxNumOfClients);
    scheduleAt(start, startTimer);
}

void DhcpServer::stopApp()
{
    leased.clear();
    ie = nullptr;
    cancelEvent(startTimer);
    // socket.close(); TODO:
}

bool DhcpServer::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (static_cast<NodeStartOperation::Stage>(stage) == NodeStartOperation::STAGE_APPLICATION_LAYER) {
            startApp();
            isOperational = true;
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (static_cast<NodeShutdownOperation::Stage>(stage) == NodeShutdownOperation::STAGE_APPLICATION_LAYER) {
            stopApp();
            isOperational = false;
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (static_cast<NodeCrashOperation::Stage>(stage) == NodeCrashOperation::STAGE_CRASH) {
            stopApp();
            isOperational = false;
        }
    }
    else
        throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

} // namespace inet


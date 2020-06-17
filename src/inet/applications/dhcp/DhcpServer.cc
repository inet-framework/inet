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
#include "inet/common/Simsignals.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"

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
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        startTimer = new cMessage("Start DHCP server", START_DHCP);
        startTime = par("startTime");
        numSent = 0;
        numReceived = 0;
        WATCH(numSent);
        WATCH(numReceived);
        WATCH_MAP(leased);

        // DHCP UDP ports
        clientPort = 68;    // client
        serverPort = 67;    // server
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        cModule *host = getContainingNode(this);
        host->subscribe(interfaceDeletedSignal, this);
        socket.setOutputGate(gate("socketOut"));
        socket.setCallback(this);
    }
}

void DhcpServer::openSocket()
{
    if (!ie)
        throw cRuntimeError("Interface to listen does not exist. aborting");
    socket.bind(serverPort);
    socket.setBroadcast(true);
    EV_INFO << "DHCP server bound to port " << serverPort << endl;
}

void DhcpServer::receiveSignal(cComponent *source, int signalID, cObject *obj, cObject *details)
{
    Enter_Method_Silent();

    if (signalID == interfaceDeletedSignal) {
        if (isUp()) {
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
        ie = ift->findInterfaceByName(interfaceName);
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

void DhcpServer::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        handleSelfMessages(msg);
    }
    else if (msg->arrivedOn("socketIn")) {
        socket.processMessage(msg);
    }
    else
        throw cRuntimeError("Unknown incoming gate: '%s'", msg->getArrivalGate()->getFullName());
}

void DhcpServer::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    // process incoming packet
    processDhcpMessage(packet);
}

void DhcpServer::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Unknown message '" << indication->getName() << "', kind = " << indication->getKind() << ", discarding it." << endl;
    delete indication;
}

void DhcpServer::socketClosed(UdpSocket *socket_)
{
    if (operationalState == State::STOPPING_OPERATION && !socket.isOpen())
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void DhcpServer::handleSelfMessages(cMessage *msg)
{
    if (msg->getKind() == START_DHCP) {
        openSocket();
    }
    else
        throw cRuntimeError("Unknown selfmessage type!");
}

void DhcpServer::processDhcpMessage(Packet *packet)
{
    ASSERT(isUp() && ie != nullptr);

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
            if (dhcpMsg->getOptions().getServerIdentifier() == ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress()) {
                // the REQUEST is in response to an offering (because SERVER_ID is filled)
                // otherwise the msg is a request to extend an existing lease (e. g. INIT-REBOOT)

                DhcpLease *lease = getLeaseByMac(dhcpMsg->getChaddr());
                if (lease != nullptr) {
                    if (lease->ip != dhcpMsg->getOptions().getRequestedIp()) {
                        EV_ERROR << "The 'requested IP address' must be filled in with the 'yiaddr' value from the chosen DHCPOFFER." << endl;
                        sendNak(dhcpMsg);
                    }
                    else {
                        EV_INFO << "From now " << lease->ip << " is leased to " << lease->mac << "." << endl;
                        lease->xid = dhcpMsg->getXid();
                        lease->leaseTime = leaseTime;
                        lease->leased = true;

                        // TODO: final check before ACK (it is not necessary but recommended)
                        sendAck(lease, dhcpMsg);

                        // TODO: update the display string to inform how many clients are assigned
                    }
                }
                else {
                    EV_ERROR << "There is no available lease for " << dhcpMsg->getChaddr() << ". Probably, the client missed to send DHCPDISCOVER before DHCPREQUEST." << endl;
                    sendNak(dhcpMsg);
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
                        sendAck(lease, dhcpMsg);
                    }
                    else {
                        EV_ERROR << "The requested IP address is incorrect, or is on the wrong network." << endl;
                        sendNak(dhcpMsg);
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
                        sendAck(lease, dhcpMsg);
                    }
                    else {
                        EV_ERROR << "Renewal/rebinding process failed: requested IP address " << dhcpMsg->getCiaddr() << " not found in the server's database!" << endl;
                        sendNak(dhcpMsg);
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

void DhcpServer::sendNak(const Ptr<const DhcpMessage>& msg)
{
    // EV_INFO << "Sending NAK to " << lease->mac << "." << endl;
    Packet *pk = new Packet("DHCPNAK");
    const auto& nak = makeShared<DhcpMessage>();
    nak->setOp(BOOTREPLY);
    uint16_t length = 236;    // packet size without the options field
    nak->setHtype(1);    // ethernet
    nak->setHlen(6);    // hardware address length (6 octets)
    nak->setHops(0);
    nak->setXid(msg->getXid());    // transaction id from client
    nak->setSecs(0);    // 0 seconds from transaction started.
    nak->setBroadcast(msg->getBroadcast());
    nak->setGiaddr(msg->getGiaddr());    // next server IP
    nak->setChaddr(msg->getChaddr());
    nak->getOptionsForUpdate().setServerIdentifier(ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress());
    length += 6;
    nak->getOptionsForUpdate().setMessageType(DHCPNAK);
    length += 3;

    // magic cookie and the end field
    length += 5;

    nak->setChunkLength(B(length));

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

void DhcpServer::sendAck(DhcpLease *lease, const Ptr<const DhcpMessage>& packet)
{
    EV_INFO << "Sending the ACK to " << lease->mac << "." << endl;

    Packet *pk = new Packet("DHCPACK");
    const auto& ack = makeShared<DhcpMessage>();
    ack->setOp(BOOTREPLY);
    uint16_t length = 236;    // packet size without the options field
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
    length += 3;

    // add the lease options
    ack->getOptionsForUpdate().setSubnetMask(lease->subnetMask);
    length += 6;
    ack->getOptionsForUpdate().setRenewalTime(SimTime(leaseTime * 0.5).trunc(SIMTIME_S));    // RFC 4.4.5
    length += 6;
    ack->getOptionsForUpdate().setRebindingTime(SimTime(leaseTime * 0.875).trunc(SIMTIME_S));
    length += 6;
    ack->getOptionsForUpdate().setLeaseTime(SimTime(leaseTime).trunc(SIMTIME_S));
    length += 6;
    ack->getOptionsForUpdate().setRouterArraySize(1);
    ack->getOptionsForUpdate().setRouter(0, lease->gateway);
    length += (2 + 1 * sizeof(uint32_t));
    ack->getOptionsForUpdate().setDnsArraySize(1);
    ack->getOptionsForUpdate().setDns(0, lease->dns);
    length += (2 + 1 * sizeof(uint32_t));

    // add the server ID as the RFC says
    ack->getOptionsForUpdate().setServerIdentifier(ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress());
    length += 6;

    // magic cookie and the end field
    length += 5;

    ack->setChunkLength(B(length));

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
    uint16_t length = 236;    // packet size without the options field
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
    length += 3;

    // add the offer options
    offer->getOptionsForUpdate().setSubnetMask(lease->subnetMask);
    length += 6;
    offer->getOptionsForUpdate().setRenewalTime(SimTime(leaseTime * 0.5).trunc(SIMTIME_S));    // RFC 4.4.5
    length += 6;
    offer->getOptionsForUpdate().setRebindingTime(SimTime(leaseTime * 0.875).trunc(SIMTIME_S));
    length += 6;
    offer->getOptionsForUpdate().setLeaseTime(SimTime(leaseTime).trunc(SIMTIME_S));
    length += 6;
    offer->getOptionsForUpdate().setRouterArraySize(1);
    offer->getOptionsForUpdate().setRouter(0, lease->gateway);
    length += (2 + 1 * sizeof(uint32_t));
    offer->getOptionsForUpdate().setDnsArraySize(1);
    offer->getOptionsForUpdate().setDns(0, lease->dns);
    length += (2 + 1 * sizeof(uint32_t));

    // add the server_id as the RFC says
    offer->getOptionsForUpdate().setServerIdentifier(ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress());
    length += 6;

    // magic cookie and the end field
    length += 5;

    offer->setChunkLength(B(length));

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

void DhcpServer::handleStartOperation(LifecycleOperation *operation)
{
    maxNumOfClients = par("maxNumClients");
    leaseTime = par("leaseTime");

    simtime_t start = std::max(startTime, simTime());
    ie = chooseInterface();
    auto ipv4data = ie->findProtocolData<Ipv4InterfaceData>();
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

void DhcpServer::handleStopOperation(LifecycleOperation *operation)
{
    leased.clear();
    ie = nullptr;
    cancelEvent(startTimer);
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void DhcpServer::handleCrashOperation(LifecycleOperation *operation)
{
    leased.clear();
    ie = nullptr;
    cancelEvent(startTimer);
    if (operation->getRootModule() != getContainingNode(this))     // closes socket when the application crashed only
        socket.destroy();         //TODO  in real operating systems, program crash detected by OS and OS closes sockets of crashed programs.
}

} // namespace inet


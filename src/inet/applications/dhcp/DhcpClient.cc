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

#include "inet/applications/dhcp/DhcpClient.h"
#include "inet/common/Simsignals.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

namespace inet {

Define_Module(DhcpClient);

DhcpClient::~DhcpClient()
{
    cancelAndDelete(timerT1);
    cancelAndDelete(timerTo);
    cancelAndDelete(timerT2);
    cancelAndDelete(leaseTimer);
    cancelAndDelete(startTimer);
    delete lease;
}

void DhcpClient::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        timerT1 = new cMessage("T1 Timer", T1);
        timerT2 = new cMessage("T2 Timer", T2);
        timerTo = new cMessage("DHCP Timeout");
        leaseTimer = new cMessage("Lease Timeout", LEASE_TIMEOUT);
        startTimer = new cMessage("Starting DHCP", START_DHCP);
        startTime = par("startTime");
        numSent = 0;
        numReceived = 0;
        xid = 0;
        responseTimeout = 60;    // response timeout in seconds RFC 2131, 4.4.3

        WATCH(numSent);
        WATCH(numReceived);
        WATCH(clientState);
        WATCH(xid);

        // DHCP UDP ports
        clientPort = 68;    // client
        serverPort = 67;    // server
        // get the routing table to update and subscribe it to the blackboard
        irt = getModuleFromPar<IIpv4RoutingTable>(par("routingTableModule"), this);
        // set client to idle state
        clientState = IDLE;
        // get the interface to configure
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        // get the hostname
        host = getContainingNode(this);
        // for a wireless interface subscribe the association event to start the DHCP protocol
        host->subscribe(l2AssociatedSignal, this);
        host->subscribe(interfaceDeletedSignal, this);
        socket.setCallback(this);
        socket.setOutputGate(gate("socketOut"));
    }
}

InterfaceEntry *DhcpClient::chooseInterface()
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
        // there should be exactly one non-loopback interface that we want to configure
        for (int i = 0; i < ift->getNumInterfaces(); i++) {
            InterfaceEntry *current = ift->getInterface(i);
            if (!current->isLoopback()) {
                if (ie)
                    throw cRuntimeError("Multiple non-loopback interfaces found, please select explicitly which one you want to configure via DHCP");
                ie = current;
            }
        }
        if (!ie)
            throw cRuntimeError("No non-loopback interface found to be configured via DHCP");
    }

    if (!ie->getProtocolData<Ipv4InterfaceData>()->getIPAddress().isUnspecified())
        throw cRuntimeError("Refusing to start DHCP on interface \"%s\" that already has an IP address", ie->getInterfaceName());
    return ie;
}

void DhcpClient::finish()
{
    cancelEvent(timerT1);
    cancelEvent(timerTo);
    cancelEvent(timerT2);
    cancelEvent(leaseTimer);
    cancelEvent(startTimer);
}

namespace {
static bool routeMatches(const Ipv4Route *entry, const Ipv4Address& target, const Ipv4Address& nmask,
        const Ipv4Address& gw, int metric, const char *dev)
{
    if (!target.equals(entry->getDestination()))
        return false;
    if (!nmask.equals(entry->getNetmask()))
        return false;
    if (!gw.equals(entry->getGateway()))
        return false;
    if (metric && metric != entry->getMetric())
        return false;
    if (strcmp(dev, entry->getInterfaceName()))
        return false;

    return true;
}
}

const char *DhcpClient::getStateName(ClientState state)
{
    switch (state) {
#define CASE(X)    case X: \
        return #X;
        CASE(INIT);
        CASE(INIT_REBOOT);
        CASE(REBOOTING);
        CASE(SELECTING);
        CASE(REQUESTING);
        CASE(BOUND);
        CASE(RENEWING);
        CASE(REBINDING);

        default:
            return "???";
#undef CASE
    }
}

const char *DhcpClient::getAndCheckMessageTypeName(DhcpMessageType type)
{
    switch (type) {
#define CASE(X)    case X: \
        return #X;
        CASE(DHCPDISCOVER);
        CASE(DHCPOFFER);
        CASE(DHCPREQUEST);
        CASE(DHCPDECLINE);
        CASE(DHCPACK);
        CASE(DHCPNAK);
        CASE(DHCPRELEASE);
        CASE(DHCPINFORM);

        default:
            throw cRuntimeError("Unknown or invalid DHCP message type %d", type);
#undef CASE
    }
}

void DhcpClient::refreshDisplay() const
{
    ApplicationBase::refreshDisplay();

    getDisplayString().setTagArg("t", 0, getStateName(clientState));
}

void DhcpClient::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        handleTimer(msg);
    }
    else if (msg->arrivedOn("socketIn")) {
        socket.processMessage(msg);
    }
    else
        throw cRuntimeError("Unknown incoming gate: '%s'", msg->getArrivalGate()->getFullName());
}

void DhcpClient::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    // process incoming packet
    handleDhcpMessage(packet);
    delete packet;
}

void DhcpClient::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
    delete indication;
}

void DhcpClient::socketClosed(UdpSocket *socket_)
{
    if (operationalState == State::STOPPING_OPERATION && !socket.isOpen())
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void DhcpClient::handleTimer(cMessage *msg)
{
    int category = msg->getKind();

    if (category == START_DHCP) {
        openSocket();
        if (lease) {
            clientState = INIT_REBOOT;
            initRebootedClient();
        }
        else {    // we have no lease from the previous DHCP process
            clientState = INIT;
            initClient();
        }
    }
    else if (category == WAIT_OFFER) {
        EV_DETAIL << "No DHCP offer received within timeout. Restarting. " << endl;
        initClient();
    }
    else if (category == WAIT_ACK) {
        EV_DETAIL << "No DHCP ACK received within timeout. Restarting." << endl;
        initClient();
    }
    else if (category == T1) {
        EV_DETAIL << "T1 expired. Starting RENEWING state." << endl;
        clientState = RENEWING;
        scheduleTimerTO(WAIT_ACK);
        sendRequest();
    }
    else if (category == T2 && clientState == RENEWING) {
        EV_DETAIL << "T2 expired. Starting REBINDING state." << endl;
        clientState = REBINDING;
        cancelEvent(timerT1);
        cancelEvent(timerT2);
        cancelEvent(timerTo);
        //cancelEvent(leaseTimer);

        sendRequest();
        scheduleTimerTO(WAIT_ACK);
    }
    else if (category == T2) {
        // T1 < T2 always holds by definition and when T1 expires client will move to RENEWING
        throw cRuntimeError("T2 occurred in wrong state. (T1 must be earlier than T2.)");
    }
    else if (category == LEASE_TIMEOUT) {
        EV_INFO << "Lease has expired. Starting DHCP process in INIT state." << endl;
        unbindLease();
        clientState = INIT;
        initClient();
    }
    else
        throw cRuntimeError("Unknown self message '%s'", msg->getName());
}

void DhcpClient::recordOffer(const Ptr<const DhcpMessage>& dhcpOffer)
{
    if (!dhcpOffer->getYiaddr().isUnspecified()) {
        Ipv4Address ip = dhcpOffer->getYiaddr();

        //Byte serverIdB = dhcpOffer->getOptions().get(SERVER_ID);
        Ipv4Address serverId = dhcpOffer->getOptions().getServerIdentifier();

        // minimal information to configure the interface
        // create the lease to request
        delete lease;
        lease = new DhcpLease();
        lease->ip = ip;
        lease->mac = macAddress;
        lease->serverId = serverId;
    }
    else
        EV_WARN << "DHCPOFFER arrived, but no IP address has been offered. Discarding it and remaining in SELECTING." << endl;
}

void DhcpClient::recordLease(const Ptr<const DhcpMessage>& dhcpACK)
{
    if (!dhcpACK->getYiaddr().isUnspecified()) {
        Ipv4Address ip = dhcpACK->getYiaddr();
        EV_DETAIL << "DHCPACK arrived with " << "IP: " << ip << endl;

        lease->subnetMask = dhcpACK->getOptions().getSubnetMask();

        if (dhcpACK->getOptions().getRouterArraySize() > 0)
            lease->gateway = dhcpACK->getOptions().getRouter(0);
        if (dhcpACK->getOptions().getDnsArraySize() > 0)
            lease->dns = dhcpACK->getOptions().getDns(0);
        if (dhcpACK->getOptions().getNtpArraySize() > 0)
            lease->ntp = dhcpACK->getOptions().getNtp(0);

        lease->leaseTime = dhcpACK->getOptions().getLeaseTime();
        lease->renewalTime = dhcpACK->getOptions().getRenewalTime();
        lease->rebindTime = dhcpACK->getOptions().getRebindingTime();

        // std::cout << lease->leaseTime << " " << lease->renewalTime << " " << lease->rebindTime << endl;
    }
    else
        EV_ERROR << "DHCPACK arrived, but no IP address confirmed." << endl;
}

void DhcpClient::bindLease()
{
    ie->getProtocolData<Ipv4InterfaceData>()->setIPAddress(lease->ip);
    ie->getProtocolData<Ipv4InterfaceData>()->setNetmask(lease->subnetMask);

    std::string banner = "Got IP " + lease->ip.str();
    host->bubble(banner.c_str());

    /*
        The client SHOULD perform a final check on the parameters (ping, Arp).
        If the client detects that the address is already in use:
        EV_INFO << "The offered IP " << lease->ip << " is not available." << endl;
        sendDecline(lease->ip);
        initClient();
     */

    EV_INFO << "The requested IP " << lease->ip << "/" << lease->subnetMask << " is available. Assigning it to "
            << host->getFullName() << "." << endl;

    Ipv4Route *iroute = nullptr;
    for (int i = 0; i < irt->getNumRoutes(); i++) {
        Ipv4Route *e = irt->getRoute(i);
        if (routeMatches(e, Ipv4Address(), Ipv4Address(), lease->gateway, 0, ie->getInterfaceName())) {
            iroute = e;
            break;
        }
    }
    if (iroute == nullptr) {
        // create gateway route
        route = new Ipv4Route();
        route->setDestination(Ipv4Address());
        route->setNetmask(Ipv4Address());
        route->setGateway(lease->gateway);
        route->setInterface(ie);
        route->setSourceType(Ipv4Route::MANUAL);
        irt->addRoute(route);
    }

    // update the routing table
    cancelEvent(leaseTimer);
    scheduleAt(simTime() + lease->leaseTime, leaseTimer);
}

void DhcpClient::unbindLease()
{
    EV_INFO << "Unbinding lease on " << ie->getInterfaceName() << "." << endl;

    cancelEvent(timerT1);
    cancelEvent(timerT2);
    cancelEvent(timerTo);
    cancelEvent(leaseTimer);

    irt->deleteRoute(route);
    ie->getProtocolData<Ipv4InterfaceData>()->setIPAddress(Ipv4Address());
    ie->getProtocolData<Ipv4InterfaceData>()->setNetmask(Ipv4Address::ALLONES_ADDRESS);
}

void DhcpClient::initClient()
{
    EV_INFO << "Starting DHCP configuration process." << endl;

    cancelEvent(timerT1);
    cancelEvent(timerT2);
    cancelEvent(timerTo);
    cancelEvent(leaseTimer);

    sendDiscover();
    scheduleTimerTO(WAIT_OFFER);
    clientState = SELECTING;
}

void DhcpClient::initRebootedClient()
{
    sendRequest();
    scheduleTimerTO(WAIT_ACK);
    clientState = REBOOTING;
}

void DhcpClient::handleDhcpMessage(Packet *packet)
{
    ASSERT(isUp() && ie != nullptr);

    const auto& msg = packet->peekAtFront<DhcpMessage>();
    if (msg->getOp() != BOOTREPLY) {
        EV_WARN << "Client received a non-BOOTREPLY message, dropping." << endl;
        return;
    }

    if (msg->getXid() != xid) {
        EV_WARN << "Message transaction ID is not valid, dropping." << endl;
        return;
    }

    DhcpMessageType messageType = msg->getOptions().getMessageType();
    switch (clientState) {
        case INIT:
            EV_WARN << getAndCheckMessageTypeName(messageType) << " message arrived in INIT state. In this state, client does not wait for any message at all, dropping." << endl;
            break;

        case SELECTING:
            if (messageType == DHCPOFFER) {
                EV_INFO << "DHCPOFFER message arrived in SELECTING state with IP address: " << msg->getYiaddr() << "." << endl;
                scheduleTimerTO(WAIT_ACK);
                clientState = REQUESTING;
                recordOffer(msg);
                sendRequest();    // we accept the first offer
            }
            else
                EV_WARN << "Client is in SELECTING and the arriving packet is not a DHCPOFFER, dropping." << endl;
            break;

        case REQUESTING:
            if (messageType == DHCPOFFER) {
                EV_WARN << "We don't accept DHCPOFFERs in REQUESTING state, dropping." << endl;    // remains in REQUESTING
            }
            else if (messageType == DHCPACK) {
                EV_INFO << "DHCPACK message arrived in REQUESTING state. The requested IP address is available in the server's pool of addresses." << endl;
                handleDhcpAck(msg);
                clientState = BOUND;
            }
            else if (messageType == DHCPNAK) {
                EV_INFO << "DHCPNAK message arrived in REQUESTING state. Restarting the configuration process." << endl;
                initClient();
            }
            else {
                EV_WARN << getAndCheckMessageTypeName(messageType) << " message arrived in REQUESTING state. In this state, client does not expect messages of this type, dropping." << endl;
            }
            break;

        case BOUND:
            EV_DETAIL << "We are in BOUND, discard all DHCP messages." << endl;    // remain in BOUND
            break;

        case RENEWING:
            if (messageType == DHCPACK) {
                handleDhcpAck(msg);
                EV_INFO << "DHCPACK message arrived in RENEWING state. The renewing process was successful." << endl;
                clientState = BOUND;
            }
            else if (messageType == DHCPNAK) {
                EV_INFO << "DHPCNAK message arrived in RENEWING state. The renewing process was unsuccessful. Restarting the DHCP configuration process." << endl;
                unbindLease();
                initClient();
            }
            else {
                EV_WARN << getAndCheckMessageTypeName(messageType) << " message arrived in RENEWING state. In this state, client does not expect messages of this type, dropping." << endl;
            }
            break;

        case REBINDING:
            if (messageType == DHCPNAK) {
                EV_INFO << "DHPCNAK message arrived in REBINDING state. The rebinding process was unsuccessful. Restarting the DHCP configuration process." << endl;
                unbindLease();
                initClient();
            }
            else if (messageType == DHCPACK) {
                handleDhcpAck(msg);
                EV_INFO << "DHCPACK message arrived in REBINDING state. The rebinding process was successful." << endl;
                clientState = BOUND;
            }
            else {
                EV_WARN << getAndCheckMessageTypeName(messageType) << " message arrived in REBINDING state. In this state, client does not expect messages of this type, dropping." << endl;
            }
            break;

        case REBOOTING:
            if (messageType == DHCPACK) {
                handleDhcpAck(msg);
                EV_INFO << "DHCPACK message arrived in REBOOTING state. Initialization with known IP address was successful." << endl;
                clientState = BOUND;
            }
            else if (messageType == DHCPNAK) {
                EV_INFO << "DHCPNAK message arrived in REBOOTING. Initialization with known IP address was unsuccessful." << endl;
                unbindLease();
                initClient();
            }
            else {
                EV_WARN << getAndCheckMessageTypeName(messageType) << " message arrived in REBOOTING state. In this state, client does not expect messages of this type, dropping." << endl;
            }
            break;

        default:
            throw cRuntimeError("Unknown or invalid client state %d", clientState);
    }
}

void DhcpClient::receiveSignal(cComponent *source, int signalID, cObject *obj, cObject *details)
{
    Enter_Method_Silent();
    printSignalBanner(signalID, obj, details);

    // host associated. link is up. change the state to init.
    if (signalID == l2AssociatedSignal) {
        InterfaceEntry *associatedIE = check_and_cast_nullable<InterfaceEntry *>(obj);
        if (associatedIE && ie == associatedIE && clientState != IDLE) {
            EV_INFO << "Interface associated, starting DHCP." << endl;
            unbindLease();
            initClient();
        }
    }
    else if (signalID == interfaceDeletedSignal) {
        if (isUp())
            throw cRuntimeError("Reacting to interface deletions is not implemented in this module");
    }
}

void DhcpClient::sendRequest()
{
    // setting the xid
    xid = intuniform(0, RAND_MAX);    // generating a new xid for each transmission

    const auto& request = makeShared<DhcpMessage>();
    request->setOp(BOOTREQUEST);
    uint16_t length = 236;  // packet size without the options field
    request->setHtype(1);    // ethernet
    request->setHlen(6);    // hardware Address length (6 octets)
    request->setHops(0);
    request->setXid(xid);    // transaction id
    request->setSecs(0);    // 0 seconds from transaction started
    request->setBroadcast(false);    // unicast
    request->setYiaddr(Ipv4Address());    // no 'your IP' addr
    request->setGiaddr(Ipv4Address());    // no DHCP Gateway Agents
    request->setChaddr(macAddress);    // my mac address;
    request->setSname("");    // no server name given
    request->setFile("");    // no file given
    auto& options = request->getOptionsForUpdate();
    options.setMessageType(DHCPREQUEST);
    length += 3;
    options.setClientIdentifier(macAddress);
    length += 9;

    // set the parameters to request
    options.setParameterRequestListArraySize(4);
    options.setParameterRequestList(0, SUBNET_MASK);
    options.setParameterRequestList(1, ROUTER);
    options.setParameterRequestList(2, DNS);
    options.setParameterRequestList(3, NTP_SRV);
    length += (2 + options.getParameterRequestListArraySize());

    L3Address destAddr;

    // RFC 4.3.6 Table 4
    if (clientState == INIT_REBOOT) {
        options.setRequestedIp(lease->ip);
        length += 6;
        request->setCiaddr(Ipv4Address());    // zero
        destAddr = Ipv4Address::ALLONES_ADDRESS;
        EV_INFO << "Sending DHCPREQUEST asking for IP " << lease->ip << " via broadcast." << endl;
    }
    else if (clientState == REQUESTING) {
        options.setServerIdentifier(lease->serverId);
        length += 6;
        options.setRequestedIp(lease->ip);
        length += 6;
        request->setCiaddr(Ipv4Address());    // zero
        destAddr = Ipv4Address::ALLONES_ADDRESS;
        EV_INFO << "Sending DHCPREQUEST asking for IP " << lease->ip << " via broadcast." << endl;
    }
    else if (clientState == RENEWING) {
        request->setCiaddr(lease->ip);    // the client IP
        destAddr = lease->serverId;
        EV_INFO << "Sending DHCPREQUEST extending lease for IP " << lease->ip << " via unicast to " << lease->serverId << "." << endl;
    }
    else if (clientState == REBINDING) {
        request->setCiaddr(lease->ip);    // the client IP
        destAddr = Ipv4Address::ALLONES_ADDRESS;
        EV_INFO << "Sending DHCPREQUEST renewing the IP " << lease->ip << " via broadcast." << endl;
    }
    else
        throw cRuntimeError("Invalid state");

    // magic cookie and the end field
    length += 5;

    request->setChunkLength(B(length));

    Packet *packet = new Packet("DHCPREQUEST");
    packet->insertAtBack(request);
    sendToUdp(packet, clientPort, destAddr, serverPort);
}

void DhcpClient::sendDiscover()
{
    // setting the xid
    xid = intuniform(0, RAND_MAX);
    //std::cout << xid << endl;
    Packet *packet = new Packet("DHCPDISCOVER");
    const auto& discover = makeShared<DhcpMessage>();
    discover->setOp(BOOTREQUEST);
    uint16_t length = 236;      // packet size without the options field
    discover->setHtype(1);    // ethernet
    discover->setHlen(6);    // hardware Address lenght (6 octets)
    discover->setHops(0);
    discover->setXid(xid);    // transaction id
    discover->setSecs(0);    // 0 seconds from transaction started
    discover->setBroadcast(false);    // unicast
    discover->setChaddr(macAddress);    // my mac address
    discover->setSname("");    // no server name given
    discover->setFile("");    // no file given
    auto& options = discover->getOptionsForUpdate();
    options.setMessageType(DHCPDISCOVER);
    length += 3;
    options.setClientIdentifier(macAddress);
    length += 9;
    options.setRequestedIp(Ipv4Address());
    //length += 6; not added because unspecified

    // set the parameters to request
    options.setParameterRequestListArraySize(4);
    options.setParameterRequestList(0, SUBNET_MASK);
    options.setParameterRequestList(1, ROUTER);
    options.setParameterRequestList(2, DNS);
    options.setParameterRequestList(3, NTP_SRV);
    length += (2 + options.getParameterRequestListArraySize());

    // magic cookie and the end field
    length += 5;

    discover->setChunkLength(B(length));

    packet->insertAtBack(discover);

    EV_INFO << "Sending DHCPDISCOVER." << endl;
    sendToUdp(packet, clientPort, Ipv4Address::ALLONES_ADDRESS, serverPort);
}

void DhcpClient::sendDecline(Ipv4Address declinedIp)
{
    xid = intuniform(0, RAND_MAX);
    Packet *packet = new Packet("DHCPDECLINE");
    const auto& decline = makeShared<DhcpMessage>();
    decline->setOp(BOOTREQUEST);
    uint16_t length = 236;    // packet size without the options field
    decline->setHtype(1);    // ethernet
    decline->setHlen(6);    // hardware Address length (6 octets)
    decline->setHops(0);
    decline->setXid(xid);    // transaction id
    decline->setSecs(0);    // 0 seconds from transaction started
    decline->setBroadcast(false);    // unicast
    decline->setChaddr(macAddress);    // my MAC address
    decline->setSname("");    // no server name given
    decline->setFile("");    // no file given
    auto& options = decline->getOptionsForUpdate();
    options.setMessageType(DHCPDECLINE);
    length += 3;
    options.setRequestedIp(declinedIp);
    length += 6;

    // magic cookie and the end field
    length += 5;

    decline->setChunkLength(B(length));

    packet->insertAtBack(decline);

    EV_INFO << "Sending DHCPDECLINE." << endl;
    sendToUdp(packet, clientPort, Ipv4Address::ALLONES_ADDRESS, serverPort);
}

void DhcpClient::handleDhcpAck(const Ptr<const DhcpMessage>& msg)
{
    recordLease(msg);
    cancelEvent(timerTo);
    scheduleTimerT1();
    scheduleTimerT2();
    bindLease();
}

void DhcpClient::scheduleTimerTO(DhcpTimerType type)
{
    // cancel the previous timeout
    cancelEvent(timerTo);
    timerTo->setKind(type);
    scheduleAt(simTime() + responseTimeout, timerTo);
}

void DhcpClient::scheduleTimerT1()
{
    // cancel the previous T1
    cancelEvent(timerT1);
    scheduleAt(simTime() + (lease->renewalTime), timerT1);    // RFC 2131 4.4.5
}

void DhcpClient::scheduleTimerT2()
{
    // cancel the previous T2
    cancelEvent(timerT2);
    scheduleAt(simTime() + (lease->rebindTime), timerT2);    // RFC 2131 4.4.5
}

void DhcpClient::sendToUdp(Packet *msg, int srcPort, const L3Address& destAddr, int destPort)
{
    EV_INFO << "Sending packet " << msg << endl;
    msg->addTagIfAbsent<InterfaceReq>()->setInterfaceId(ie->getInterfaceId());
    socket.sendTo(msg, destAddr, destPort);
}

void DhcpClient::openSocket()
{
    socket.bind(clientPort);
    socket.setBroadcast(true);
    EV_INFO << "DHCP server bound to port " << serverPort << "." << endl;
}

void DhcpClient::handleStartOperation(LifecycleOperation *operation)
{
    simtime_t start = std::max(startTime, simTime());
    ie = chooseInterface();
    macAddress = ie->getMacAddress();
    scheduleAt(start, startTimer);
}

void DhcpClient::handleStopOperation(LifecycleOperation *operation)
{
    cancelEvent(timerT1);
    cancelEvent(timerT2);
    cancelEvent(timerTo);
    cancelEvent(leaseTimer);
    cancelEvent(startTimer);
    ie = nullptr;

    // TODO: Client should send DHCPRELEASE to the server. However, the correct operation
    // of DHCP does not depend on the transmission of DHCPRELEASE messages.

    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void DhcpClient::handleCrashOperation(LifecycleOperation *operation)
{
    cancelEvent(timerT1);
    cancelEvent(timerT2);
    cancelEvent(timerTo);
    cancelEvent(leaseTimer);
    cancelEvent(startTimer);
    ie = nullptr;

    if (operation->getRootModule() != getContainingNode(this))     // closes socket when the application crashed only
        socket.destroy();         //TODO  in real operating systems, program crash detected by OS and OS closes sockets of crashed programs.
}

} // namespace inet


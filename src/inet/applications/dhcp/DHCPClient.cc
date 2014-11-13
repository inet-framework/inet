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

#include "inet/applications/dhcp/DHCPClient.h"

#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/NotifierConsts.h"
#include "inet/common/lifecycle/NodeOperations.h"

namespace inet {

Define_Module(DHCPClient);

DHCPClient::DHCPClient() :
    host(NULL),
    ie(NULL),
    irt(NULL),
    timerT1(NULL),
    timerT2(NULL),
    timerTo(NULL),
    leaseTimer(NULL),
    startTimer(NULL),
    lease(NULL),
    route(NULL)
{
}

DHCPClient::~DHCPClient()
{
    cancelAndDelete(timerT1);
    cancelAndDelete(timerTo);
    cancelAndDelete(timerT2);
    cancelAndDelete(leaseTimer);
    cancelAndDelete(startTimer);
    delete lease;
}

void DHCPClient::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        timerT1 = new cMessage("T1 Timer", T1);
        timerT2 = new cMessage("T2 Timer", T2);
        timerTo = new cMessage("DHCP Timeout");
        leaseTimer = new cMessage("Lease Timeout", LEASE_TIMEOUT);
        startTimer = new cMessage("Starting DHCP", START_DHCP);
        startTime = par("startTime");
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;

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

        // get the hostname
        host = getContainingNode(this);
        hostName = host->getFullName();

        // for a wireless interface subscribe the association event to start the DHCP protocol
        host->subscribe(NF_L2_ASSOCIATED, this);
        host->subscribe(NF_INTERFACE_DELETED, this);

        // get the routing table to update and subscribe it to the blackboard
        irt = getModuleFromPar<IIPv4RoutingTable>(par("routingTableModule"), this);
        // set client to idle state
        clientState = IDLE;
        // get the interface to configure

        if (isOperational) {
            ie = chooseInterface();
            // grab the interface MAC address
            macAddress = ie->getMacAddress();
            startApp();
        }
    }
}

InterfaceEntry *DHCPClient::chooseInterface()
{
    IInterfaceTable *ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    const char *interfaceName = par("interface");
    InterfaceEntry *ie = NULL;

    if (strlen(interfaceName) > 0) {
        ie = ift->getInterfaceByName(interfaceName);
        if (ie == NULL)
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

    if (!ie->ipv4Data()->getIPAddress().isUnspecified())
        throw cRuntimeError("Refusing to start DHCP on interface \"%s\" that already has an IP address", ie->getName());
    return ie;
}

void DHCPClient::finish()
{
    cancelEvent(timerT1);
    cancelEvent(timerTo);
    cancelEvent(timerT2);
    cancelEvent(leaseTimer);
    cancelEvent(startTimer);
}

static bool routeMatches(const IPv4Route *entry, const IPv4Address& target, const IPv4Address& nmask,
        const IPv4Address& gw, int metric, const char *dev)
{
    if (!target.isUnspecified() && !target.equals(entry->getDestination()))
        return false;
    if (!nmask.isUnspecified() && !nmask.equals(entry->getNetmask()))
        return false;
    if (!gw.isUnspecified() && !gw.equals(entry->getGateway()))
        return false;
    if (metric && metric != entry->getMetric())
        return false;
    if (dev && strcmp(dev, entry->getInterfaceName()))
        return false;

    return true;
}

const char *DHCPClient::getStateName(ClientState state)
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

const char *DHCPClient::getAndCheckMessageTypeName(DHCPMessageType type)
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

void DHCPClient::updateDisplayString()
{
    getDisplayString().setTagArg("t", 0, getStateName(clientState));
}

void DHCPClient::handleMessage(cMessage *msg)
{
    if (!isOperational) {
        EV_ERROR << "Message '" << msg << "' arrived when module status is down, dropping." << endl;
        delete msg;
        return;
    }
    if (msg->isSelfMessage()) {
        handleTimer(msg);
    }
    else if (msg->arrivedOn("udpIn")) {
        DHCPMessage *dhcpPacket = dynamic_cast<DHCPMessage *>(msg);
        if (!dhcpPacket)
            throw cRuntimeError(dhcpPacket, "Unexpected packet received (not a DHCPMessage)");

        handleDHCPMessage(dhcpPacket);
        delete msg;
    }

    if (ev.isGUI())
        updateDisplayString();
}

void DHCPClient::handleTimer(cMessage *msg)
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
        cancelEvent(leaseTimer);

        sendRequest();
        scheduleTimerTO(WAIT_ACK);
    }
    else if (category == T2) {
        // T1 < T2 always holds by definition and when T1 expires client will move to RENEWING
        throw cRuntimeError("T2 occurred in wrong state. (T1 must be earlier than T2.)");
    }
    else if (category == LEASE_TIMEOUT) {
        EV_INFO << "Lease has expired. Starting DHCP process in INIT state." << endl;
        unboundLease();
        clientState = INIT;
        initClient();
    }
    else
        throw cRuntimeError("Unknown self message '%s'", msg->getName());
}

void DHCPClient::recordOffer(DHCPMessage *dhcpOffer)
{
    if (!dhcpOffer->getYiaddr().isUnspecified()) {
        IPv4Address ip = dhcpOffer->getYiaddr();

        //Byte serverIdB = dhcpOffer->getOptions().get(SERVER_ID);
        IPv4Address serverId = dhcpOffer->getOptions().getServerIdentifier();

        // minimal information to configure the interface
        // create the lease to request
        delete lease;
        lease = new DHCPLease();
        lease->ip = ip;
        lease->mac = macAddress;
        lease->serverId = serverId;
    }
    else
        EV_WARN << "DHCPOFFER arrived, but no IP address has been offered. Discarding it and remaining in SELECTING." << endl;
}

void DHCPClient::recordLease(DHCPMessage *dhcpACK)
{
    if (!dhcpACK->getYiaddr().isUnspecified()) {
        IPv4Address ip = dhcpACK->getYiaddr();
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

void DHCPClient::bindLease()
{
    ie->ipv4Data()->setIPAddress(lease->ip);
    ie->ipv4Data()->setNetmask(lease->ip.getNetworkMask());

    std::string banner = "Got IP " + lease->ip.str();
    this->getParentModule()->bubble(banner.c_str());

    /*
        The client SHOULD perform a final check on the parameters (ping, ARP).
        If the client detects that the address is already in use:
        EV_INFO << "The offered IP " << lease->ip << " is not available." << endl;
        sendDecline(lease->ip);
        initClient();
     */

    EV_INFO << "The requested IP " << lease->ip << "/" << lease->subnetMask << " is available. Assigning it to "
            << this->getParentModule()->getFullName() << "." << endl;

    IPv4Route *iroute = NULL;
    for (int i = 0; i < irt->getNumRoutes(); i++) {
        IPv4Route *e = irt->getRoute(i);
        if (routeMatches(e, IPv4Address(), IPv4Address(), lease->gateway, 0, ie->getName())) {
            iroute = e;
            break;
        }
    }
    if (iroute == NULL) {
        // create gateway route
        route = new IPv4Route();
        route->setDestination(IPv4Address());
        route->setNetmask(IPv4Address());
        route->setGateway(lease->gateway);
        route->setInterface(ie);
        route->setSourceType(IPv4Route::MANUAL);
        irt->addRoute(route);
    }

    // update the routing table
    cancelEvent(leaseTimer);
    scheduleAt(simTime() + lease->leaseTime, leaseTimer);
}

void DHCPClient::unboundLease()
{
    EV_INFO << "Unbound lease on " << ie->getName() << "." << endl;

    cancelEvent(timerT1);
    cancelEvent(timerT2);
    cancelEvent(timerTo);
    cancelEvent(leaseTimer);

    irt->deleteRoute(route);
    ie->ipv4Data()->setIPAddress(IPv4Address());
    ie->ipv4Data()->setNetmask(IPv4Address::ALLONES_ADDRESS);
}

void DHCPClient::initClient()
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

void DHCPClient::initRebootedClient()
{
    sendRequest();
    scheduleTimerTO(WAIT_ACK);
    clientState = REBOOTING;
}

void DHCPClient::handleDHCPMessage(DHCPMessage *msg)
{
    ASSERT(isOperational && ie != NULL);

    if (msg->getOp() != BOOTREPLY) {
        EV_WARN << "Client received a non-BOOTREPLY message, dropping." << endl;
        return;
    }

    if (msg->getXid() != xid) {
        EV_WARN << "Message transaction ID is not valid, dropping." << endl;
        return;
    }

    DHCPMessageType messageType = (DHCPMessageType)msg->getOptions().getMessageType();
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
                handleDHCPACK(msg);
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
                handleDHCPACK(msg);
                EV_INFO << "DHCPACK message arrived in RENEWING state. The renewing process was successful." << endl;
                clientState = BOUND;
            }
            else if (messageType == DHCPNAK) {
                EV_INFO << "DHPCNAK message arrived in RENEWING state. The renewing process was unsuccessful. Restarting the DHCP configuration process." << endl;
                unboundLease();    // halt network (remove address)
                initClient();
            }
            else {
                EV_WARN << getAndCheckMessageTypeName(messageType) << " message arrived in RENEWING state. In this state, client does not expect messages of this type, dropping." << endl;
            }
            break;

        case REBINDING:
            if (messageType == DHCPNAK) {
                EV_INFO << "DHPCNAK message arrived in REBINDING state. The rebinding process was unsuccessful. Restarting the DHCP configuration process." << endl;
                unboundLease();    // halt network (remove address)
                initClient();
            }
            else if (messageType == DHCPACK) {
                handleDHCPACK(msg);
                EV_INFO << "DHCPACK message arrived in REBINDING state. The rebinding process was successful." << endl;
                clientState = BOUND;
            }
            else {
                EV_WARN << getAndCheckMessageTypeName(messageType) << " message arrived in REBINDING state. In this state, client does not expect messages of this type, dropping." << endl;
            }
            break;

        case REBOOTING:
            if (messageType == DHCPACK) {
                handleDHCPACK(msg);
                EV_INFO << "DHCPACK message arrived in REBOOTING state. Initialization with known IP address was successful." << endl;
                clientState = BOUND;
            }
            else if (messageType == DHCPNAK) {
                EV_INFO << "DHCPNAK message arrived in REBOOTING. Initialization with known IP address was unsuccessful." << endl;
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

void DHCPClient::receiveSignal(cComponent *source, int signalID, cObject *obj)
{
    Enter_Method_Silent();
    printNotificationBanner(signalID, obj);

    // host associated. link is up. change the state to init.
    if (signalID == NF_L2_ASSOCIATED) {
        InterfaceEntry *associatedIE = check_and_cast_nullable<InterfaceEntry *>(obj);
        if (associatedIE && ie == associatedIE) {
            EV_INFO << "Interface associated, starting DHCP." << endl;
            initClient();
        }
    }
    else if (signalID == NF_INTERFACE_DELETED) {
        if (isOperational)
            throw cRuntimeError("Reacting to interface deletions is not implemented in this module");
    }
}

void DHCPClient::sendRequest()
{
    // setting the xid
    xid = intuniform(0, RAND_MAX);    // generating a new xid for each transmission

    DHCPMessage *request = new DHCPMessage("DHCPREQUEST");
    request->setOp(BOOTREQUEST);
    request->setByteLength(280);    // DHCP request packet size
    request->setHtype(1);    // ethernet
    request->setHlen(6);    // hardware Address length (6 octets)
    request->setHops(0);
    request->setXid(xid);    // transaction id
    request->setSecs(0);    // 0 seconds from transaction started
    request->setBroadcast(false);    // unicast
    request->setYiaddr(IPv4Address());    // no 'your IP' addr
    request->setGiaddr(IPv4Address());    // no DHCP Gateway Agents
    request->setChaddr(macAddress);    // my mac address;
    request->setSname("");    // no server name given
    request->setFile("");    // no file given
    request->getOptions().setMessageType(DHCPREQUEST);
    request->getOptions().setClientIdentifier(macAddress);

    // set the parameters to request
    request->getOptions().setParameterRequestListArraySize(4);
    request->getOptions().setParameterRequestList(0, SUBNET_MASK);
    request->getOptions().setParameterRequestList(1, ROUTER);
    request->getOptions().setParameterRequestList(2, DNS);
    request->getOptions().setParameterRequestList(3, NTP_SRV);

    // RFC 4.3.6 Table 4
    if (clientState == INIT_REBOOT) {
        request->getOptions().setRequestedIp(lease->ip);
        request->setCiaddr(IPv4Address());    // zero
        EV_INFO << "Sending DHCPREQUEST asking for IP " << lease->ip << " via broadcast." << endl;
        sendToUDP(request, clientPort, IPv4Address::ALLONES_ADDRESS, serverPort);
    }
    else if (clientState == REQUESTING) {
        request->getOptions().setServerIdentifier(lease->serverId);
        request->getOptions().setRequestedIp(lease->ip);
        request->setCiaddr(IPv4Address());    // zero
        EV_INFO << "Sending DHCPREQUEST asking for IP " << lease->ip << " via broadcast." << endl;
        sendToUDP(request, clientPort, IPv4Address::ALLONES_ADDRESS, serverPort);
    }
    else if (clientState == RENEWING) {
        request->setCiaddr(lease->ip);    // the client IP
        EV_INFO << "Sending DHCPREQUEST extending lease for IP " << lease->ip << " via unicast to " << lease->serverId << "." << endl;
        sendToUDP(request, clientPort, lease->serverId, serverPort);
    }
    else if (clientState == REBINDING) {
        request->setCiaddr(lease->ip);    // the client IP
        EV_INFO << "Sending DHCPREQUEST renewing the IP " << lease->ip << " via broadcast." << endl;
        sendToUDP(request, clientPort, IPv4Address::ALLONES_ADDRESS, serverPort);
    }
    else
        throw cRuntimeError("Invalid state");
}

void DHCPClient::sendDiscover()
{
    // setting the xid
    xid = intuniform(0, RAND_MAX);
    //std::cout << xid << endl;
    DHCPMessage *discover = new DHCPMessage("DHCPDISCOVER");
    discover->setOp(BOOTREQUEST);
    discover->setByteLength(280);    // DHCP Discover packet size;
    discover->setHtype(1);    // ethernet
    discover->setHlen(6);    // hardware Address lenght (6 octets)
    discover->setHops(0);
    discover->setXid(xid);    // transaction id
    discover->setSecs(0);    // 0 seconds from transaction started
    discover->setBroadcast(false);    // unicast
    discover->setChaddr(macAddress);    // my mac address
    discover->setSname("");    // no server name given
    discover->setFile("");    // no file given
    discover->getOptions().setMessageType(DHCPDISCOVER);
    discover->getOptions().setClientIdentifier(macAddress);
    discover->getOptions().setRequestedIp(IPv4Address());

    // set the parameters to request
    discover->getOptions().setParameterRequestListArraySize(4);
    discover->getOptions().setParameterRequestList(0, SUBNET_MASK);
    discover->getOptions().setParameterRequestList(1, ROUTER);
    discover->getOptions().setParameterRequestList(2, DNS);
    discover->getOptions().setParameterRequestList(3, NTP_SRV);

    EV_INFO << "Sending DHCPDISCOVER." << endl;
    sendToUDP(discover, clientPort, IPv4Address::ALLONES_ADDRESS, serverPort);
}

void DHCPClient::sendDecline(IPv4Address declinedIp)
{
    xid = intuniform(0, RAND_MAX);
    DHCPMessage *decline = new DHCPMessage("DHCPDECLINE");
    decline->setOp(BOOTREQUEST);
    decline->setByteLength(280);    // DHCPDECLINE packet size
    decline->setHtype(1);    // ethernet
    decline->setHlen(6);    // hardware Address length (6 octets)
    decline->setHops(0);
    decline->setXid(xid);    // transaction id
    decline->setSecs(0);    // 0 seconds from transaction started
    decline->setBroadcast(false);    // unicast
    decline->setChaddr(macAddress);    // my MAC address
    decline->setSname("");    // no server name given
    decline->setFile("");    // no file given
    decline->getOptions().setMessageType(DHCPDECLINE);
    decline->getOptions().setRequestedIp(declinedIp);
    EV_INFO << "Sending DHCPDECLINE." << endl;
    sendToUDP(decline, clientPort, IPv4Address::ALLONES_ADDRESS, serverPort);
}

void DHCPClient::handleDHCPACK(DHCPMessage *msg)
{
    recordLease(msg);
    cancelEvent(timerTo);
    scheduleTimerT1();
    scheduleTimerT2();
    bindLease();
}

void DHCPClient::scheduleTimerTO(TimerType type)
{
    // cancel the previous timeout
    cancelEvent(timerTo);
    timerTo->setKind(type);
    scheduleAt(simTime() + responseTimeout, timerTo);
}

void DHCPClient::scheduleTimerT1()
{
    // cancel the previous T1
    cancelEvent(timerT1);
    scheduleAt(simTime() + (lease->renewalTime), timerT1);    // RFC 2131 4.4.5
}

void DHCPClient::scheduleTimerT2()
{
    // cancel the previous T2
    cancelEvent(timerT2);
    scheduleAt(simTime() + (lease->rebindTime), timerT2);    // RFC 2131 4.4.5
}

void DHCPClient::sendToUDP(cPacket *msg, int srcPort, const L3Address& destAddr, int destPort)
{
    EV_INFO << "Sending packet " << msg << endl;
    UDPSocket::SendOptions options;
    options.outInterfaceId = ie->getInterfaceId();
    socket.sendTo(msg, destAddr, destPort, &options);
}

void DHCPClient::openSocket()
{
    socket.setOutputGate(gate("udpOut"));
    socket.bind(clientPort);
    socket.setBroadcast(true);
    EV_INFO << "DHCP server bound to port " << serverPort << "." << endl;
}

void DHCPClient::startApp()
{
    simtime_t start = std::max(startTime, simTime());
    ie = chooseInterface();
    scheduleAt(start, startTimer);
}

void DHCPClient::stopApp()
{
    cancelEvent(timerT1);
    cancelEvent(timerT2);
    cancelEvent(timerTo);
    cancelEvent(leaseTimer);
    cancelEvent(startTimer);

    // TODO: Client should send DHCPRELEASE to the server. However, the correct operation
    // of DHCP does not depend on the transmission of DHCPRELEASE messages.
    // TODO: socket.close();
}

bool DHCPClient::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (stage == NodeStartOperation::STAGE_APPLICATION_LAYER) {
            startApp();
            isOperational = true;
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (stage == NodeShutdownOperation::STAGE_APPLICATION_LAYER) {
            stopApp();
            isOperational = false;
            ie = NULL;
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (stage == NodeCrashOperation::STAGE_CRASH) {
            stopApp();
            isOperational = false;
            ie = NULL;
        }
    }
    else
        throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

} // namespace inet


//
// Copyright (C) 2008 Juan-Carlos Maureira
// Copyright (C) INRIA
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
        initialRetransmitDelay = par("initialRetransmitDelay");
        maxRetransmitDelay = par("maxRetransmitDelay");
        maxRetransmitCount = par("maxRetransmitCount");
        minRenewRetransmitInterval = par("minRenewRetransmitInterval");
        const char *declineIpStr = par("declineOfferedIp");
        if (declineIpStr && *declineIpStr)
            declineOfferedIp.set(declineIpStr);
        currentRetransmitDelay = initialRetransmitDelay;
        retransmitCount = 0;

        WATCH(numSent);
        WATCH(numReceived);
        WATCH(clientState);
        WATCH(xid);
        WATCH(dhcpStartTime);
        WATCH_EXPR("clientStateName", getStateName(clientState));

        // DHCP UDP ports
        clientPort = 68; // client
        serverPort = 67; // server
        WATCH(clientPort);
        WATCH(serverPort);
        WATCH(macAddress);
        WATCH(currentRetransmitDelay);
        WATCH(retransmitCount);
        WATCH(t2AbsoluteTime);
        WATCH(leaseAbsoluteExpiry);
        WATCH(releaseInFlight);
        WATCH(declineOfferedIp);
        // get the routing table to update and subscribe it to the blackboard
        irt.reference(this, "routingTableModule", true);
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

NetworkInterface *DhcpClient::chooseInterface()
{
    IInterfaceTable *ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    const char *interfaceName = par("interface");
    NetworkInterface *ie = nullptr;

    if (strlen(interfaceName) > 0) {
        ie = ift->findInterfaceByName(interfaceName);
        if (ie == nullptr)
            throw cRuntimeError("Interface \"%s\" does not exist", interfaceName);
    }
    else {
        // there should be exactly one non-loopback interface that we want to configure
        for (int i = 0; i < ift->getNumInterfaces(); i++) {
            NetworkInterface *current = ift->getInterface(i);
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

} // namespace

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
    if (operationalState == State::STOPPING_OPERATION && !socket.isOpen()) {
        simtime_t extra = par("stopOperationExtraTime");
        // If we sent a DHCPRELEASE while stopping, hold the lifecycle off long
        // enough for the IPv4 layer to forward the queued packet — otherwise
        // the subsequent network-layer stop stage would flush it as a pending
        // ARP-waiting packet (see Ipv4::flush()).
        if (releaseInFlight && extra < SIMTIME_ZERO)
            extra = SimTime(1, SIMTIME_MS);
        startActiveOperationExtraTimeOrFinish(extra);
    }
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
        else { // we have no lease from the previous DHCP process
            clientState = INIT;
            initClient();
        }
    }
    else if (category == WAIT_OFFER) {
        if (retransmitCount < maxRetransmitCount) {
            retransmitCount++;
            currentRetransmitDelay = std::min(currentRetransmitDelay * 2, maxRetransmitDelay);
            EV_INFO << "No DHCPOFFER yet; retransmitting DHCPDISCOVER (attempt "
                    << retransmitCount << ", next wait ~" << currentRetransmitDelay << "s)." << endl;
            sendDiscover(true); // reuse xid
            scheduleRetransmit(WAIT_OFFER, currentRetransmitDelay);
        }
        else {
            EV_DETAIL << "Maximum DHCPDISCOVER retransmits reached; restarting DHCP process." << endl;
            initClient();
        }
    }
    else if (category == WAIT_ACK) {
        if ((clientState == REQUESTING || clientState == REBOOTING) && retransmitCount < maxRetransmitCount) {
            retransmitCount++;
            currentRetransmitDelay = std::min(currentRetransmitDelay * 2, maxRetransmitDelay);
            EV_INFO << "No DHCPACK yet; retransmitting DHCPREQUEST (attempt "
                    << retransmitCount << ", next wait ~" << currentRetransmitDelay << "s)." << endl;
            sendRequest(true); // reuse xid
            scheduleRetransmit(WAIT_ACK, currentRetransmitDelay);
        }
        else if (clientState == RENEWING) {
            // RFC 2131 §4.4.5: retransmit at half the remaining interval to T2,
            // floored at minRenewRetransmitInterval.
            simtime_t remaining = t2AbsoluteTime - simTime();
            if (remaining <= SIMTIME_ZERO) {
                // T2 will fire (or has just fired); let its handler take over.
                EV_DETAIL << "RENEWING: T2 reached, not retransmitting." << endl;
            }
            else {
                simtime_t nextDelay = std::max(remaining / 2, minRenewRetransmitInterval);
                EV_DETAIL << "RENEWING: no DHCPACK yet; retransmitting in ~" << nextDelay << "s." << endl;
                sendRequest(true);
                scheduleRetransmit(WAIT_ACK, nextDelay);
            }
        }
        else if (clientState == REBINDING) {
            simtime_t remaining = leaseAbsoluteExpiry - simTime();
            if (remaining <= SIMTIME_ZERO) {
                EV_DETAIL << "REBINDING: lease expired, not retransmitting." << endl;
            }
            else {
                simtime_t nextDelay = std::max(remaining / 2, minRenewRetransmitInterval);
                EV_DETAIL << "REBINDING: no DHCPACK yet; retransmitting in ~" << nextDelay << "s." << endl;
                sendRequest(true);
                scheduleRetransmit(WAIT_ACK, nextDelay);
            }
        }
        else {
            EV_DETAIL << "Maximum DHCPREQUEST retransmits reached; restarting DHCP process." << endl;
            initClient();
        }
    }
    else if (category == T1) {
        EV_DETAIL << "T1 expired. Starting RENEWING state." << endl;
        clientState = RENEWING;
        sendRequest();
        // §4.4.5 retransmit schedule for RENEWING — start with half-remaining-to-T2 (or min).
        simtime_t remaining = t2AbsoluteTime - simTime();
        simtime_t firstDelay = std::max(remaining / 2, minRenewRetransmitInterval);
        scheduleRetransmit(WAIT_ACK, firstDelay);
    }
    else if (category == T2 && clientState == RENEWING) {
        EV_DETAIL << "T2 expired. Starting REBINDING state." << endl;
        clientState = REBINDING;
        cancelEvent(timerT1);
        cancelEvent(timerT2);
        cancelEvent(timerTo);

        sendRequest();
        simtime_t remaining = leaseAbsoluteExpiry - simTime();
        simtime_t firstDelay = std::max(remaining / 2, minRenewRetransmitInterval);
        scheduleRetransmit(WAIT_ACK, firstDelay);
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

//        Byte serverIdB = dhcpOffer->getOptions().get(SERVER_ID);
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

        // RFC 2131, 4.4.5: apply defaults if server did not provide T1/T2
        if (lease->renewalTime == SIMTIME_ZERO)
            lease->renewalTime = lease->leaseTime * 0.5;
        if (lease->rebindTime == SIMTIME_ZERO)
            lease->rebindTime = lease->leaseTime * 0.875;
    }
    else
        EV_ERROR << "DHCPACK arrived, but no IP address confirmed." << endl;
}

void DhcpClient::bindLease()
{
    auto ipv4Data = ie->getProtocolDataForUpdate<Ipv4InterfaceData>();
    ipv4Data->setIPAddress(lease->ip);
    ipv4Data->setNetmask(lease->subnetMask);

    std::string banner = "Got IP " + lease->ip.str();
    host->bubble(banner.c_str());

    // RFC 2131 §2.2 / RFC 5227 recommend an ARP probe before adopting the
    // address, sending DHCPDECLINE on conflict. We expose that decision via
    // the declineOfferedIp parameter and handle it in the REQUESTING-state
    // branch before bindLease is reached; see triggerDeclineIfConfigured.

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
    rescheduleAfter(lease->leaseTime, leaseTimer);
    t2AbsoluteTime = simTime() + lease->rebindTime;
    leaseAbsoluteExpiry = simTime() + lease->leaseTime;
}

void DhcpClient::unbindLease()
{
    EV_INFO << "Unbinding lease on " << ie->getInterfaceName() << "." << endl;

    cancelEvent(timerT1);
    cancelEvent(timerT2);
    cancelEvent(timerTo);
    cancelEvent(leaseTimer);

    irt->deleteRoute(route);
    ie->getProtocolDataForUpdate<Ipv4InterfaceData>()->setIPAddress(Ipv4Address());
    ie->getProtocolDataForUpdate<Ipv4InterfaceData>()->setNetmask(Ipv4Address::ALLONES_ADDRESS);
}

void DhcpClient::initClient()
{
    EV_INFO << "Starting DHCP configuration process." << endl;

    cancelEvent(timerT1);
    cancelEvent(timerT2);
    cancelEvent(timerTo);
    cancelEvent(leaseTimer);

    dhcpStartTime = simTime();
    sendDiscover();
    scheduleTimerTO(WAIT_OFFER);
    clientState = SELECTING;
}

void DhcpClient::initRebootedClient()
{
    dhcpStartTime = simTime();
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
                sendRequest(); // we accept the first offer
            }
            else
                EV_WARN << "Client is in SELECTING and the arriving packet is not a DHCPOFFER, dropping." << endl;
            break;

        case REQUESTING:
            if (messageType == DHCPOFFER) {
                EV_WARN << "We don't accept DHCPOFFERs in REQUESTING state, dropping." << endl; // remains in REQUESTING
            }
            else if (messageType == DHCPACK) {
                EV_INFO << "DHCPACK message arrived in REQUESTING state. The requested IP address is available in the server's pool of addresses." << endl;
                recordLease(msg);
                if (triggerDeclineIfConfigured())
                    break; // restart triggered; do not bind
                cancelEvent(timerTo);
                scheduleTimerT1();
                scheduleTimerT2();
                bindLease();
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
            EV_DETAIL << "We are in BOUND, discard all DHCP messages." << endl; // remain in BOUND
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
    Enter_Method("%s", cComponent::getSignalName(signalID));
    printSignalBanner(signalID, obj, details);

    // host associated. link is up. change the state to init.
    if (signalID == l2AssociatedSignal) {
        NetworkInterface *associatedIE = check_and_cast_nullable<NetworkInterface *>(obj);
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

void DhcpClient::sendRequest(bool retransmit)
{
    // Per RFC 2131 §4.1, retransmissions of the same DHCPREQUEST must reuse
    // the original xid so the server can match the reply.
    if (!retransmit)
        xid = intuniform(0, RAND_MAX);

    const auto& request = makeShared<DhcpMessage>();
    request->setOp(BOOTREQUEST);
    uint16_t length = 236; // packet size without the options field
    request->setHtype(1); // ethernet
    request->setHlen(6); // hardware Address length (6 octets)
    request->setHops(0);
    request->setXid(xid); // transaction id
    request->setSecs((uint16_t)(simTime() - dhcpStartTime).dbl()); // seconds since DHCP process started
    // RFC 2131, 4.1: set BROADCAST if client cannot receive unicasts (no IP yet)
    request->setBroadcast(clientState == REQUESTING || clientState == REBOOTING || clientState == INIT_REBOOT);
    request->setYiaddr(Ipv4Address()); // no 'your IP' addr
    request->setGiaddr(Ipv4Address()); // no DHCP Gateway Agents
    request->setChaddr(macAddress); // my mac address;
    request->setSname(""); // no server name given
    request->setFile(""); // no file given
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
        request->setCiaddr(Ipv4Address()); // zero
        destAddr = Ipv4Address::ALLONES_ADDRESS;
        EV_INFO << "Sending DHCPREQUEST asking for IP " << lease->ip << " via broadcast." << endl;
    }
    else if (clientState == REQUESTING) {
        options.setServerIdentifier(lease->serverId);
        length += 6;
        options.setRequestedIp(lease->ip);
        length += 6;
        request->setCiaddr(Ipv4Address()); // zero
        destAddr = Ipv4Address::ALLONES_ADDRESS;
        EV_INFO << "Sending DHCPREQUEST asking for IP " << lease->ip << " via broadcast." << endl;
    }
    else if (clientState == RENEWING) {
        request->setCiaddr(lease->ip); // the client IP
        destAddr = lease->serverId;
        EV_INFO << "Sending DHCPREQUEST extending lease for IP " << lease->ip << " via unicast to " << lease->serverId << "." << endl;
    }
    else if (clientState == REBINDING) {
        request->setCiaddr(lease->ip); // the client IP
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

void DhcpClient::sendDiscover(bool retransmit)
{
    // Same xid across retransmits, RFC 2131 §4.1.
    if (!retransmit)
        xid = intuniform(0, RAND_MAX);
    Packet *packet = new Packet("DHCPDISCOVER");
    const auto& discover = makeShared<DhcpMessage>();
    discover->setOp(BOOTREQUEST);
    uint16_t length = 236; // packet size without the options field
    discover->setHtype(1); // ethernet
    discover->setHlen(6); // hardware Address lenght (6 octets)
    discover->setHops(0);
    discover->setXid(xid); // transaction id
    discover->setSecs((uint16_t)(simTime() - dhcpStartTime).dbl()); // seconds since DHCP process started
    discover->setBroadcast(true); // client cannot receive unicasts yet (no IP configured)
    discover->setChaddr(macAddress); // my mac address
    discover->setSname(""); // no server name given
    discover->setFile(""); // no file given
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
    uint16_t length = 236; // packet size without the options field
    decline->setHtype(1); // ethernet
    decline->setHlen(6); // hardware Address length (6 octets)
    decline->setHops(0);
    decline->setXid(xid); // transaction id
    decline->setSecs(0); // 0 seconds from transaction started
    decline->setBroadcast(false); // unicast
    decline->setChaddr(macAddress); // my MAC address
    decline->setSname(""); // no server name given
    decline->setFile(""); // no file given
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

bool DhcpClient::triggerDeclineIfConfigured()
{
    if (declineOfferedIp.isUnspecified() || lease == nullptr || lease->ip != declineOfferedIp)
        return false;
    EV_INFO << "Simulated conflict on offered IP " << lease->ip
            << "; sending DHCPDECLINE." << endl;
    Ipv4Address declined = lease->ip;
    sendDecline(declined);
    // Clear the one-shot trigger so the retried DORA can complete normally.
    declineOfferedIp = Ipv4Address();
    // Discard the rejected lease and restart from INIT.
    delete lease;
    lease = nullptr;
    cancelEvent(timerTo);
    initClient();
    return true;
}

void DhcpClient::sendRelease()
{
    // RFC 2131 §4.4.4: DHCPRELEASE is unicast to the server identifier with
    // ciaddr set to the IP being released. A fresh xid is used.
    xid = intuniform(0, RAND_MAX);
    Packet *packet = new Packet("DHCPRELEASE");
    const auto& release = makeShared<DhcpMessage>();
    release->setOp(BOOTREQUEST);
    uint16_t length = 236;
    release->setHtype(1);
    release->setHlen(6);
    release->setHops(0);
    release->setXid(xid);
    release->setSecs(0);
    release->setBroadcast(false);
    release->setCiaddr(lease->ip); // the IP being given up
    release->setChaddr(macAddress);
    release->setSname("");
    release->setFile("");
    auto& options = release->getOptionsForUpdate();
    options.setMessageType(DHCPRELEASE);
    length += 3;
    options.setClientIdentifier(macAddress);
    length += 9;
    options.setServerIdentifier(lease->serverId);
    length += 6;
    length += 5; // magic cookie + end
    release->setChunkLength(B(length));

    packet->insertAtBack(release);

    EV_INFO << "Sending DHCPRELEASE for " << lease->ip << " to server "
            << lease->serverId << "." << endl;
    sendToUdp(packet, clientPort, lease->serverId, serverPort);
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
    // Start of a fresh wait — reset the retransmit progression (RFC 2131 §4.1).
    resetRetransmitState();
    scheduleRetransmit(type, currentRetransmitDelay);
}

void DhcpClient::resetRetransmitState()
{
    retransmitCount = 0;
    currentRetransmitDelay = initialRetransmitDelay;
}

void DhcpClient::scheduleRetransmit(DhcpTimerType type, simtime_t delay)
{
    // RFC 2131 §4.1: each interval is randomized by ±1 second.
    simtime_t jitter = uniform(-1, 1);
    simtime_t actualDelay = delay + jitter;
    if (actualDelay < SIMTIME_ZERO)
        actualDelay = SIMTIME_ZERO;
    timerTo->setKind(type);
    rescheduleAfter(actualDelay, timerTo);
}

void DhcpClient::scheduleTimerT1()
{
    // cancel the previous T1
    rescheduleAfter(lease->renewalTime, timerT1); // RFC 2131 4.4.5
}

void DhcpClient::scheduleTimerT2()
{
    // cancel the previous T2
    rescheduleAfter(lease->rebindTime, timerT2); // RFC 2131 4.4.5
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
    EV_INFO << "DHCP client bound to port " << clientPort << "." << endl;
}

void DhcpClient::handleStartOperation(LifecycleOperation *operation)
{
    simtime_t start = std::max(startTime, simTime());
    ie = chooseInterface();
    macAddress = ie->getMacAddress();
    releaseInFlight = false;
    scheduleAt(start, startTimer);
}

void DhcpClient::handleStopOperation(LifecycleOperation *operation)
{
    // RFC 2131 §4.4.4: a client SHOULD send DHCPRELEASE if it relinquishes
    // its lease (e.g. graceful shutdown). Only meaningful when we currently
    // hold one; in INIT/SELECTING/REQUESTING/REBOOTING there is nothing to
    // release.
    if (lease != nullptr && (clientState == BOUND || clientState == RENEWING || clientState == REBINDING)) {
        sendRelease();
        releaseInFlight = true;
        // unbindLease() would clear the interface IP synchronously, but the
        // RELEASE packet is processed *after* this method returns. With the
        // IP cleared, the ARP request that the unicast RELEASE triggers would
        // assert. Drop the route now (it does not affect the outgoing packet)
        // and leave the interface IP for the lifecycle teardown that will
        // shortly take the interface itself down.
        if (route != nullptr) {
            irt->deleteRoute(route);
            route = nullptr;
        }
        // The lease is gone — a subsequent startup should do a fresh DORA,
        // not INIT-REBOOT a slot the server has already freed.
        delete lease;
        lease = nullptr;
        clientState = IDLE;
    }

    cancelEvent(timerT1);
    cancelEvent(timerT2);
    cancelEvent(timerTo);
    cancelEvent(leaseTimer);
    cancelEvent(startTimer);
    ie = nullptr;

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

    if (operation->getRootModule() != getContainingNode(this)) // closes socket when the application crashed only
        socket.destroy(); // TODO  in real operating systems, program crash detected by OS and OS closes sockets of crashed programs.
}

} // namespace inet


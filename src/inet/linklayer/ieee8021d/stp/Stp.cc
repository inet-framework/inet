//
// Copyright (C) 2013 OpenSim Ltd.
// Copyright (C) ANSA Team
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// Authors: ANSA Team
//

#include "inet/linklayer/ieee8021d/stp/Stp.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

Define_Module(Stp);

const double Stp::tickInterval = 1;

Stp::Stp()
{
}

void Stp::initialize(int stage)
{
    StpBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        tick = new cMessage("STP_TICK", 0);
        configuredBridgePriority = par("bridgePriority");
        holdTime = par("holdTime");
        configuredMaxAge = par("maxAge");
        configuredForwardDelay = par("forwardDelay");
        configuredHelloInterval = par("helloTime");

        WATCH(bridgeAddress);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        registerProtocol(Protocol::stp, gate("relayOut"), gate("relayIn"));

        for (int i = 0; i < ifTable->getNumInterfaces(); i++) {
            auto ie = ifTable->getInterface(i);
            if (!ie->isLoopback() && ie->isWired() && ie->isMulticast() /* && ie->getProtocol() == &Protocol::ethernetMac */) {   // TODO check protocol
                ie->addMulticastMacAddress(MacAddress::STP_MULTICAST_ADDRESS);
            }
        }
    }
}

Stp::~Stp()
{
    cancelAndDelete(tick);
}

void Stp::initPortTable()
{
    EV_DEBUG << "IEE8021D Interface Data initialization. Setting port infos to the protocol defaults." << endl;
    for (unsigned int i = 0; i < numPorts; i++) {
        auto ie = ifTable->getInterface(i);
        if (!ie->isLoopback() && ie->isWired() && ie->isMulticast() /* && ie->getProtocol() == &Protocol::ethernetMac */) {  // TODO check protocol
            initInterfacedata(ie->getInterfaceId());
        }
    }
}

void Stp::handleMessageWhenUp(cMessage *msg)
{
    if (!msg->isSelfMessage()) {
        Packet *packet = check_and_cast<Packet *>(msg);
        const auto& bpdu = packet->peekAtFront<BpduBase>();

        switch (bpdu->getBpduType()) {
            case BPDU_CFG:
                handleBpdu(packet, CHK(dynamicPtrCast<const BpduCfg>(bpdu)));
                break;
            case BPDU_TCN:
                handleTcn(packet, CHK(dynamicPtrCast<const BpduTcn>(bpdu)));
                break;
            default:
                throw cRuntimeError("unknown BPDU TYPE: %d", bpdu->getBpduType());
        }
    }
    else {
        if (msg == tick) {
            handleTick();
            scheduleAfter(1, tick);
        }
        else
            throw cRuntimeError("Unknown self-message received");
    }
}

void Stp::handleBpdu(Packet *packet, const Ptr<const BpduCfg>& bpdu)
{
    int arrivalGate = packet->getTag<InterfaceInd>()->getInterfaceId();
    const Ieee8021dInterfaceData *port = getPortInterfaceData(arrivalGate);

    if (bpdu->getTcaFlag()) {
        topologyChangeRecvd = true;
        topologyChangeNotification = false;
    }

    // get inferior BPDU, reply with superior
    if (!isSuperiorBpdu(arrivalGate, bpdu)) {
        if (port->getRole() == Ieee8021dInterfaceData::DESIGNATED) {
            EV_DETAIL << "Inferior Configuration BPDU " << bpdu << " arrived on port=" << arrivalGate << " responding to it with a superior BPDU." << endl;
            generateBpdu(arrivalGate);
        }
    }
    // BPDU from root
    else if (port->getRole() == Ieee8021dInterfaceData::ROOT) {
        EV_INFO << "Configuration BPDU " << bpdu << " arrived from Root Switch." << endl;

        if (bpdu->getTcFlag()) {
            EV_DEBUG << "MacForwardingTable aging time set to " << forwardDelay << "." << endl;
            macTable->setAgingTime(forwardDelay);

            // config BPDU with TC flag
            for (auto& elem : desPorts)
                generateBpdu(elem, MacAddress::STP_MULTICAST_ADDRESS, true, false);
        }
        else {
            macTable->setAgingTime(-1);

            EV_INFO << "Sending BPDUs on all designated ports." << endl;

            // BPDUs are sent on all designated ports
            for (auto& elem : desPorts)
                generateBpdu(elem);
        }
    }

    tryRoot();
    delete packet;
}

void Stp::handleTcn(Packet *packet, const Ptr<const BpduTcn>& tcn)
{
    EV_INFO << "Topology Change Notification BPDU " << tcn << " arrived." << endl;
    topologyChangeNotification = true;

    int arrivalGate = packet->getTag<InterfaceInd>()->getInterfaceId();
    const auto& addressInd = packet->getTag<MacAddressInd>();
    MacAddress srcAddress = addressInd->getSrcAddress();

    // send ACK to the sender
    EV_INFO << "Sending Topology Change Notification ACK." << endl;
    generateBpdu(arrivalGate, srcAddress, false, true);

    delete packet;
}

void Stp::generateBpdu(int interfaceId, const MacAddress& address, bool tcFlag, bool tcaFlag)
{
    auto portData = getPortInterfaceDataForUpdate(interfaceId);
    if (simTime() < portData->getEarliestBpduSendTime()) {
        portData->setBpduSendPending(true);
        return;
    }

    Packet *packet = new Packet("stp-hello");
    const auto& bpdu = makeShared<BpduCfg>();

    bpdu->setProtocolIdentifier(SPANNING_TREE_PROTOCOL);
    bpdu->setProtocolVersionIdentifier(SPANNING_TREE);

    bpdu->setBridgeAddress(bridgeAddress);
    bpdu->setBridgePriority(configuredBridgePriority);
    bpdu->setRootPathCost(rootPathCost);
    bpdu->setRootAddress(rootAddress);
    bpdu->setRootPriority(rootPriority);
    bpdu->setPortNum(interfaceId);
    bpdu->setPortPriority(getPortInterfaceData(interfaceId)->getPriority());
    bpdu->setMessageAge(0);
    bpdu->setMaxAge(maxAge);
    bpdu->setHelloTime(helloInterval);
    bpdu->setForwardDelay(forwardDelay);

    if (topologyChangeNotification) {
        if (isRoot || tcFlag) {
            bpdu->setTcFlag(true);
            bpdu->setTcaFlag(false);
            packet->setName("stp-tc");
        }
        else if (tcaFlag) {
            bpdu->setTcFlag(false);
            bpdu->setTcaFlag(true);
            packet->setName("stp-tca");
        }
    }

    packet->insertAtBack(bpdu);
    sendOut(packet, interfaceId, address);

    portData->setEarliestBpduSendTime(simTime() + holdTime);
    portData->setBpduSendPending(false);
}

void Stp::generateTcn()
{
    // there is something to notify
    if (topologyChangeNotification || !topologyChangeRecvd) {
        if (getPortInterfaceData(rootInterfaceId)->getRole() == Ieee8021dInterfaceData::ROOT) {
            // exist root port to notifying
            topologyChangeNotification = false;
            Packet *packet = new Packet("stp-tcn");
            const auto& tcn = makeShared<BpduTcn>();
            tcn->setProtocolIdentifier(SPANNING_TREE_PROTOCOL);
            tcn->setProtocolVersionIdentifier(SPANNING_TREE);
            packet->insertAtBack(tcn);

            EV_INFO << "The topology has changed. Sending Topology Change Notification BPDU " << tcn << " to the Root Switch." << endl;
            sendOut(packet, rootInterfaceId, MacAddress::STP_MULTICAST_ADDRESS);
        }
    }
}

bool Stp::isSuperiorBpdu(int interfaceId, const Ptr<const BpduCfg>& bpdu)
{
    auto port = getPortInterfaceDataForUpdate(interfaceId);
    Ieee8021dInterfaceData *xBpdu = new Ieee8021dInterfaceData();

    int result;

    xBpdu->setRootPriority(bpdu->getRootPriority());
    xBpdu->setRootAddress(bpdu->getRootAddress());
    xBpdu->setRootPathCost(bpdu->getRootPathCost() + port->getLinkCost());
    xBpdu->setBridgePriority(bpdu->getBridgePriority());
    xBpdu->setBridgeAddress(bpdu->getBridgeAddress());
    xBpdu->setPortPriority(bpdu->getPortPriority());
    xBpdu->setPortNum(bpdu->getPortNum());

    result = comparePorts(port, xBpdu);

    // port is superior
    if (result > 0) {
        delete xBpdu;
        return false;
    }

    if (result < 0) {
        // BPDU is superior
        port->setFdWhile(0); // renew info
        port->setState(Ieee8021dInterfaceData::DISCARDING);
        setSuperiorBpdu(interfaceId, bpdu); // renew information
        delete xBpdu;
        return true;
    }

    setSuperiorBpdu(interfaceId, bpdu); // renew information
    delete xBpdu;
    return true;
}

void Stp::setSuperiorBpdu(int interfaceId, const Ptr<const BpduCfg>& bpdu)
{
    // BPDU is out-of-date
    if (bpdu->getMessageAge() >= bpdu->getMaxAge())
        return;

    auto portData = getPortInterfaceDataForUpdate(interfaceId);

    portData->setRootPriority(bpdu->getRootPriority());
    portData->setRootAddress(bpdu->getRootAddress());
    portData->setRootPathCost(bpdu->getRootPathCost() + portData->getLinkCost());
    portData->setBridgePriority(bpdu->getBridgePriority());
    portData->setBridgeAddress(bpdu->getBridgeAddress());
    portData->setPortPriority(bpdu->getPortPriority());
    portData->setPortNum(bpdu->getPortNum());
    portData->setMaxAge(bpdu->getMaxAge());
    portData->setFwdDelay(bpdu->getForwardDelay());
    portData->setHelloTime(bpdu->getHelloTime());

    // we just set new port info so reset the age timer
    portData->setAge(0);
}

void Stp::generateHelloBpdus()
{
    EV_INFO << "It is hello time. Root switch sending hello BPDUs on all its ports." << endl;

    // send hello BPDUs on all ports
    for (unsigned int i = 0; i < numPorts; i++) {
        int interfaceId = ifTable->getInterface(i)->getInterfaceId();
        generateBpdu(interfaceId);
    }
}

void Stp::handleTick()
{
    // hello BPDU timer
    if (isRoot)
        timeSinceLastHello = timeSinceLastHello + 1;
    else
        timeSinceLastHello = 0;

    for (unsigned int i = 0; i < numPorts; i++) {
        int interfaceId = ifTable->getInterface(i)->getInterfaceId();
        auto port = getPortInterfaceDataForUpdate(interfaceId);

        // disabled ports don't count
        if (port->getRole() == Ieee8021dInterfaceData::DISABLED)
            continue;

        // increment the MessageAge and FdWhile timers
        if (port->getRole() != Ieee8021dInterfaceData::DESIGNATED) {
            EV_DEBUG << "Message Age timer incremented on port=" << interfaceId << endl;
            port->setAge(port->getAge() + tickInterval);
        }
        if (port->getRole() == Ieee8021dInterfaceData::ROOT || port->getRole() == Ieee8021dInterfaceData::DESIGNATED) {
            EV_DEBUG << "Forward While timer incremented on port=" << interfaceId << endl;
            port->setFdWhile(port->getFdWhile() + tickInterval);
        }
    }
    // send pending BPDUs that were suppressed by the hold timer
    for (unsigned int i = 0; i < numPorts; i++) {
        int interfaceId = ifTable->getInterface(i)->getInterfaceId();
        auto port = getPortInterfaceDataForUpdate(interfaceId);
        if (port->getBpduSendPending() && simTime() >= port->getEarliestBpduSendTime())
            generateBpdu(interfaceId);
    }

    checkTimers();
    checkParametersChange();
    generateTcn();
}

void Stp::checkTimers()
{
    Ieee8021dInterfaceData *port;

    // hello timer check
    if (timeSinceLastHello >= helloInterval) {
        // only the root switch can generate Hello BPDUs
        if (isRoot)
            generateHelloBpdus();

        timeSinceLastHello = 0;
    }

    // information age check
    for (unsigned int i = 0; i < numPorts; i++) {
        int interfaceId = ifTable->getInterface(i)->getInterfaceId();
        port = getPortInterfaceDataForUpdate(interfaceId);

        if (port->getAge() >= maxAge) {
            EV_DETAIL << "Port=" << i << " reached its maximum age. Setting it to the default port info." << endl;
            if (port->getRole() == Ieee8021dInterfaceData::ROOT) {
                initInterfacedata(interfaceId);
                lostRoot();
            }
            else {
                initInterfacedata(interfaceId);
                lostNonDesignated();
            }
        }
    }

    // fdWhile timer
    for (unsigned int i = 0; i < numPorts; i++) {
        int interfaceId = ifTable->getInterface(i)->getInterfaceId();
        port = getPortInterfaceDataForUpdate(interfaceId);

        // ROOT / DESIGNATED, can transition
        if (port->getRole() == Ieee8021dInterfaceData::ROOT || port->getRole() == Ieee8021dInterfaceData::DESIGNATED) {
            if (port->getFdWhile() >= forwardDelay) {
                switch (port->getState()) {
                    case Ieee8021dInterfaceData::DISCARDING:
                        EV_DETAIL << "Port=" << interfaceId << " goes into learning state." << endl;
                        port->setState(Ieee8021dInterfaceData::LEARNING);
                        port->setFdWhile(0);
                        break;

                    case Ieee8021dInterfaceData::LEARNING:
                        EV_DETAIL << "Port=" << interfaceId << " goes into forwarding state." << endl;
                        port->setState(Ieee8021dInterfaceData::FORWARDING);
                        port->setFdWhile(0);
                        break;

                    default:
                        port->setFdWhile(0);
                        break;
                }
            }
        }
        else {
            EV_DETAIL << "Port=" << interfaceId << " goes into discarding state." << endl;
            port->setFdWhile(0);
            port->setState(Ieee8021dInterfaceData::DISCARDING);
        }
    }
}

void Stp::checkParametersChange()
{
    if (isRoot) {
        helloInterval = configuredHelloInterval;
        maxAge = configuredMaxAge;
        forwardDelay = configuredForwardDelay;
    }
    if (bridgePriority != configuredBridgePriority) {
        bridgePriority = configuredBridgePriority;
        reset();
    }
}

bool Stp::checkRootEligibility()
{
    for (unsigned int i = 0; i < numPorts; i++) {
        int interfaceId = ifTable->getInterface(i)->getInterfaceId();
        const Ieee8021dInterfaceData *port = getPortInterfaceData(interfaceId);

        if (compareBridgeIDs(port->getRootPriority(), port->getRootAddress(), configuredBridgePriority, bridgeAddress) > 0)
            return false;
    }

    return true;
}

void Stp::tryRoot()
{
    if (checkRootEligibility()) {
        EV_DETAIL << "Switch is elected as root switch." << endl;
        isRoot = true;
        setAllDesignated();
        rootPriority = configuredBridgePriority;
        rootAddress = bridgeAddress;
        rootPathCost = 0;
        helloInterval = configuredHelloInterval;
        maxAge = configuredMaxAge;
        forwardDelay = configuredForwardDelay;
    }
    else {
        isRoot = false;
        selectRootPort();
        selectDesignatedPorts();
    }
}

int Stp::compareBridgeIDs(unsigned int aPriority, MacAddress aAddress, unsigned int bPriority, MacAddress bAddress)
{
    if (aPriority < bPriority)
        return 1; // A is superior
    else if (aPriority > bPriority)
        return -1;

    // APR == BPR
    if (aAddress.compareTo(bAddress) == -1)
        return 1; // A is superior
    else if (aAddress.compareTo(bAddress) == 1)
        return -1;

    // A==B
    // (can happen if bridge have two port connected to one not bridged lan,
    // "cable loopback"
    return 0;
}

int Stp::comparePortIDs(unsigned int aPriority, unsigned int aNum, unsigned int bPriority, unsigned int bNum)
{
    if (aPriority < bPriority)
        return 1; // A is superior

    else if (aPriority > bPriority)
        return -1;

    // APR == BPR
    if (aNum < bNum)
        return 1; // A is superior

    else if (aNum > bNum)
        return -1;

    // A==B
    return 0;
}

int Stp::comparePorts(const Ieee8021dInterfaceData *portA, const Ieee8021dInterfaceData *portB)
{
    int result;

    result = compareBridgeIDs(portA->getRootPriority(), portA->getRootAddress(), portB->getRootPriority(),
                portB->getRootAddress());

    // not same, so pass result
    if (result != 0)
        return result;

    if (portA->getRootPathCost() < portB->getRootPathCost())
        return 1;

    if (portA->getRootPathCost() > portB->getRootPathCost())
        return -1;

    // designated bridge
    result = compareBridgeIDs(portA->getBridgePriority(), portA->getBridgeAddress(), portB->getBridgePriority(),
                portB->getBridgeAddress());

    // not same, so pass result
    if (result != 0)
        return result;

    // designated port of designated Bridge
    result = comparePortIDs(portA->getPortPriority(), portA->getPortNum(), portB->getPortPriority(), portB->getPortNum());

    // not same, so pass result
    if (result != 0)
        return result;

    return 0; // same
}

void Stp::selectRootPort()
{
    desPorts.clear();
    unsigned int xRootIdx = 0;
    int result;
    auto best = ifTable->getInterface(0)->getProtocolData<Ieee8021dInterfaceData>();
    Ieee8021dInterfaceData *currentPort = nullptr;

    for (unsigned int i = 0; i < numPorts; i++) {
        currentPort = ifTable->getInterface(i)->getProtocolDataForUpdate<Ieee8021dInterfaceData>();
        currentPort->setRole(Ieee8021dInterfaceData::NOTASSIGNED);
        result = comparePorts(currentPort, best);
        if (result > 0) {
            xRootIdx = i;
            best = currentPort;
            continue;
        }
        if (result < 0) {
            continue;
        }
        if (currentPort->getPriority() < best->getPriority()) {
            xRootIdx = i;
            best = currentPort;
            continue;
        }
    }

    unsigned int xRootInterfaceId = ifTable->getInterface(xRootIdx)->getInterfaceId();
    if (rootInterfaceId != xRootInterfaceId) {
        EV_DETAIL << "Port=" << xRootInterfaceId << " selected as root port." << endl;
        topologyChangeNotification = true;
    }
    rootInterfaceId = xRootInterfaceId;
    getPortInterfaceDataForUpdate(rootInterfaceId)->setRole(Ieee8021dInterfaceData::ROOT);
    rootPathCost = best->getRootPathCost();
    rootAddress = best->getRootAddress();
    rootPriority = best->getRootPriority();

    maxAge = best->getMaxAge();
    forwardDelay = best->getFwdDelay();
    helloInterval = best->getHelloTime();
}

void Stp::selectDesignatedPorts()
{
    // select designated ports
    desPorts.clear();
    Ieee8021dInterfaceData *bridgeGlobal = new Ieee8021dInterfaceData();
    int result;

    bridgeGlobal->setBridgePriority(configuredBridgePriority);
    bridgeGlobal->setBridgeAddress(bridgeAddress);
    bridgeGlobal->setRootAddress(rootAddress);
    bridgeGlobal->setRootPriority(rootPriority);

    for (unsigned int i = 0; i < numPorts; i++) {
        NetworkInterface *ie = ifTable->getInterface(i);
        auto portData = ie->getProtocolDataForUpdate<Ieee8021dInterfaceData>();
        ASSERT(portData != nullptr);

        if (portData->getRole() == Ieee8021dInterfaceData::ROOT || portData->getRole() == Ieee8021dInterfaceData::DISABLED)
            continue;

        bridgeGlobal->setPortPriority(portData->getPriority());
        int interfaceId = ie->getInterfaceId();
        bridgeGlobal->setPortNum(interfaceId);

        bridgeGlobal->setRootPathCost(rootPathCost + portData->getLinkCost());

        result = comparePorts(bridgeGlobal, portData);

        if (result > 0) {
            EV_DETAIL << "Port=" << ie->getFullName() << " is elected as designated portData." << endl;
            desPorts.push_back(interfaceId);
            portData->setRole(Ieee8021dInterfaceData::DESIGNATED);
            continue;
        }
        if (result < 0) {
            EV_DETAIL << "Port=" << ie->getFullName() << " lost election, stays blocking." << endl;
            ASSERT(portData->getRole() != Ieee8021dInterfaceData::ALTERNATE);
            continue;
        }
    }
    delete bridgeGlobal;
}

void Stp::setAllDesignated()
{
    // all ports of the root switch are designated ports
    EV_DETAIL << "All ports become designated." << endl; // todo

    desPorts.clear();
    for (unsigned int i = 0; i < numPorts; i++) {
        NetworkInterface *ie = ifTable->getInterface(i);
        auto portData = ie->getProtocolDataForUpdate<Ieee8021dInterfaceData>();
        ASSERT(portData != nullptr);
        if (portData->getRole() == Ieee8021dInterfaceData::DISABLED)
            continue;

        int interfaceId = ie->getInterfaceId();
        portData->setRole(Ieee8021dInterfaceData::DESIGNATED);
        desPorts.push_back(interfaceId);
    }
}

void Stp::lostRoot()
{
    topologyChangeNotification = true;
    tryRoot();
}

void Stp::lostNonDesignated()
{
    selectDesignatedPorts();
    topologyChangeNotification = true;
}

void Stp::reset()
{
    // upon booting all switches believe themselves to be the root
    isRoot = true;
    rootPriority = configuredBridgePriority;
    rootAddress = bridgeAddress;
    rootPathCost = 0;
    helloInterval = configuredHelloInterval;
    maxAge = configuredMaxAge;
    forwardDelay = configuredForwardDelay;
    setAllDesignated();
}

void Stp::start()
{
    StpBase::start();

    initPortTable();
    bridgePriority = configuredBridgePriority;
    isRoot = true;
    topologyChangeNotification = true;
    topologyChangeRecvd = true;
    rootPriority = configuredBridgePriority;
    rootAddress = bridgeAddress;
    rootPathCost = 0;
    rootInterfaceId = ifTable->getInterface(0)->getInterfaceId();
    helloInterval = configuredHelloInterval;
    maxAge = configuredMaxAge;
    forwardDelay = configuredForwardDelay;
    timeSinceLastHello = 0;
    setAllDesignated();

    scheduleAfter(tickInterval, tick);
}

void Stp::stop()
{
    StpBase::stop();
    isRoot = false;
    topologyChangeNotification = true;
    cancelEvent(tick);
}

void Stp::initInterfacedata(unsigned int interfaceId)
{
    auto ifd = getPortInterfaceDataForUpdate(interfaceId);
    ifd->setRole(Ieee8021dInterfaceData::NOTASSIGNED);
    ifd->setState(Ieee8021dInterfaceData::DISCARDING);
    ifd->setRootPriority(configuredBridgePriority);
    ifd->setRootAddress(bridgeAddress);
    ifd->setRootPathCost(0);
    ifd->setAge(0);
    ifd->setBridgePriority(configuredBridgePriority);
    ifd->setBridgeAddress(bridgeAddress);
    ifd->setPortPriority(-1);
    ifd->setPortNum(-1);
    ifd->setLostBPDU(0);
}

} // namespace inet


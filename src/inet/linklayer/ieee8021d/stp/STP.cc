//
// Copyright (C) 2013 OpenSim Ltd.
// Copyright (C) ANSA Team
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
// Authors: ANSA Team, Benjamin Martin Seregi
//

#include "inet/linklayer/ethernet/EtherFrame.h"
#include "inet/linklayer/ieee8021d/stp/STP.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {

Define_Module(STP);

const double STP::tickInterval = 1;

STP::STP() :
    tick(NULL)
{
}

void STP::initialize(int stage)
{
    STPBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        tick = new cMessage("STP_TICK", 0);
        WATCH(bridgeAddress);
    }
    else if (stage == INITSTAGE_LINK_LAYER_2) {
        updateDisplay();
    }
}

STP::~STP()
{
    cancelAndDelete(tick);
}

void STP::initPortTable()
{
    EV_DEBUG << "IEE8021D Interface Data initialization. Setting port infos to the protocol defaults." << endl;
    for (unsigned int i = 0; i < numPorts; i++) {
        initInterfacedata(i);
    }
}

void STP::handleMessage(cMessage *msg)
{
    if (!isOperational) {
        EV << "Message '" << msg << "' arrived when module status is down, dropped it\n";
        delete msg;
        return;
    }

    cMessage *tmp = msg;

    if (!msg->isSelfMessage()) {
        if (dynamic_cast<BPDU *>(tmp)) {
            BPDU *bpdu = (BPDU *)tmp;

            if (bpdu->getBpduType() == CONFIG_BDPU)
                handleBPDU(bpdu);
            else if (bpdu->getBpduType() == TCN_BPDU)
                handleTCN(bpdu);
        }
        else
            throw cRuntimeError("Non-BPDU packet received");
    }
    else {
        if (msg == tick) {
            handleTick();
            updateDisplay();
            scheduleAt(simTime() + 1, tick);
        }
        else
            throw cRuntimeError("Unknown self-message received");
    }
}

void STP::handleBPDU(BPDU *bpdu)
{
    Ieee802Ctrl *controlInfo = check_and_cast<Ieee802Ctrl *>(bpdu->getControlInfo());
    int arrivalGate = controlInfo->getSwitchPort();
    Ieee8021dInterfaceData *port = getPortInterfaceData(arrivalGate);

    if (bpdu->getTcaFlag()) {
        topologyChangeRecvd = true;
        topologyChangeNotification = false;
    }

    // get inferior BPDU, reply with superior
    if (!isSuperiorBPDU(arrivalGate, bpdu)) {
        if (port->getRole() == Ieee8021dInterfaceData::DESIGNATED) {
            EV_DETAIL << "Inferior Configuration BPDU " << bpdu << " arrived on port=" << arrivalGate << " responding to it with a superior BPDU." << endl;
            generateBPDU(arrivalGate);
        }
    }
    // BPDU from root
    else if (port->getRole() == Ieee8021dInterfaceData::ROOT) {
        EV_INFO << "Configuration BPDU " << bpdu << " arrived from Root Switch." << endl;

        if (bpdu->getTcFlag()) {
            EV_DEBUG << "MACAddressTable aging time set to " << currentFwdDelay << "." << endl;
            macTable->setAgingTime(currentFwdDelay);

            // config BPDU with TC flag
            for (unsigned int i = 0; i < desPorts.size(); i++)
                generateBPDU(desPorts.at(i), MACAddress::STP_MULTICAST_ADDRESS, true, false);
        }
        else {
            macTable->resetDefaultAging();

            EV_INFO << "Sending BPDUs on all designated ports." << endl;

            // BPDUs are sent on all designated ports
            for (unsigned int i = 0; i < desPorts.size(); i++)
                generateBPDU(desPorts.at(i));
        }
    }

    tryRoot();
    delete bpdu;
}

void STP::handleTCN(BPDU *tcn)
{
    EV_INFO << "Topology Change Notification BDPU " << tcn << " arrived." << endl;
    topologyChangeNotification = true;

    Ieee802Ctrl *controlInfo = check_and_cast<Ieee802Ctrl *>(tcn->getControlInfo());
    int arrivalGate = controlInfo->getSwitchPort();
    MACAddress address = controlInfo->getSrc();

    // send ACK to the sender
    EV_INFO << "Sending Topology Change Notification ACK." << endl;
    generateBPDU(arrivalGate, address, false, true);

    controlInfo->setSwitchPort(rootPort);    // send TCN to the Root Switch

    if (!isRoot)
        send(tcn, "relayOut");
    else
        delete tcn;
}

void STP::generateBPDU(int port, const MACAddress& address, bool tcFlag, bool tcaFlag)
{
    BPDU *bpdu = new BPDU();
    Ieee802Ctrl *controlInfo = new Ieee802Ctrl();
    controlInfo->setDest(address);
    controlInfo->setSwitchPort(port);

    bpdu->setName("BPDU");
    bpdu->setProtocolIdentifier(0);
    bpdu->setProtocolVersionIdentifier(0);
    bpdu->setBpduType(0);    // 0 if configuration BPDU

    bpdu->setBridgeAddress(bridgeAddress);
    bpdu->setBridgePriority(bridgePriority);
    bpdu->setRootPathCost(rootPathCost);
    bpdu->setRootAddress(rootAddress);
    bpdu->setRootPriority(rootPriority);
    bpdu->setPortNum(port);
    bpdu->setPortPriority(getPortInterfaceData(port)->getPriority());
    bpdu->setMessageAge(0);
    bpdu->setMaxAge(currentMaxAge);
    bpdu->setHelloTime(currentHelloTime);
    bpdu->setForwardDelay(currentFwdDelay);

    if (topologyChangeNotification) {
        if (isRoot || tcFlag) {
            bpdu->setTcFlag(true);
            bpdu->setTcaFlag(false);
        }
        else if (tcaFlag) {
            bpdu->setTcFlag(false);
            bpdu->setTcaFlag(true);
        }
    }

    bpdu->setControlInfo(controlInfo);

    send(bpdu, "relayOut");
}

void STP::generateTCN()
{
    // there is something to notify
    if (topologyChangeNotification || !topologyChangeRecvd) {
        if (getPortInterfaceData(rootPort)->getRole() == Ieee8021dInterfaceData::ROOT) {
            // exist root port to notifying
            topologyChangeNotification = false;
            BPDU *tcn = new BPDU();
            tcn->setProtocolIdentifier(0);
            tcn->setProtocolVersionIdentifier(0);
            tcn->setName("BPDU");

            // 1 if Topology Change Notification BPDU
            tcn->setBpduType(1);

            Ieee802Ctrl *controlInfo = new Ieee802Ctrl();
            controlInfo->setDest(MACAddress::STP_MULTICAST_ADDRESS);
            controlInfo->setSwitchPort(rootPort);
            tcn->setControlInfo(controlInfo);

            EV_INFO << "The topology has changed. Sending Topology Change Notification BPDU " << tcn << " to the Root Switch." << endl;
            send(tcn, "relayOut");
        }
    }
}

bool STP::isSuperiorBPDU(int portNum, BPDU *bpdu)
{
    Ieee8021dInterfaceData *port = getPortInterfaceData(portNum);
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
        port->setFdWhile(0);    // renew info
        port->setState(Ieee8021dInterfaceData::DISCARDING);
        setSuperiorBPDU(portNum, bpdu);    // renew information
        delete xBpdu;
        return true;
    }

    setSuperiorBPDU(portNum, bpdu);    // renew information
    delete xBpdu;
    return true;
}

void STP::setSuperiorBPDU(int portNum, BPDU *bpdu)
{
    // BDPU is out-of-date
    if (bpdu->getMessageAge() >= bpdu->getMaxAge())
        return;

    Ieee8021dInterfaceData *portData = getPortInterfaceData(portNum);

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

void STP::generateHelloBDPUs()
{
    EV_INFO << "It is hello time. Root switch sending hello BDPUs on all its ports." << endl;

    // send hello BDPUs on all ports
    for (unsigned int i = 0; i < numPorts; i++)
        generateBPDU(i);
}

void STP::handleTick()
{
    // hello BDPU timer
    if (isRoot)
        helloTime = helloTime + 1;
    else
        helloTime = 0;

    for (unsigned int i = 0; i < numPorts; i++) {
        Ieee8021dInterfaceData *port = getPortInterfaceData(i);

        // disabled ports don't count
        if (port->getRole() == Ieee8021dInterfaceData::DISABLED)
            continue;

        // increment the MessageAge and FdWhile timers
        if (port->getRole() != Ieee8021dInterfaceData::DESIGNATED) {
            EV_DEBUG << "Message Age timer incremented on port=" << i << endl;
            port->setAge(port->getAge() + tickInterval);
        }
        if (port->getRole() == Ieee8021dInterfaceData::ROOT || port->getRole() == Ieee8021dInterfaceData::DESIGNATED) {
            EV_DEBUG << "Forward While timer incremented on port=" << i << endl;
            port->setFdWhile(port->getFdWhile() + tickInterval);
        }
    }
    checkTimers();
    checkParametersChange();
    generateTCN();
}

void STP::checkTimers()
{
    Ieee8021dInterfaceData *port;

    // hello timer check
    if (helloTime >= currentHelloTime) {
        // only the root switch can generate Hello BPDUs
        if (isRoot)
            generateHelloBDPUs();

        helloTime = 0;
    }

    // information age check
    for (unsigned int i = 0; i < numPorts; i++) {
        port = getPortInterfaceData(i);

        if (port->getAge() >= currentMaxAge) {
            EV_DETAIL << "Port=" << i << " reached its maximum age. Setting it to the default port info." << endl;
            if (port->getRole() == Ieee8021dInterfaceData::ROOT) {
                initInterfacedata(i);
                lostRoot();
            }
            else {
                initInterfacedata(i);
                lostAlternate(i);
            }
        }
    }

    // fdWhile timer
    for (unsigned int i = 0; i < numPorts; i++) {
        port = getPortInterfaceData(i);

        // ROOT / DESIGNATED, can transition
        if (port->getRole() == Ieee8021dInterfaceData::ROOT || port->getRole() == Ieee8021dInterfaceData::DESIGNATED) {
            if (port->getFdWhile() >= currentFwdDelay) {
                switch (port->getState()) {
                    case Ieee8021dInterfaceData::DISCARDING:
                        EV_DETAIL << "Port=" << i << " goes into learning state." << endl;
                        port->setState(Ieee8021dInterfaceData::LEARNING);
                        port->setFdWhile(0);
                        break;

                    case Ieee8021dInterfaceData::LEARNING:
                        EV_DETAIL << "Port=" << i << " goes into forwarding state." << endl;
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
            EV_DETAIL << "Port=" << i << " goes into discarding state." << endl;
            port->setFdWhile(0);
            port->setState(Ieee8021dInterfaceData::DISCARDING);
        }
    }
}

void STP::checkParametersChange()
{
    if (isRoot) {
        currentHelloTime = helloTime;
        currentMaxAge = maxAge;
        currentFwdDelay = forwardDelay;
    }
    if (currentBridgePriority != bridgePriority) {
        currentBridgePriority = bridgePriority;
        reset();
    }
}

bool STP::checkRootEligibility()
{
    Ieee8021dInterfaceData *port;

    for (unsigned int i = 0; i < numPorts; i++) {
        port = getPortInterfaceData(i);

        if (compareBridgeIDs(port->getRootPriority(), port->getRootAddress(), bridgePriority, bridgeAddress) > 0)
            return false;
    }

    return true;
}

void STP::tryRoot()
{
    if (checkRootEligibility()) {
        EV_DETAIL << "Switch is elected as root switch." << endl;
        isRoot = true;
        setAllDesignated();
        rootPriority = bridgePriority;
        rootAddress = bridgeAddress;
        rootPathCost = 0;
        currentHelloTime = helloTime;
        currentMaxAge = maxAge;
        currentFwdDelay = forwardDelay;
    }
    else {
        isRoot = false;
        selectRootPort();
        selectDesignatedPorts();
    }
}

int STP::compareBridgeIDs(unsigned int aPriority, MACAddress aAddress, unsigned int bPriority, MACAddress bAddress)
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

int STP::comparePortIDs(unsigned int aPriority, unsigned int aNum, unsigned int bPriority, unsigned int bNum)
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

int STP::comparePorts(Ieee8021dInterfaceData *portA, Ieee8021dInterfaceData *portB)
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

    return 0;    // same
}

void STP::selectRootPort()
{
    desPorts.clear();
    unsigned int xRootPort = 0;
    int result;
    Ieee8021dInterfaceData *best = getPortInterfaceData(0);
    Ieee8021dInterfaceData *currentPort;

    for (unsigned int i = 0; i < numPorts; i++) {
        currentPort = getPortInterfaceData(i);
        currentPort->setRole(Ieee8021dInterfaceData::NOTASSIGNED);
        result = comparePorts(currentPort, best);
        if (result > 0) {
            xRootPort = i;
            best = currentPort;
            continue;
        }
        if (result < 0) {
            continue;
        }
        if (currentPort->getPriority() < best->getPriority()) {
            xRootPort = i;
            best = currentPort;
            continue;
        }
        if (currentPort->getPriority() > best->getPriority())
            continue;
    }

    if (rootPort != xRootPort) {
        EV_DETAIL << "Port=" << xRootPort << " selected as root port." << endl;
        topologyChangeNotification = true;
    }
    rootPort = xRootPort;
    getPortInterfaceData(rootPort)->setRole(Ieee8021dInterfaceData::ROOT);
    rootPathCost = best->getRootPathCost();
    rootAddress = best->getRootAddress();
    rootPriority = best->getRootPriority();

    currentMaxAge = best->getMaxAge();
    currentFwdDelay = best->getFwdDelay();
    currentHelloTime = best->getHelloTime();
}

void STP::selectDesignatedPorts()
{
    // select designated ports
    desPorts.clear();
    Ieee8021dInterfaceData *port;
    Ieee8021dInterfaceData *bridgeGlobal = new Ieee8021dInterfaceData();
    int result;

    bridgeGlobal->setBridgePriority(bridgePriority);
    bridgeGlobal->setBridgeAddress(bridgeAddress);
    bridgeGlobal->setRootAddress(rootAddress);
    bridgeGlobal->setRootPriority(rootPriority);

    for (unsigned int i = 0; i < numPorts; i++) {
        port = getPortInterfaceData(i);

        if (port->getRole() == Ieee8021dInterfaceData::ROOT || port->getRole() == Ieee8021dInterfaceData::DISABLED)
            continue;

        bridgeGlobal->setPortPriority(port->getPriority());
        bridgeGlobal->setPortNum(i);

        bridgeGlobal->setRootPathCost(rootPathCost + port->getLinkCost());

        result = comparePorts(bridgeGlobal, port);

        if (result > 0) {
            EV_DETAIL << "Port=" << i << " is elected as designated port." << endl;
            desPorts.push_back(i);
            port->setRole(Ieee8021dInterfaceData::DESIGNATED);
            continue;
        }
        if (result < 0) {
            EV_DETAIL << "Port=" << i << " goes into alternate role." << endl;
            port->setRole(Ieee8021dInterfaceData::ALTERNATE);
            continue;
        }
    }
    delete bridgeGlobal;
}

void STP::setAllDesignated()
{
    // all ports of the root switch are designated ports
    EV_DETAIL << "All ports become designated." << endl;    // todo

    Ieee8021dInterfaceData *port;
    desPorts.clear();
    for (unsigned int i = 0; i < numPorts; i++) {
        port = getPortInterfaceData(i);
        if (port->getRole() == Ieee8021dInterfaceData::DISABLED)
            continue;

        port->setRole(Ieee8021dInterfaceData::DESIGNATED);
        desPorts.push_back(i);
    }
}

void STP::lostRoot()
{
    topologyChangeNotification = true;
    tryRoot();
}

void STP::lostAlternate(int port)
{
    selectDesignatedPorts();
    topologyChangeNotification = true;
}

void STP::reset()
{
    // upon booting all switches believe themselves to be the root
    isRoot = true;
    rootPriority = bridgePriority;
    rootAddress = bridgeAddress;
    rootPathCost = 0;
    currentHelloTime = helloTime;
    currentMaxAge = maxAge;
    currentFwdDelay = forwardDelay;
    setAllDesignated();
}

void STP::start()
{
    STPBase::start();

    initPortTable();
    currentBridgePriority = bridgePriority;
    isRoot = true;
    topologyChangeNotification = true;
    topologyChangeRecvd = true;
    rootPriority = bridgePriority;
    rootAddress = bridgeAddress;
    rootPathCost = 0;
    rootPort = 0;
    currentHelloTime = helloTime;
    currentMaxAge = maxAge;
    currentFwdDelay = forwardDelay;
    helloTime = 0;
    setAllDesignated();

    scheduleAt(simTime() + tickInterval, tick);
}

void STP::stop()
{
    STPBase::stop();
    isRoot = false;
    topologyChangeNotification = true;
    cancelEvent(tick);
}

void STP::initInterfacedata(unsigned int portNum)
{
    Ieee8021dInterfaceData *ifd = getPortInterfaceData(portNum);
    ifd->setRole(Ieee8021dInterfaceData::NOTASSIGNED);
    ifd->setState(Ieee8021dInterfaceData::DISCARDING);
    ifd->setRootPriority(bridgePriority);
    ifd->setRootAddress(bridgeAddress);
    ifd->setRootPathCost(0);
    ifd->setAge(0);
    ifd->setBridgePriority(bridgePriority);
    ifd->setBridgeAddress(bridgeAddress);
    ifd->setPortPriority(-1);
    ifd->setPortNum(-1);
    ifd->setLostBPDU(0);
}

} // namespace inet


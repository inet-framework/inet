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

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/ieee8021d/stp/Stp.h"
#include "inet/networklayer/common/InterfaceEntry.h"

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
        WATCH(bridgeAddress);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        registerService(Protocol::stp, nullptr, gate("relayIn"));
        registerProtocol(Protocol::stp, gate("relayOut"), nullptr);
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
        initInterfacedata(ifTable->getInterface(i)->getInterfaceId());
    }
}

void Stp::handleMessageWhenUp(cMessage *msg)
{
    if (!msg->isSelfMessage()) {
        Packet *packet = check_and_cast<Packet*>(msg);
        const auto& bpdu = packet->peekAtFront<BpduBase>();

        switch (bpdu->getBpduType()) {
            case BPDU_CFG:
                handleBPDU(packet, CHK(dynamicPtrCast<const BpduCfg>(bpdu)));
                break;
            case BPDU_TCN:
                handleTCN(packet, CHK(dynamicPtrCast<const BpduTcn>(bpdu)));
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

void Stp::handleBPDU(Packet *packet, const Ptr<const BpduCfg>& bpdu)
{
    int arrivalGate = packet->getTag<InterfaceInd>()->getInterfaceId();
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
            EV_DEBUG << "MacAddressTable aging time set to " << currentFwdDelay << "." << endl;
            macTable->setAgingTime(currentFwdDelay);

            // config BPDU with TC flag
            for (auto & elem : desPorts)
                generateBPDU(elem, MacAddress::STP_MULTICAST_ADDRESS, true, false);
        }
        else {
            macTable->resetDefaultAging();

            EV_INFO << "Sending BPDUs on all designated ports." << endl;

            // BPDUs are sent on all designated ports
            for (auto & elem : desPorts)
                generateBPDU(elem);
        }
    }

    tryRoot();
    delete packet;
}

void Stp::handleTCN(Packet *packet, const Ptr<const BpduTcn>& tcn)
{
    EV_INFO << "Topology Change Notification BPDU " << tcn << " arrived." << endl;
    topologyChangeNotification = true;

    int arrivalGate = packet->getTag<InterfaceInd>()->getInterfaceId();
    MacAddressInd *addressInd = packet->getTag<MacAddressInd>();
    MacAddress srcAddress = addressInd->getSrcAddress();
    MacAddress destAddress = addressInd->getDestAddress();

    // send ACK to the sender
    EV_INFO << "Sending Topology Change Notification ACK." << endl;
    generateBPDU(arrivalGate, srcAddress, false, true);

    if (!isRoot) {
        Packet *outPacket = new Packet(packet->getName());
        outPacket->insertAtBack(tcn);
        outPacket->addTag<InterfaceReq>()->setInterfaceId(rootInterfaceId);
        auto macAddressReq = outPacket->addTag<MacAddressReq>();
        macAddressReq->setSrcAddress(bridgeAddress);
        macAddressReq->setDestAddress(destAddress);
        outPacket->addTag<PacketProtocolTag>()->setProtocol(&Protocol::stp);
        outPacket->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ethernetMac);
        send(outPacket, "relayOut");
    }
    delete packet;
}

void Stp::generateBPDU(int interfaceId, const MacAddress& address, bool tcFlag, bool tcaFlag)
{
    Packet *packet = new Packet("BPDU");
    const auto& bpdu = makeShared<BpduCfg>();
    auto macAddressReq = packet->addTag<MacAddressReq>();
    macAddressReq->setSrcAddress(bridgeAddress);
    macAddressReq->setDestAddress(address);
    packet->addTag<InterfaceReq>()->setInterfaceId(interfaceId);
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::stp);
    packet->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ethernetMac);

    bpdu->setProtocolIdentifier(SPANNING_TREE_PROTOCOL);
    bpdu->setProtocolVersionIdentifier(SPANNING_TREE);

    bpdu->setBridgeAddress(bridgeAddress);
    bpdu->setBridgePriority(bridgePriority);
    bpdu->setRootPathCost(rootPathCost);
    bpdu->setRootAddress(rootAddress);
    bpdu->setRootPriority(rootPriority);
    bpdu->setPortNum(interfaceId);
    bpdu->setPortPriority(getPortInterfaceData(interfaceId)->getPriority());
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

    packet->insertAtBack(bpdu);
    send(packet, "relayOut");
}

void Stp::generateTCN()
{
    // there is something to notify
    if (topologyChangeNotification || !topologyChangeRecvd) {
        if (getPortInterfaceData(rootInterfaceId)->getRole() == Ieee8021dInterfaceData::ROOT) {
            // exist root port to notifying
            topologyChangeNotification = false;
            Packet *packet = new Packet("BPDU-TCN");
            const auto& tcn = makeShared<BpduTcn>();
            tcn->setProtocolIdentifier(SPANNING_TREE_PROTOCOL);
            tcn->setProtocolVersionIdentifier(SPANNING_TREE);

            auto macAddressReq = packet->addTag<MacAddressReq>();
            macAddressReq->setSrcAddress(bridgeAddress);
            macAddressReq->setDestAddress(MacAddress::STP_MULTICAST_ADDRESS);
            packet->addTag<InterfaceReq>()->setInterfaceId(rootInterfaceId);
            packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::stp);
            packet->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ethernetMac);

            packet->insertAtBack(tcn);
            EV_INFO << "The topology has changed. Sending Topology Change Notification BPDU " << tcn << " to the Root Switch." << endl;
            send(packet, "relayOut");
        }
    }
}

bool Stp::isSuperiorBPDU(int interfaceId, const Ptr<const BpduCfg>& bpdu)
{
    Ieee8021dInterfaceData *port = getPortInterfaceData(interfaceId);
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
        setSuperiorBPDU(interfaceId, bpdu);    // renew information
        delete xBpdu;
        return true;
    }

    setSuperiorBPDU(interfaceId, bpdu);    // renew information
    delete xBpdu;
    return true;
}

void Stp::setSuperiorBPDU(int interfaceId, const Ptr<const BpduCfg>& bpdu)
{
    // BPDU is out-of-date
    if (bpdu->getMessageAge() >= bpdu->getMaxAge())
        return;

    Ieee8021dInterfaceData *portData = getPortInterfaceData(interfaceId);

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

void Stp::generateHelloBPDUs()
{
    EV_INFO << "It is hello time. Root switch sending hello BPDUs on all its ports." << endl;

    // send hello BPDUs on all ports
    for (unsigned int i = 0; i < numPorts; i++) {
        int interfaceId = ifTable->getInterface(i)->getInterfaceId();
        generateBPDU(interfaceId);
    }
}

void Stp::handleTick()
{
    // hello BPDU timer
    if (isRoot)
        helloTime = helloTime + 1;
    else
        helloTime = 0;

    for (unsigned int i = 0; i < numPorts; i++) {
        int interfaceId = ifTable->getInterface(i)->getInterfaceId();
        Ieee8021dInterfaceData *port = getPortInterfaceData(interfaceId);

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
    checkTimers();
    checkParametersChange();
    generateTCN();
}

void Stp::checkTimers()
{
    Ieee8021dInterfaceData *port;

    // hello timer check
    if (helloTime >= currentHelloTime) {
        // only the root switch can generate Hello BPDUs
        if (isRoot)
            generateHelloBPDUs();

        helloTime = 0;
    }

    // information age check
    for (unsigned int i = 0; i < numPorts; i++) {
        int interfaceId = ifTable->getInterface(i)->getInterfaceId();
        port = getPortInterfaceData(interfaceId);

        if (port->getAge() >= currentMaxAge) {
            EV_DETAIL << "Port=" << i << " reached its maximum age. Setting it to the default port info." << endl;
            if (port->getRole() == Ieee8021dInterfaceData::ROOT) {
                initInterfacedata(interfaceId);
                lostRoot();
            }
            else {
                initInterfacedata(interfaceId);
                lostAlternate();
            }
        }
    }

    // fdWhile timer
    for (unsigned int i = 0; i < numPorts; i++) {
        int interfaceId = ifTable->getInterface(i)->getInterfaceId();
        port = getPortInterfaceData(interfaceId);

        // ROOT / DESIGNATED, can transition
        if (port->getRole() == Ieee8021dInterfaceData::ROOT || port->getRole() == Ieee8021dInterfaceData::DESIGNATED) {
            if (port->getFdWhile() >= currentFwdDelay) {
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
        currentHelloTime = helloTime;
        currentMaxAge = maxAge;
        currentFwdDelay = forwardDelay;
    }
    if (currentBridgePriority != bridgePriority) {
        currentBridgePriority = bridgePriority;
        reset();
    }
}

bool Stp::checkRootEligibility()
{
    for (unsigned int i = 0; i < numPorts; i++) {
        int interfaceId = ifTable->getInterface(i)->getInterfaceId();
        Ieee8021dInterfaceData *port = getPortInterfaceData(interfaceId);

        if (compareBridgeIDs(port->getRootPriority(), port->getRootAddress(), bridgePriority, bridgeAddress) > 0)
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

int Stp::comparePorts(Ieee8021dInterfaceData *portA, Ieee8021dInterfaceData *portB)
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

void Stp::selectRootPort()
{
    desPorts.clear();
    unsigned int xRootIdx = 0;
    int result;
    Ieee8021dInterfaceData *best = ifTable->getInterface(0)->getProtocolData<Ieee8021dInterfaceData>();
    Ieee8021dInterfaceData *currentPort = nullptr;

    for (unsigned int i = 0; i < numPorts; i++) {
        currentPort = ifTable->getInterface(i)->getProtocolData<Ieee8021dInterfaceData>();
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
    getPortInterfaceData(rootInterfaceId)->setRole(Ieee8021dInterfaceData::ROOT);
    rootPathCost = best->getRootPathCost();
    rootAddress = best->getRootAddress();
    rootPriority = best->getRootPriority();

    currentMaxAge = best->getMaxAge();
    currentFwdDelay = best->getFwdDelay();
    currentHelloTime = best->getHelloTime();
}

void Stp::selectDesignatedPorts()
{
    // select designated ports
    desPorts.clear();
    Ieee8021dInterfaceData *bridgeGlobal = new Ieee8021dInterfaceData();
    int result;

    bridgeGlobal->setBridgePriority(bridgePriority);
    bridgeGlobal->setBridgeAddress(bridgeAddress);
    bridgeGlobal->setRootAddress(rootAddress);
    bridgeGlobal->setRootPriority(rootPriority);

    for (unsigned int i = 0; i < numPorts; i++) {
        InterfaceEntry *ie = ifTable->getInterface(i);
        Ieee8021dInterfaceData *portData = ie->getProtocolData<Ieee8021dInterfaceData>();
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
            EV_DETAIL << "Port=" << ie->getFullName() << " goes into alternate role." << endl;
            portData->setRole(Ieee8021dInterfaceData::ALTERNATE);
            continue;
        }
    }
    delete bridgeGlobal;
}

void Stp::setAllDesignated()
{
    // all ports of the root switch are designated ports
    EV_DETAIL << "All ports become designated." << endl;    // todo

    desPorts.clear();
    for (unsigned int i = 0; i < numPorts; i++) {
        InterfaceEntry *ie = ifTable->getInterface(i);
        Ieee8021dInterfaceData *portData = ie->getProtocolData<Ieee8021dInterfaceData>();
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

void Stp::lostAlternate()
{
    selectDesignatedPorts();
    topologyChangeNotification = true;
}

void Stp::reset()
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

void Stp::start()
{
    StpBase::start();

    initPortTable();
    currentBridgePriority = bridgePriority;
    isRoot = true;
    topologyChangeNotification = true;
    topologyChangeRecvd = true;
    rootPriority = bridgePriority;
    rootAddress = bridgeAddress;
    rootPathCost = 0;
    rootInterfaceId = ifTable->getInterface(0)->getInterfaceId();
    currentHelloTime = helloTime;
    currentMaxAge = maxAge;
    currentFwdDelay = forwardDelay;
    helloTime = 0;
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
    Ieee8021dInterfaceData *ifd = getPortInterfaceData(interfaceId);
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


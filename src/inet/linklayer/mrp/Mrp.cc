// Copyright (C) 2024 Daniel Zeitler
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Mrp.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/Simsignals.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/linklayer/common/Ieee802SapTag_m.h"
#include "inet/linklayer/configurator/MrpInterfaceData.h"
#include "inet/linklayer/ethernet/common/Ethernet.h"
#include "inet/linklayer/ethernet/contract/IMacForwardingTable.h"
#include "inet/linklayer/mrp/MrpPdu_m.h"
#include "inet/linklayer/mrp/ContinuityCheckMessage_m.h"
#include "inet/linklayer/mrp/MrpRelay.h"
#include "inet/linklayer/mrp/Timers_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/common/InterfaceTable.h"

namespace inet {

static const char *ENABLED_LINK_COLOR = "#000000";
static const char *DISABLED_LINK_COLOR = "#bbbbbb";

Define_Module(Mrp);

Mrp::Mrp() {
}

Mrp::~Mrp() {
    cancelAndDelete(linkDownTimer);
    cancelAndDelete(linkUpTimer);
    cancelAndDelete(fdbClearTimer);
    cancelAndDelete(fdbClearDelay);
    cancelAndDelete(topologyChangeTimer);
    cancelAndDelete(testTimer);
    cancelAndDelete(startUpTimer);
    cancelAndDelete(linkUpHysteresisTimer);
}

void Mrp::setRingInterfaces(int InterfaceIndex1, int InterfaceIndex2) {
    ringInterface1 = interfaceTable->getInterface(InterfaceIndex1);
    if (ringInterface1->isLoopback()) {
        ringInterface1 = nullptr;
        EV_DEBUG << "Chosen Interface is Loopback-Interface" << EV_ENDL;
    } else
        primaryRingPort = ringInterface1->getInterfaceId();
    ringInterface2 = interfaceTable->getInterface(InterfaceIndex2);
    if (ringInterface2->isLoopback()) {
        ringInterface2 = nullptr;
        EV_DEBUG << "Chosen Interface is Loopback-Interface" << EV_ENDL;
    } else
        secondaryRingPort = ringInterface2->getInterfaceId();
}

void Mrp::setRingInterface(int InterfaceNumber, int InterfaceIndex) {
    if (InterfaceNumber == 1) {
        ringInterface1 = interfaceTable->getInterface(InterfaceIndex);
        if (ringInterface1->isLoopback()) {
            ringInterface1 = nullptr;
            EV_DEBUG << "Chosen Interface is Loopback-Interface" << EV_ENDL;
        } else
            primaryRingPort = ringInterface1->getInterfaceId();
    } else if (InterfaceNumber == 2) {
        ringInterface2 = interfaceTable->getInterface(InterfaceIndex);
        if (ringInterface2->isLoopback()) {
            ringInterface2 = nullptr;
            EV_DEBUG << "Chosen Interface is Loopback-Interface" << EV_ENDL;
        } else
            secondaryRingPort = ringInterface2->getInterfaceId();
    } else
        EV_DEBUG << "only 2 MRP Ring-Interfaces per Node allowed" << EV_ENDL;
}

void Mrp::setPortState(int InterfaceId, MrpInterfaceData::PortState State) {
    auto portData = getPortInterfaceDataForUpdate(InterfaceId);
    portData->setState(State);
    emit(portStateChangedSignal, portData->getState());
    EV_INFO << "Setting Port State" << EV_FIELD(InterfaceId) << EV_FIELD(State) << EV_ENDL;
}

void Mrp::setPortRole(int InterfaceId, MrpInterfaceData::PortRole Role) {
    auto portData = getPortInterfaceDataForUpdate(InterfaceId);
    portData->setRole(Role);
}

MrpInterfaceData::PortState Mrp::getPortState(int InterfaceId) {
    auto portData = getPortInterfaceDataForUpdate(InterfaceId);
    return portData->getState();
}

const MrpInterfaceData* Mrp::getPortInterfaceData(unsigned int interfaceId) const {
    return getPortNetworkInterface(interfaceId)->getProtocolData<MrpInterfaceData>();
}

MrpInterfaceData* Mrp::getPortInterfaceDataForUpdate(unsigned int interfaceId) {
    return getPortNetworkInterface(interfaceId)->getProtocolDataForUpdate<MrpInterfaceData>();
}

NetworkInterface* Mrp::getPortNetworkInterface(unsigned int interfaceId) const {
    NetworkInterface *gateIfEntry = interfaceTable->getInterfaceById(interfaceId);
    if (!gateIfEntry)
        throw cRuntimeError("gate's Interface is nullptr");
    return gateIfEntry;
}

MrpInterfaceData::PortRole Mrp::getPortRole(int InterfaceId) {
    auto portData = getPortInterfaceDataForUpdate(InterfaceId);
    return portData->getRole();
}

void Mrp::toggleRingPorts() {
    int RingPort = secondaryRingPort;
    secondaryRingPort = primaryRingPort;
    primaryRingPort = RingPort;
    setPortRole(primaryRingPort, MrpInterfaceData::PRIMARY);
    setPortRole(secondaryRingPort, MrpInterfaceData::SECONDARY);
}

void Mrp::initialize(int stage) {
    OperationalBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        //modules
        mrpMacForwardingTable.reference(this, "macTableModule", true);
        interfaceTable.reference(this, "interfaceTableModule", true);
        switchModule = getContainingNode(this);
        relay.reference(this, "mrpRelayModule", true);

        //parameters
        visualize = par("visualize");
        expectedRole = static_cast<MrpRole>(par("mrpRole").intValue());
        //currently only inferfaceIndex
        primaryRingPort = par("ringPort1");
        secondaryRingPort = par("ringPort2");
        domainID.uuid0 = par("uuid0");
        domainID.uuid1 = par("uuid1");
        timingProfile = par("timingProfile");
        ccmInterval = par("ccmInterval");
        interconnectionLinkCheckAware = par("interconnectionLinkCheckAware");
        interconnectionRingCheckAware = par("interconnectionRingCheckAware");
        enableLinkCheckOnRing = par("enableLinkCheckOnRing");
        //manager variables
        localManagerPrio = static_cast<MrpPriority>(par("mrpPriority").intValue());
        nonBlockingMRC = par("nonBlockingMRC");
        reactOnLinkChange = par("reactOnLinkChange");
        checkMediaRedundancy = par("checkMediaRedundancy");
        noTopologyChange = par("noTopologyChange");
        //client variables
        blockedStateSupported = par("blockedStateSupported");

        //signals
        linkChangeSignal = registerSignal("linkChange");
        topologyChangeSignal = registerSignal("topologyChange");
        testSignal = registerSignal("test");
        continuityCheckSignal = registerSignal("continuityCheck");
        receivedChangeSignal = registerSignal("receivedChange");
        receivedTestSignal = registerSignal("receivedTest");
        receivedContinuityCheckSignal = registerSignal("receivedContinuityCheck");
        ringStateChangedSignal = registerSignal("ringStateChanged");
        portStateChangedSignal = registerSignal("portStateChanged");
        clearFDBSignal = registerSignal("clearFDB");
        switchModule->subscribe(interfaceStateChangedSignal, this);

    }
    if (stage == INITSTAGE_LINK_LAYER) { // "auto" MAC addresses assignment takes place in stage 0
        registerProtocol(Protocol::mrp, gate("relayOut"), gate("relayIn"), nullptr, nullptr);
        registerProtocol(Protocol::ieee8021qCFM, gate("relayOut"), gate("relayIn"), nullptr, nullptr);
        initPortTable();
        //set interface and change Port-Indexes to Port-IDs
        setRingInterfaces(primaryRingPort, secondaryRingPort);
        localBridgeAddress = relay->getBridgeAddress();
        initRingPorts();
        EV_DETAIL << "Initialize MRP link layer" << EV_ENDL;
        linkUpHysteresisTimer = new cMessage("linkUpHysteresisTimer");
        startUpTimer = new cMessage("startUpTimer");
        scheduleAt(SimTime(0, SIMTIME_MS), startUpTimer);
    }
}

void Mrp::initPortTable() {
    EV_DEBUG << "MRP Interface Data initialization. Setting port infos to the protocol defaults." << EV_ENDL;
    for (unsigned int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
        auto ie = interfaceTable->getInterface(i);
        if (!ie->isLoopback() && ie->isWired() && ie->isMulticast()
                && ie->getProtocol() == &Protocol::ethernetMac) {
            initInterfacedata(ie->getInterfaceId());
        }
    }
}

void Mrp::initInterfacedata(unsigned int interfaceId) {
    auto ifd = getPortInterfaceDataForUpdate(interfaceId);
    ifd->setRole(MrpInterfaceData::NOTASSIGNED);
    ifd->setState(MrpInterfaceData::FORWARDING);
    ifd->setLostPDU(0);
    ifd->setContinuityCheckInterval(SimTime(ccmInterval, SIMTIME_MS));
    ifd->setContinuityCheck(false);
    ifd->setNextUpdate(SimTime(ccmInterval * 3.5, SIMTIME_MS));
}

void Mrp::initRingPorts() {
    if (ringInterface1 == nullptr)
        setRingInterface(1, primaryRingPort);
    if (ringInterface2 == nullptr)
        setRingInterface(2, secondaryRingPort);
    auto ifd = getPortInterfaceDataForUpdate(ringInterface1->getInterfaceId());
    ifd->setRole(MrpInterfaceData::PRIMARY);
    ifd->setState(MrpInterfaceData::BLOCKED);
    ifd->setContinuityCheck(enableLinkCheckOnRing);
    ifd->setContinuityCheckInterval(SimTime(ccmInterval, SIMTIME_MS));

    ifd = getPortInterfaceDataForUpdate(ringInterface2->getInterfaceId());
    ifd->setRole(MrpInterfaceData::SECONDARY);
    ifd->setState(MrpInterfaceData::BLOCKED);
    ifd->setContinuityCheck(enableLinkCheckOnRing);
    ifd->setContinuityCheckInterval(SimTime(ccmInterval, SIMTIME_MS));
}

void Mrp::startContinuityCheck() {
    for (unsigned int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
        auto ie = interfaceTable->getInterface(i);
        if (!ie->isLoopback() && ie->isWired() && ie->isMulticast()
                && ie->getProtocol() == &Protocol::ethernetMac) {
            int interfaceId = ie->getInterfaceId();
            auto portData = getPortInterfaceDataForUpdate(interfaceId);
            if (portData->getContinuityCheck()) {
                auto interface = getPortNetworkInterface(interfaceId);
                MacAddress address = interface->getMacAddress();
                std::string namePort = "CFM CCM " + address.str();
                portData->setCfmName(namePort);
                EV_DETAIL << "CFM-name port set:" << EV_FIELD(portData->getCfmName()) << EV_ENDL;
                setupContinuityCheck(interfaceId);
                class ContinuityCheckTimer *checkTimer = new ContinuityCheckTimer("continuityCheckTimer");
                checkTimer->setPort(interfaceId);
                checkTimer->setKind(1);
                scheduleAt(simTime() + portData->getContinuityCheckInterval(), checkTimer);
                EV_DEBUG << "Next CCM-Interval:" << EV_FIELD(simTime() + portData->getContinuityCheckInterval()) << EV_ENDL;
            }
        }
    }
    relay->registerAddress(ccmMulticastAddress);
}

void Mrp::setTimingProfile(int maxRecoveryTime) {
    //maxrecoverytime in ms,
    switch (maxRecoveryTime) {
    case 500:
        topologyChangeInterval = 20;
        shortTestInterval = 30;
        defaultTestInterval = 50;
        testMonitoringCount = 5;
        linkUpInterval = 20;
        linkDownInterval = 20;
        break;
    case 200:
        topologyChangeInterval = 10;
        shortTestInterval = 10;
        defaultTestInterval = 20;
        testMonitoringCount = 3;
        linkUpInterval = 20;
        linkDownInterval = 20;
        break;
    case 30:
        topologyChangeInterval = 0.5;
        shortTestInterval = 1;
        defaultTestInterval = 3.5;
        testMonitoringCount = 3;
        linkUpInterval = 3;
        linkDownInterval = 3;
        break;
    case 10:
        topologyChangeInterval = 0.5;
        shortTestInterval = 0.5;
        defaultTestInterval = 1;
        testMonitoringCount = 3;
        linkUpInterval = 1;
        linkDownInterval = 1;
        break;
    default:
        throw cRuntimeError("Only RecoveryTimes 500, 200, 30 and 10 ms are defined!");
    }
}

void Mrp::start() {
    fdbClearTimer = new cMessage("fdbClearTimer");
    fdbClearDelay = new cMessage("fdbClearDelay");
    linkDownTimer = new cMessage("LinkDownTimer");
    linkUpTimer = new cMessage("LinkUpTimer");
    topologyChangeTimer = new cMessage("topologyChangeTimer");
    testTimer = new cMessage("testTimer");
    setTimingProfile(timingProfile);
    topologyChangeRepeatCount = topologyChangeMaxRepeatCount - 1;
    if (enableLinkCheckOnRing) {
        startContinuityCheck();
    }
    if (expectedRole == CLIENT) {
        mrcInit();
    } else if (expectedRole == MANAGER) {
        mrmInit();
    } else if (expectedRole == MANAGER_AUTO) {
        mraInit();
    }
}

void Mrp::stop() {
    setPortRole(primaryRingPort, MrpInterfaceData::NOTASSIGNED);
    setPortRole(secondaryRingPort, MrpInterfaceData::NOTASSIGNED);
    setPortState(primaryRingPort, MrpInterfaceData::DISABLED);
    setPortState(secondaryRingPort, MrpInterfaceData::DISABLED);
    expectedRole = DISABLED;
    cancelAndDelete(fdbClearTimer);
    cancelAndDelete(fdbClearDelay);
    cancelAndDelete(topologyChangeTimer);
    cancelAndDelete(testTimer);
    cancelAndDelete(linkDownTimer);
    cancelAndDelete(linkUpTimer);
    cancelAndDelete(linkUpHysteresisTimer);
    cancelAndDelete(startUpTimer);
}

void Mrp::mrcInit() {
    linkChangeCount = linkMaxChange;
    mrpMacForwardingTable->addMrpForwardingInterface(primaryRingPort, static_cast<MacAddress>(MC_CONTROL), vlanID);
    mrpMacForwardingTable->addMrpForwardingInterface(secondaryRingPort, static_cast<MacAddress>(MC_CONTROL), vlanID);
    mrpMacForwardingTable->addMrpForwardingInterface(primaryRingPort, static_cast<MacAddress>(MC_TEST), vlanID);
    mrpMacForwardingTable->addMrpForwardingInterface(secondaryRingPort, static_cast<MacAddress>(MC_TEST), vlanID);
    relay->registerAddress(static_cast<MacAddress>(MC_CONTROL));

    if (interconnectionRingCheckAware || interconnectionLinkCheckAware) {
        mrpMacForwardingTable->addMrpForwardingInterface(primaryRingPort, static_cast<MacAddress>(MC_INCONTROL), vlanID);
        mrpMacForwardingTable->addMrpForwardingInterface(secondaryRingPort, static_cast<MacAddress>(MC_INCONTROL), vlanID);
    }
    if (interconnectionRingCheckAware) {
        mrpMacForwardingTable->addMrpForwardingInterface(primaryRingPort, static_cast<MacAddress>(MC_INTEST), vlanID);
        mrpMacForwardingTable->addMrpForwardingInterface(secondaryRingPort, static_cast<MacAddress>(MC_INTEST), vlanID);
    }
    if (expectedRole == CLIENT) {
        currentRingState = UNDEFINED;
        currentState = AC_STAT1; //only for client, managerAuto is keeping the current state
    }
    mauTypeChangeInd(primaryRingPort, getPortNetworkInterface(primaryRingPort)->getState());
    mauTypeChangeInd(secondaryRingPort, getPortNetworkInterface(secondaryRingPort)->getState());
}

void Mrp::mrmInit() {
    localManagerPrio = DEFAULT;
    currentRingState = OPEN;
    addTest = false;
    testRetransmissionCount = 0;
    relay->registerAddress(static_cast<MacAddress>(MC_TEST));
    relay->registerAddress(static_cast<MacAddress>(MC_CONTROL));
    if (interconnectionRingCheckAware || interconnectionLinkCheckAware)
        relay->registerAddress(static_cast<MacAddress>(MC_INCONTROL));
    if (interconnectionRingCheckAware)
        relay->registerAddress(static_cast<MacAddress>(MC_INTEST));
    testMaxRetransmissionCount = testMonitoringCount - 1;
    testRetransmissionCount = 0;
    if (expectedRole == MANAGER)
        currentState = AC_STAT1;
    if (expectedRole == MANAGER_AUTO) {
        //case: switching from Client to manager. in managerRole no Forwarding on RingPorts may take place
        mrpMacForwardingTable->removeMrpForwardingInterface(primaryRingPort, static_cast<MacAddress>(MC_TEST), vlanID);
        mrpMacForwardingTable->removeMrpForwardingInterface(secondaryRingPort, static_cast<MacAddress>(MC_TEST), vlanID);
        mrpMacForwardingTable->removeMrpForwardingInterface(primaryRingPort, static_cast<MacAddress>(MC_CONTROL), vlanID);
        mrpMacForwardingTable->removeMrpForwardingInterface(secondaryRingPort, static_cast<MacAddress>(MC_CONTROL), vlanID);
    }
    mauTypeChangeInd(primaryRingPort, getPortNetworkInterface(primaryRingPort)->getState());
    mauTypeChangeInd(secondaryRingPort, getPortNetworkInterface(secondaryRingPort)->getState());
}

void Mrp::mraInit() {
    localManagerPrio = MRADEFAULT;
    currentRingState = OPEN;
    addTest = false;
    testRetransmissionCount = 0;
    relay->registerAddress(static_cast<MacAddress>(MC_TEST));
    relay->registerAddress(static_cast<MacAddress>(MC_CONTROL));
    if (interconnectionRingCheckAware || interconnectionLinkCheckAware)
        relay->registerAddress(static_cast<MacAddress>(MC_INCONTROL));
    if (interconnectionRingCheckAware)
        relay->registerAddress(static_cast<MacAddress>(MC_INTEST));
    testMaxRetransmissionCount = testMonitoringCount - 1;
    testRetransmissionCount = 0;
    addTest = false;
    reactOnLinkChange = false;
    hostBestMRMSourceAddress = static_cast<MacAddress>(0xFFFFFFFFFFFF);
    hostBestMRMPriority = static_cast<MrpPriority>(0xFFFF);
    monNReturn = 0;
    currentState = AC_STAT1;
    mauTypeChangeInd(primaryRingPort, getPortNetworkInterface(primaryRingPort)->getState());
    mauTypeChangeInd(secondaryRingPort, getPortNetworkInterface(secondaryRingPort)->getState());
}

void Mrp::clearFDB(double time) {
    if (!fdbClearTimer->isScheduled())
        scheduleAt(simTime() + SimTime(time, SIMTIME_MS), fdbClearTimer);
    else if (fdbClearTimer->getArrivalTime() > (simTime() + SimTime(time, SIMTIME_MS))) {
        cancelEvent(fdbClearTimer);
        scheduleAt(simTime() + SimTime(time, SIMTIME_MS), fdbClearTimer);
    }
}

void Mrp::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) {
    Enter_Method("receiveSignal");
    EV_DETAIL << "Received Signal:" << EV_FIELD(signalID) << EV_ENDL;
    if (signalID == interfaceStateChangedSignal) {
        NetworkInterfaceChangeDetails *interfaceDetails = static_cast<NetworkInterfaceChangeDetails*>(obj);
        auto interface = interfaceDetails->getNetworkInterface();
        auto field = interfaceDetails->getFieldId();
        auto interfaceId = interface->getInterfaceId();
        EV_DETAIL << "Received Signal:" << EV_FIELD(field) << EV_FIELD(interfaceId) << EV_ENDL;
        if (interfaceId >= 0) {
            ProcessDelayTimer *delayTimer = new ProcessDelayTimer("DelayTimer");
            delayTimer->setPort(interface->getInterfaceId());
            delayTimer->setField(field);
            delayTimer->setKind(0);
            if (field == NetworkInterface::F_STATE
                    || field == NetworkInterface::F_CARRIER) {
                if (interface->isUp() && interface->hasCarrier())
                    linkDetectionDelay = SimTime(1, SIMTIME_US); //linkUP is handled faster than linkDown
                else
                    linkDetectionDelay = SimTime(par("linkDetectionDelay").doubleValue(), SIMTIME_MS);
                if (linkUpHysteresisTimer->isScheduled())
                    cancelEvent(linkUpHysteresisTimer);
                scheduleAt(simTime() + linkDetectionDelay, linkUpHysteresisTimer);
                scheduleAt(simTime() + linkDetectionDelay, delayTimer);
            }
        }
    }
}

void Mrp::handleMessageWhenUp(cMessage *msg) {
    if (!msg->isSelfMessage()) {
        msg->setKind(2);
        EV_INFO << "Received Message on MrpNode, Rescheduling:" << EV_FIELD(msg) << EV_ENDL;
        processingDelay = SimTime(par("processingDelay").doubleValue(), SIMTIME_US);
        scheduleAt(simTime() + processingDelay, msg);
    } else {
        EV_INFO << "Received Self-Message:" << EV_FIELD(msg) << EV_ENDL;
        if (msg == testTimer)
            handleTestTimer();
        else if (msg == topologyChangeTimer)
            handleTopologyChangeTimer();
        else if (msg == linkUpTimer)
            handleLinkUpTimer();
        else if (msg == linkDownTimer)
            handleLinkDownTimer();
        else if (msg == fdbClearTimer)
            clearLocalFDB();
        else if (msg == fdbClearDelay)
            clearLocalFDBDelayed();
        else if (msg == startUpTimer) {
            start();
        } else if (msg == linkUpHysteresisTimer) {
            //action done by handleDelayTimer, linkUpHysteresisTimer requested by standard
            //but not further descripted
        } else if (msg->getKind() == 0) {
            ProcessDelayTimer *timer = check_and_cast<ProcessDelayTimer*>(msg);
            if (timer != nullptr) {
                handleDelayTimer(timer->getPort(), timer->getField());
            }
            delete timer;
        } else if (msg->getKind() == 1) {
            ContinuityCheckTimer *timer = check_and_cast<ContinuityCheckTimer*>(msg);
            handleContinuityCheckTimer(timer->getPort());
            delete timer;
        } else if (msg->getKind() == 2) {
            Packet *packet = check_and_cast<Packet*>(msg);
            auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
            if (protocol == &Protocol::ieee8021qCFM) {
                handleContinuityCheckMessage(packet);
            }
            if (protocol == &Protocol::mrp) {
                handleMrpPDU(packet);
            }
        } else
            throw cRuntimeError("Unknown self-message received");
    }
}

void Mrp::handleMrpPDU(Packet* packet) {
    auto interfaceInd = packet->findTag<InterfaceInd>();
    auto macAddressInd = packet->findTag<MacAddressInd>();
    auto sourceAddress = macAddressInd->getSrcAddress();
    auto destinationAddress = macAddressInd->getDestAddress();
    int ringPort = interfaceInd->getInterfaceId();
    auto incomingInterface = interfaceTable->getInterfaceById(ringPort);
    //unsigned int vlanId = 0;
    //if (auto vlanInd = Packet->findTag<VlanInd>())
    //    vlanId = vlanInd->getVlanId();

    auto version = packet->peekAtFront<MrpVersion>();
    auto offset = version->getChunkLength();
    auto firstTlv = packet->peekDataAt<MrpTlvHeader>(offset);
    offset = offset + B(firstTlv->getHeaderLength()) + B(2);
    auto commonTlv = packet->peekDataAt<MrpCommon>(offset);
    auto sequence = commonTlv->getSequenceID();
    bool ringID = false;
    if (commonTlv->getUuid0() == domainID.uuid0
            && commonTlv->getUuid1() == domainID.uuid1)
        ringID = true;
    offset = offset + commonTlv->getChunkLength();

    switch (firstTlv->getHeaderType()) {
    case TEST: {
        EV_DETAIL << "Received Test-Frame" << EV_ENDL;
        auto testTlv = dynamicPtrCast<const MrpTest>(firstTlv);
        if (ringID) {
            if (testTlv->getSa() == localBridgeAddress) {
                auto ringTime = simTime().inUnit(SIMTIME_MS) - testTlv->getTimeStamp();
                auto lastTestFrameSent = testFrameSent.find(sequence);
                if (lastTestFrameSent != testFrameSent.end()) {
                    int64_t ringTimePrecise = simTime().inUnit(SIMTIME_US) - lastTestFrameSent->second;
                    emit(receivedTestSignal, sequence);
                    EV_DETAIL << "RingTime" << EV_FIELD(ringTime) << EV_FIELD(ringTimePrecise) << EV_ENDL;
                } else {
                    emit(receivedTestSignal, sequence);
                    EV_DETAIL << "RingTime" << EV_FIELD(ringTime) << EV_ENDL;
                }
            }
            testRingInd(ringPort, testTlv->getSa(), static_cast<MrpPriority>(testTlv->getPrio()));
        } else {
            EV_DETAIL << "Received packet from other Mrp-Domain"
                             << EV_FIELD(incomingInterface) << EV_FIELD(packet)
                             << EV_ENDL;
        }
        break;
    }
    case TOPOLOGYCHANGE: {
        EV_DETAIL << "Received TopologyChange-Frame" << EV_ENDL;
        auto topologyTlv = dynamicPtrCast<const MrpTopologyChange>(firstTlv);
        if (ringID) {
            if (sequence > lastTopologyId) {
                topologyChangeInd(topologyTlv->getSa(), topologyTlv->getInterval());
                emit(receivedChangeSignal, topologyTlv->getInterval());
            } else {
                EV_DETAIL << "Received same Frame already" << EV_ENDL;
                delete packet;
            }
        } else {
            EV_DETAIL << "Received packet from other Mrp-Domain"
                             << EV_FIELD(incomingInterface) << EV_FIELD(packet)
                             << EV_ENDL;
        }
        break;
    }
    case LINKDOWN:
    case LINKUP: {
        EV_DETAIL << "Received LinkChange-Frame" << EV_ENDL;
        auto linkTlv = dynamicPtrCast<const MrpLinkChange>(firstTlv);
        if (ringID) {
            linkChangeInd(linkTlv->getPortRole(), linkTlv->getBlocked());
            emit(receivedChangeSignal, linkTlv->getInterval());
        } else {
            EV_DETAIL << "Received packet from other Mrp-Domain"
                             << EV_FIELD(incomingInterface) << EV_FIELD(packet)
                             << EV_ENDL;
        }
        break;
    }
    case OPTION: {
        EV_DETAIL << "Received Option-Frame" << EV_ENDL;
        if (ringID) {
            auto optionTlv = dynamicPtrCast<const MrpOption>(firstTlv);
            b subOffset = version->getChunkLength() + optionTlv->getChunkLength();
            //handle if manufactorerData is present
            if (optionTlv->getOuiType() != MrpOuiType::IEC
                    && (optionTlv->getEd1Type() == 0x00
                            || optionTlv->getEd1Type() == 0x04)) {
                if (optionTlv->getEd1Type() == 0x00) {
                    auto dataChunk = packet->peekDataAt<FieldsChunk>(subOffset);
                    subOffset = subOffset + B(Ed1DataLength::LENGTH0);
                } else if (optionTlv->getEd1Type() == 0x04) {
                    auto dataChunk = packet->peekDataAt<FieldsChunk>(subOffset);
                    subOffset = subOffset + B(Ed1DataLength::LENGTH4);
                }
            }

            //handle suboption2 if present
            if ((optionTlv->getOuiType() == MrpOuiType::IEC
                    && optionTlv->getHeaderLength() > 4)
                    || (optionTlv->getEd1Type() == 0x00
                            && optionTlv->getHeaderLength()
                                    > (4 + Ed1DataLength::LENGTH0))
                    || (optionTlv->getEd1Type() == 0x04
                            && optionTlv->getHeaderLength()
                                    > (4 + Ed1DataLength::LENGTH4))) {
                auto subTlv = packet->peekDataAt<MrpSubTlvHeader>(subOffset);
                switch (subTlv->getSubType()) {
                case RESERVED: {
                    auto subOptionTlv = dynamicPtrCast<const MrpManufacturerFkt>(subTlv);
                    //not implemented
                    break;
                }
                case TEST_MGR_NACK: {
                    if (expectedRole == MANAGER_AUTO) {
                        auto subOptionTlv = dynamicPtrCast<const MrpSubTlvTest>(subTlv);
                        testMgrNackInd(ringPort, subOptionTlv->getSa(), static_cast<MrpPriority>(subOptionTlv->getPrio()), subOptionTlv->getOtherMRMSa());
                    }
                    break;
                }
                case TEST_PROPAGATE: {
                    if (expectedRole == MANAGER_AUTO) {
                        auto subOptionTlv = dynamicPtrCast<const MrpSubTlvTest>(subTlv);
                        testPropagateInd(ringPort, subOptionTlv->getSa(), static_cast<MrpPriority>(subOptionTlv->getPrio()), subOptionTlv->getOtherMRMSa(), static_cast<MrpPriority>(subOptionTlv->getOtherMRMPrio()));
                    }
                    break;
                }
                case AUTOMGR: {
                    //nothing else to do
                    EV_DETAIL << "Received TestFrame from Automanager" << EV_ENDL;
                    break;
                }
                default:
                    throw cRuntimeError("unknown subTlv TYPE: %d", subTlv->getSubType());
                }
            }
        } else {
            EV_DETAIL << "Received packet from other Mrp-Domain" << EV_FIELD(incomingInterface) << EV_FIELD(packet) << EV_ENDL;
        }
        break;
    }
    case INTEST: {
        EV_DETAIL << "Received inTest-Frame" << EV_ENDL;
        auto inTestTlv = dynamicPtrCast<const MrpInTest>(firstTlv);
        interconnTestInd(inTestTlv->getSa(), ringPort, inTestTlv->getInID(), packet->dup());
        break;
    }
    case INTOPOLOGYCHANGE: {
        EV_DETAIL << "Received inTopologyChange-Frame" << EV_ENDL;
        auto inTopologyTlv = dynamicPtrCast<const MrpInTopologyChange>(firstTlv);
        interconnTopologyChangeInd(inTopologyTlv->getSa(), inTopologyTlv->getInterval(), inTopologyTlv->getInID(), ringPort, packet->dup());
        break;
    }
    case INLINKDOWN:
    case INLINKUP: {
        EV_DETAIL << "Received inLinkChange-Frame" << EV_ENDL;
        auto inLinkTlv = dynamicPtrCast<const MrpInLinkChange>(firstTlv);
        interconnLinkChangeInd(inLinkTlv->getInID(), inLinkTlv->getLinkInfo(), ringPort, packet->dup());
        break;
    }
    case INLINKSTATUSPOLL: {
        EV_DETAIL << "Received inLinkStatusPoll" << EV_ENDL;
        auto inLinkStatusTlv = dynamicPtrCast<const MrpInLinkStatusPoll>(firstTlv);
        interconnLinkStatusPollInd(inLinkStatusTlv->getInID(), ringPort, packet->dup());
        break;
    }
    default:
        throw cRuntimeError("unknown Tlv TYPE: %d", firstTlv->getHeaderType());
    }

    //addtional Option-Frame
    auto thirdTlv = packet->peekDataAt<MrpTlvHeader>(offset);
    if (thirdTlv->getHeaderType() != END && ringID) {
        EV_DETAIL << "Received additional Option-Frame" << EV_ENDL;
        auto optionTlv = dynamicPtrCast<const MrpOption>(thirdTlv);
        b subOffset = offset + optionTlv->getChunkLength();
        //handle if manufactorerData is present
        if (optionTlv->getOuiType() != MrpOuiType::IEC
                && (optionTlv->getEd1Type() == 0x00
                        || optionTlv->getEd1Type() == 0x04)) {
            if (optionTlv->getEd1Type() == 0x00) {
                auto dataChunk = packet->peekDataAt<FieldsChunk>(subOffset);
                subOffset = subOffset + B(Ed1DataLength::LENGTH0);
            } else if (optionTlv->getEd1Type() == 0x04) {
                auto dataChunk = packet->peekDataAt<FieldsChunk>(subOffset);
                subOffset = subOffset + B(Ed1DataLength::LENGTH4);
            }
        }

        //handle suboption2 if present
        if ((optionTlv->getOuiType() == MrpOuiType::IEC
                && optionTlv->getHeaderLength() > 4)
                || (optionTlv->getEd1Type() == 0x00
                        && optionTlv->getHeaderLength()
                                > (4 + Ed1DataLength::LENGTH0))
                || (optionTlv->getEd1Type() == 0x04
                        && optionTlv->getHeaderLength()
                                > (4 + Ed1DataLength::LENGTH4))) {
            auto subTlv = packet->peekDataAt<MrpSubTlvHeader>(subOffset);
            switch (subTlv->getSubType()) {
            case RESERVED: {
                auto subOptionTlv = dynamicPtrCast<const MrpManufacturerFkt>(subTlv);
                //not implemented
                break;
            }
            case TEST_MGR_NACK: {
                if (expectedRole == MANAGER_AUTO) {
                    auto subOptionTlv = dynamicPtrCast<const MrpSubTlvTest>(subTlv);
                    testMgrNackInd(ringPort, subOptionTlv->getSa(), static_cast<MrpPriority>(subOptionTlv->getPrio()), subOptionTlv->getOtherMRMSa());
                }
                break;
            }
            case TEST_PROPAGATE: {
                if (expectedRole == MANAGER_AUTO) {
                    auto subOptionTlv = dynamicPtrCast<const MrpSubTlvTest>(subTlv);
                    testPropagateInd(ringPort, subOptionTlv->getSa(), static_cast<MrpPriority>(subOptionTlv->getPrio()), subOptionTlv->getOtherMRMSa(), static_cast<MrpPriority>(subOptionTlv->getOtherMRMPrio()));
                }
                break;
            }
            case AUTOMGR: {
                //nothing else to do
                EV_DETAIL << "Received TestFrame from Automanager" << EV_ENDL;
                break;
            }
            default:
                throw cRuntimeError("unknown subTlv TYPE: %d", subTlv->getSubType());
            }
        }
        offset = offset + B(thirdTlv->getHeaderLength()) + B(2);
    }
    //auto endTlv=Packet->peekDataAt<TlvHeader>(offset);
    delete packet;
}

void Mrp::handleContinuityCheckMessage(Packet* packet) {
    EV_DETAIL << "Handling CCM" << EV_ENDL;
    auto interfaceInd = packet->getTag<InterfaceInd>();
    auto macAddressInd = packet->getTag<MacAddressInd>();
    auto sourceAddress = macAddressInd->getSrcAddress();
    int ringPort = interfaceInd->getInterfaceId();
    auto incomingInterface = interfaceTable->getInterfaceById(ringPort);
    auto ccm = packet->popAtFront<ContinuityCheckMessage>();
    auto portData = getPortInterfaceDataForUpdate(ringPort);
    if (ccm->getEndpointIdentifier() == portData->getCfmEndpointID()) {
        int i = sourceAddress.compareTo(incomingInterface->getMacAddress());
        if (i == -1) {
            portData->setCfmEndpointID(2);
            MacAddress address = incomingInterface->getMacAddress();
            std::string namePort = "CFM CCM " + address.str();
            portData->setCfmName(namePort);
        }
    }
    portData->setNextUpdate(simTime() + portData->getContinuityCheckInterval() * 3.5);
    EV_DEBUG << "new ccm-Data" << EV_FIELD(portData->getNextUpdate())
            << EV_FIELD(portData->getCfmEndpointID())
            << EV_FIELD(sourceAddress)
            << EV_FIELD(incomingInterface->getMacAddress())
            << EV_ENDL;
    mauTypeChangeInd(ringPort, NetworkInterface::UP);
    emit(receivedContinuityCheckSignal, ringPort);
    delete packet;
}

void Mrp::handleDelayTimer(int interfaceId, int field) {
    EV_DETAIL << "handling DelayTimer" << EV_ENDL;
    auto interface = getPortNetworkInterface(interfaceId);
    if (field == NetworkInterface::F_STATE
            || (field == NetworkInterface::F_CARRIER && interface->isUp())) {
        auto portData = getPortInterfaceDataForUpdate(interfaceId);
        if (portData->getContinuityCheck()) {
            auto nextUpdate = simTime() + portData->getContinuityCheckInterval() * 3.5;
            portData->setNextUpdate(nextUpdate);
        }
        if (interface->isUp() && interface->hasCarrier())
            mauTypeChangeInd(interfaceId, NetworkInterface::UP);
        else
            mauTypeChangeInd(interfaceId, NetworkInterface::DOWN);
    }
}

void Mrp::clearLocalFDB() {
    EV_DETAIL << "clearing FDB" << EV_ENDL;
    if (fdbClearDelay->isScheduled())
        cancelEvent(fdbClearDelay);
    processingDelay = SimTime(par("processingDelay").doubleValue(), SIMTIME_US);
    scheduleAt(simTime() + processingDelay, fdbClearDelay);
    emit(clearFDBSignal, processingDelay.dbl());
}

void Mrp::clearLocalFDBDelayed() {
    mrpMacForwardingTable->clearTable();
    EV_DETAIL << "FDB cleared" << EV_ENDL;
}

bool Mrp::isBetterThanOwnPrio(MrpPriority remotePrio, MacAddress remoteAddress) {
    if (remotePrio < localManagerPrio)
        return true;
    if (remotePrio == localManagerPrio && remoteAddress < localBridgeAddress)
        return true;
    return false;
}

bool Mrp::isBetterThanBestPrio(MrpPriority remotePrio, MacAddress remoteAddress) {
    if (remotePrio < hostBestMRMPriority)
        return true;
    if (remotePrio == hostBestMRMPriority && remoteAddress < hostBestMRMSourceAddress)
        return true;
    return false;
}

void Mrp::handleContinuityCheckTimer(int ringPort) {
    auto portData = getPortInterfaceDataForUpdate(ringPort);
    EV_DETAIL << "Checktimer:" << EV_FIELD(simTime()) << EV_FIELD(ringPort)
                     << EV_FIELD(portData->getNextUpdate()) << EV_ENDL;
    if (simTime() >= portData->getNextUpdate()) {
        //no Message received within Lifetime
        EV_DETAIL << "Checktimer: Link considered down" << EV_ENDL;
        mauTypeChangeInd(ringPort, NetworkInterface::DOWN);
    }
    setupContinuityCheck(ringPort);
    ContinuityCheckTimer *checkTimer = new ContinuityCheckTimer("continuityCheckTimer");
    checkTimer->setPort(ringPort);
    checkTimer->setKind(1);
    scheduleAt(simTime() + portData->getContinuityCheckInterval(), checkTimer);
}

void Mrp::handleTestTimer() {
    switch (currentState) {
    case POWER_ON:
    case AC_STAT1:
        if (expectedRole == MANAGER) {
            mauTypeChangeInd(primaryRingPort, getPortNetworkInterface(primaryRingPort)->getState());
        }
        break;
    case PRM_UP:
        addTest = false;
        testRingReq(defaultTestInterval);
        break;
    case CHK_RO:
        addTest = false;
        testRingReq(defaultTestInterval);
        break;
    case CHK_RC:
        if (testRetransmissionCount >= testMaxRetransmissionCount) {
            setPortState(secondaryRingPort, MrpInterfaceData::FORWARDING);
            testMaxRetransmissionCount = testMonitoringCount - 1;
            testRetransmissionCount = 0;
            addTest = false;
            if (!noTopologyChange) {
                topologyChangeReq(topologyChangeInterval);
            }
            testRingReq(defaultTestInterval);
            currentState = CHK_RO;
            EV_DETAIL << "Switching State from CHK_RC to CHK_RO" << EV_FIELD(currentState) << EV_ENDL;
            currentRingState = OPEN;
            emit(ringStateChangedSignal, currentRingState);
        } else {
            testRetransmissionCount++;
            addTest = false;
            testRingReq(defaultTestInterval);
        }
        break;
    case DE:
    case DE_IDLE:
        if (expectedRole == MANAGER_AUTO) {
            scheduleAt(simTime() + SimTime(shortTestInterval, SIMTIME_MS), testTimer);
            if (monNReturn <= monNRmax) {
                monNReturn++;
            } else {
                mrmInit();
                currentState = PRM_UP;
                EV_DETAIL << "Switching State from DE_IDLE to PRM_UP" << EV_FIELD(currentState) << EV_ENDL;
            }
        }
        break;
    case PT:
        if (expectedRole == MANAGER_AUTO) {
            scheduleAt(simTime() + SimTime(shortTestInterval, SIMTIME_MS), testTimer);
            if (monNReturn <= monNRmax) {
                monNReturn++;
            } else {
                mrmInit();
                currentState = CHK_RC;
                EV_DETAIL << "Switching State from PT to CHK_RC" << EV_FIELD(currentState) << EV_ENDL;
            }
        }
        break;
    case PT_IDLE:
        if (expectedRole == MANAGER_AUTO) {
            scheduleAt(simTime() + SimTime(shortTestInterval, SIMTIME_MS), testTimer);
            if (monNReturn <= monNRmax) {
                monNReturn++;
            } else {
                mrmInit();
                currentState = CHK_RO;
                EV_DETAIL << "Switching State from PT_IDLE to CHK_RO" << EV_FIELD(currentState) << EV_ENDL;
            }
        }
        break;
    default:
        throw cRuntimeError("Unknown Node State");
    }
}

void Mrp::handleTopologyChangeTimer() {
    if (topologyChangeRepeatCount > 0) {
        setupTopologyChangeReq(topologyChangeRepeatCount * topologyChangeInterval);
        topologyChangeRepeatCount--;
        scheduleAt(simTime() + SimTime(topologyChangeInterval, SIMTIME_MS), topologyChangeTimer);
    } else {
        topologyChangeRepeatCount = topologyChangeMaxRepeatCount - 1;
        clearLocalFDB();
    }
}

void Mrp::handleLinkUpTimer() {
    switch (currentState) {
    case PT:
        if (linkChangeCount == 0) {
            setPortState(secondaryRingPort, MrpInterfaceData::FORWARDING);
            linkChangeCount = linkMaxChange;
            currentState = PT_IDLE;
            EV_DETAIL << "Switching State from PT to PT_IDLE" << EV_FIELD(currentState) << EV_ENDL;
        } else {
            linkChangeReq(primaryRingPort, NetworkInterface::UP);
        }
        break;
    case DE:
    case PT_IDLE:
    case POWER_ON:
    case AC_STAT1:
    case PRM_UP:
    case CHK_RO:
    case CHK_RC:
    case DE_IDLE:
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void Mrp::handleLinkDownTimer() {
    switch (currentState) {
    case DE:
        if (linkChangeCount == 0) {
            linkChangeCount = linkMaxChange;
            currentState = DE_IDLE;
            EV_DETAIL << "Switching State from DE to DE_IDLE" << EV_FIELD(currentState) << EV_ENDL;
        } else {
            linkChangeReq(primaryRingPort, NetworkInterface::DOWN);
        }
        break;
    case POWER_ON:
    case AC_STAT1:
    case PRM_UP:
    case CHK_RO:
    case CHK_RC:
    case DE_IDLE:
    case PT:
    case PT_IDLE:
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void Mrp::setupContinuityCheck(int ringPort) {
    auto ccm = makeShared<ContinuityCheckMessage>();
    auto portData = getPortInterfaceDataForUpdate(ringPort);
    if (portData->getContinuityCheckInterval() == 3.3) {
        ccm->setFlags(0x00000001);
    } else if (portData->getContinuityCheckInterval() == 10) {
        ccm->setFlags(0x00000002);
    }
    if (ringPort == primaryRingPort) {
        ccm->setSequenceNumber(sequenceCCM1);
        sequenceCCM1++;
    } else if (ringPort == secondaryRingPort) {
        ccm->setSequenceNumber(sequenceCCM2);
        sequenceCCM2++;
    }
    ccm->setEndpointIdentifier(portData->getCfmEndpointID());
    auto name = portData->getCfmName();
    ccm->setMessageName(name.c_str());
    auto packet = new Packet("ContinuityCheck", ccm);
    sendCCM(ringPort, packet);
    emit(continuityCheckSignal, ringPort);
}

void Mrp::testRingReq(double time) {
    if (addTest)
        cancelEvent(testTimer);
    if (!testTimer->isScheduled()) {
        scheduleAt(simTime() + SimTime(time, SIMTIME_MS), testTimer);
        setupTestRingReq();
    } else
        EV_DETAIL << "Testtimer already scheduled" << EV_ENDL;
}

void Mrp::topologyChangeReq(double time) {
    if (time == 0) {
        clearLocalFDB();
        setupTopologyChangeReq(time * topologyChangeMaxRepeatCount);
    } else if (!topologyChangeTimer->isScheduled()) {
        scheduleAt(simTime() + SimTime(time, SIMTIME_MS), topologyChangeTimer);
        setupTopologyChangeReq(time * topologyChangeMaxRepeatCount);
    } else
        EV_DETAIL << "TopologyChangeTimer already scheduled" << EV_ENDL;

}

void Mrp::linkChangeReq(int ringPort, uint16_t linkState) {
    if (linkState == NetworkInterface::DOWN) {
        if (!linkDownTimer->isScheduled()) {
            scheduleAt(simTime() + SimTime(linkDownInterval, SIMTIME_MS), linkDownTimer);
            setupLinkChangeReq(primaryRingPort, NetworkInterface::DOWN, linkChangeCount * linkDownInterval);
            linkChangeCount--;
        }
    } else if (linkState == NetworkInterface::UP) {
        if (!linkUpTimer->isScheduled()) {
            scheduleAt(simTime() + SimTime(linkUpInterval, SIMTIME_MS), linkUpTimer);
            setupLinkChangeReq(primaryRingPort, NetworkInterface::UP, linkChangeCount * linkUpInterval);
            linkChangeCount--;
        }
    } else
        throw cRuntimeError("Unknown LinkState in LinkChangeReq");
}

void Mrp::setupTestRingReq() {
    //Create MRP-PDU according MRP_Test
    auto version = makeShared<MrpVersion>();
    auto testTlv1 = makeShared<MrpTest>();
    auto testTlv2 = makeShared<MrpTest>();
    auto commonTlv = makeShared<MrpCommon>();
    auto endTlv = makeShared<MrpEnd>();

    auto timestamp = simTime().inUnit(SIMTIME_MS);
    auto lastTestFrameSent = simTime().inUnit(SIMTIME_US);
    testFrameSent.insert( { sequenceID, lastTestFrameSent });

    testTlv1->setPrio(localManagerPrio);
    testTlv1->setSa(localBridgeAddress);
    testTlv1->setPortRole(MrpInterfaceData::PRIMARY);
    testTlv1->setRingState(currentRingState);
    testTlv1->setTransition(transition);
    testTlv1->setTimeStamp(timestamp);

    testTlv2->setPrio(localManagerPrio);
    testTlv2->setSa(localBridgeAddress);
    testTlv2->setPortRole(MrpInterfaceData::PRIMARY);
    testTlv2->setRingState(currentRingState);
    testTlv2->setTransition(transition);
    testTlv2->setTimeStamp(timestamp);

    commonTlv->setSequenceID(sequenceID);
    sequenceID++;
    commonTlv->setUuid0(domainID.uuid0);
    commonTlv->setUuid1(domainID.uuid1);

    Packet *packet1 = new Packet("mrpTestFrame");
    packet1->insertAtBack(version);
    packet1->insertAtBack(testTlv1);
    packet1->insertAtBack(commonTlv);

    Packet *packet2 = new Packet("mrpTestFrame");
    packet2->insertAtBack(version);
    packet2->insertAtBack(testTlv2);
    packet2->insertAtBack(commonTlv);

    //MRA only
    if (expectedRole == MANAGER_AUTO) {
        auto optionTlv = makeShared<MrpOption>();
        auto autoMgrTlv = makeShared<MrpSubTlvHeader>();
        uint8_t headerLength = optionTlv->getHeaderLength() + autoMgrTlv->getSubHeaderLength() + 2;
        optionTlv->setHeaderLength(headerLength);
        packet1->insertAtBack(optionTlv);
        packet1->insertAtBack(autoMgrTlv);
        packet2->insertAtBack(optionTlv);
        packet2->insertAtBack(autoMgrTlv);
    }
    packet1->insertAtBack(endTlv);
    MacAddress sourceAddress1 = getPortNetworkInterface(primaryRingPort)->getMacAddress();
    sendFrameReq(primaryRingPort, static_cast<MacAddress>(MC_TEST), sourceAddress1, priority, MRP_LT, packet1);

    packet2->insertAtBack(endTlv);
    MacAddress sourceAddress2 = getPortNetworkInterface(secondaryRingPort)->getMacAddress();
    sendFrameReq(secondaryRingPort, static_cast<MacAddress>(MC_TEST), sourceAddress2, priority, MRP_LT, packet2);
    emit(testSignal, lastTestFrameSent);
}

void Mrp::setupTopologyChangeReq(uint32_t Interval) {
    //Create MRP-PDU according MRP_TopologyChange
    auto version = makeShared<MrpVersion>();
    auto topologyChangeTlv = makeShared<MrpTopologyChange>();
    auto topologyChangeTlv2 = makeShared<MrpTopologyChange>();
    auto commonTlv = makeShared<MrpCommon>();
    auto endTlv = makeShared<MrpEnd>();

    topologyChangeTlv->setPrio(localManagerPrio);
    topologyChangeTlv->setSa(localBridgeAddress);
    topologyChangeTlv->setPortRole(MrpInterfaceData::PRIMARY);
    topologyChangeTlv->setInterval(Interval);
    topologyChangeTlv2->setPrio(localManagerPrio);
    topologyChangeTlv2->setPortRole(MrpInterfaceData::SECONDARY);
    topologyChangeTlv2->setSa(localBridgeAddress);
    topologyChangeTlv2->setInterval(Interval);

    commonTlv->setSequenceID(sequenceID);
    sequenceID++;
    commonTlv->setUuid0(domainID.uuid0);
    commonTlv->setUuid1(domainID.uuid1);

    auto packet1 = new Packet("mrpTopologyChange");
    packet1->insertAtBack(version);
    packet1->insertAtBack(topologyChangeTlv);
    packet1->insertAtBack(commonTlv);
    packet1->insertAtBack(endTlv);
    MacAddress sourceAddress1 = getPortNetworkInterface(primaryRingPort)->getMacAddress();
    sendFrameReq(primaryRingPort, static_cast<MacAddress>(MC_CONTROL), sourceAddress1, priority, MRP_LT, packet1);

    auto packet2 = new Packet("mrpTopologyChange");
    packet2->insertAtBack(version);
    packet2->insertAtBack(topologyChangeTlv2);
    packet2->insertAtBack(commonTlv);
    packet2->insertAtBack(endTlv);
    MacAddress sourceAddress2 = getPortNetworkInterface(secondaryRingPort)->getMacAddress();
    sendFrameReq(secondaryRingPort, static_cast<MacAddress>(MC_CONTROL), sourceAddress2, priority, MRP_LT, packet2);
    emit(topologyChangeSignal, Interval);
}

void Mrp::setupLinkChangeReq(int ringPort, uint16_t linkState, double time) {
    //Create MRP-PDU according MRP_LinkChange
    auto version = makeShared<MrpVersion>();
    auto linkChangeTlv = makeShared<MrpLinkChange>();
    auto commonTlv = makeShared<MrpCommon>();
    auto endTlv = makeShared<MrpEnd>();

    if (linkState == NetworkInterface::UP) {
        linkChangeTlv->setHeaderType(LINKUP);
    } else if (linkState == NetworkInterface::DOWN) {
        linkChangeTlv->setHeaderType(LINKDOWN);
    } else {
        throw cRuntimeError("Unknown LinkState in linkChangeRequest");
    }
    linkChangeTlv->setSa(localBridgeAddress);
    linkChangeTlv->setPortRole(getPortRole(ringPort));
    linkChangeTlv->setInterval(time);
    linkChangeTlv->setBlocked(linkState);

    commonTlv->setSequenceID(sequenceID);
    sequenceID++;
    commonTlv->setUuid0(domainID.uuid0);
    commonTlv->setUuid1(domainID.uuid1);

    auto packet1 = new Packet("mrpLinkChange");
    packet1->insertAtBack(version);
    packet1->insertAtBack(linkChangeTlv);
    packet1->insertAtBack(commonTlv);
    packet1->insertAtBack(endTlv);
    MacAddress sourceAddress1 = getPortNetworkInterface(ringPort)->getMacAddress();
    sendFrameReq(ringPort, static_cast<MacAddress>(MC_CONTROL), sourceAddress1, priority, MRP_LT, packet1);
    emit(linkChangeSignal, time);
}

void Mrp::testMgrNackReq(int ringPort, MrpPriority managerPrio, MacAddress sourceAddress) {
    //Create MRP-PDU according MRP_Option and Suboption2 MRP-TestMgrNack
    auto version = makeShared<MrpVersion>();
    auto optionTlv = makeShared<MrpOption>();
    auto testMgrTlv = makeShared<MrpSubTlvTest>();
    auto commonTlv = makeShared<MrpCommon>();
    auto endTlv = makeShared<MrpEnd>();

    testMgrTlv->setSubType(SubTlvHeaderType::TEST_MGR_NACK);
    testMgrTlv->setPrio(localManagerPrio);
    testMgrTlv->setSa(localBridgeAddress);
    testMgrTlv->setOtherMRMPrio(0x00);
    testMgrTlv->setOtherMRMSa(sourceAddress);

    optionTlv->setHeaderLength(optionTlv->getHeaderLength() + testMgrTlv->getSubHeaderLength() + 2);

    commonTlv->setSequenceID(sequenceID);
    sequenceID++;
    commonTlv->setUuid0(domainID.uuid0);
    commonTlv->setUuid1(domainID.uuid1);

    Packet *packet1 = new Packet("mrpTestMgrNackFrame");
    packet1->insertAtBack(version);
    packet1->insertAtBack(optionTlv);
    packet1->insertAtBack(testMgrTlv);
    packet1->insertAtBack(commonTlv);
    packet1->insertAtBack(endTlv);

    //Standard request sending out both packets, but defines a handshake.
    //sending the answer back would be enough
    //Code will remain comment out for easy optimization
    //MacAddress SourceAddress1 = getPortNetworkInterface(RingPort)->getMacAddress();
    //sendFrameReq(RingPort, static_cast<MacAddress>(MC_TEST), SourceAddress1, priority, MRP_LT , packet1);

    Packet *packet2 = new Packet("mrpTestMgrNackFrame");
    packet2->insertAtBack(version);
    packet2->insertAtBack(optionTlv);
    packet2->insertAtBack(testMgrTlv);
    packet2->insertAtBack(commonTlv);
    packet2->insertAtBack(endTlv);

    MacAddress sourceAddress1 = getPortNetworkInterface(primaryRingPort)->getMacAddress();
    sendFrameReq(primaryRingPort, static_cast<MacAddress>(MC_TEST), sourceAddress1, priority, MRP_LT, packet1);
    MacAddress sourceAddress2 = getPortNetworkInterface(secondaryRingPort)->getMacAddress();
    sendFrameReq(secondaryRingPort, static_cast<MacAddress>(MC_TEST), sourceAddress2, priority, MRP_LT, packet2);
}

void Mrp::testPropagateReq(int ringPort, MrpPriority managerPrio, MacAddress sourceAddress) {
    //Create MRP-PDU according MRP_Option and Suboption2 MRP-TestPropagate
    auto version = makeShared<MrpVersion>();
    auto optionTlv = makeShared<MrpOption>();
    auto testMgrTlv = makeShared<MrpSubTlvTest>();
    auto commonTlv = makeShared<MrpCommon>();
    auto endTlv = makeShared<MrpEnd>();

    testMgrTlv->setPrio(localManagerPrio);
    testMgrTlv->setSa(localBridgeAddress);
    testMgrTlv->setOtherMRMPrio(managerPrio);
    testMgrTlv->setOtherMRMSa(sourceAddress);

    optionTlv->setHeaderLength(optionTlv->getHeaderLength() + testMgrTlv->getSubHeaderLength() + 2);

    commonTlv->setSequenceID(sequenceID);
    sequenceID++;
    commonTlv->setUuid0(domainID.uuid0);
    commonTlv->setUuid1(domainID.uuid1);

    Packet *packet1 = new Packet("mrpTestPropagateFrame");
    packet1->insertAtBack(version);
    packet1->insertAtBack(optionTlv);
    packet1->insertAtBack(testMgrTlv);
    packet1->insertAtBack(commonTlv);
    packet1->insertAtBack(endTlv);

    //Standard request sending out both packets, but defines a handshake.
    //sending the answer back would be enough
    //Code will remain comment out for easy optimization
    //MacAddress SourceAddress1 = getPortNetworkInterface(RingPort)->getMacAddress();
    //sendFrameReq(RingPort, static_cast<MacAddress>(MC_TEST), SourceAddress1, priority, MRP_LT , packet1);

    Packet *packet2 = new Packet("mrpTestPropagateFrame");
    packet2->insertAtBack(version);
    packet2->insertAtBack(optionTlv);
    packet2->insertAtBack(testMgrTlv);
    packet2->insertAtBack(commonTlv);
    packet2->insertAtBack(endTlv);

    MacAddress sourceAddress1 = getPortNetworkInterface(primaryRingPort)->getMacAddress();
    sendFrameReq(primaryRingPort, static_cast<MacAddress>(MC_TEST), sourceAddress1, priority, MRP_LT, packet1);
    MacAddress sourceAddress2 = getPortNetworkInterface(secondaryRingPort)->getMacAddress();
    sendFrameReq(secondaryRingPort, static_cast<MacAddress>(MC_TEST), sourceAddress2, priority, MRP_LT, packet2);
}

void Mrp::testRingInd(int ringPort, MacAddress sourceAddress, MrpPriority managerPrio) {
    switch (currentState) {
    case POWER_ON:
    case AC_STAT1:
        break;
    case PRM_UP:
        if (sourceAddress == localBridgeAddress) {
            testMaxRetransmissionCount = testMonitoringCount - 1;
            testRetransmissionCount = 0;
            noTopologyChange = false;
            testRingReq(defaultTestInterval);
            currentState = CHK_RC;
            EV_DETAIL << "Switching State from PRM_UP to CHK_RC" << EV_FIELD(currentState) << EV_ENDL;
            currentRingState = CLOSED;
            emit(ringStateChangedSignal, currentRingState);
        } else if (expectedRole == MANAGER_AUTO
                && !isBetterThanOwnPrio(managerPrio, sourceAddress)) {
            testMgrNackReq(ringPort, managerPrio, sourceAddress);
        }
        //all other cases: ignore
        break;
    case CHK_RO:
        if (sourceAddress == localBridgeAddress) {
            setPortState(secondaryRingPort, MrpInterfaceData::BLOCKED);
            testMaxRetransmissionCount = testMonitoringCount - 1;
            testRetransmissionCount = 0;
            noTopologyChange = false;
            testRingReq(defaultTestInterval);
            if (!reactOnLinkChange) {
                topologyChangeReq(topologyChangeInterval);
            } else {
                double time = 0;
                topologyChangeReq(time);
            }
            currentState = CHK_RC;
            EV_DETAIL << "Switching State from CHK_RO to CHK_RC" << EV_FIELD(currentState) << EV_ENDL;
            currentRingState = CLOSED;
            emit(ringStateChangedSignal, currentRingState);
        } else if (expectedRole == MANAGER_AUTO
                && !isBetterThanOwnPrio(managerPrio, sourceAddress)) {
            testMgrNackReq(ringPort, managerPrio, sourceAddress);
        }
        //all other cases: ignore
        break;
    case CHK_RC:
        if (sourceAddress == localBridgeAddress) {
            testMaxRetransmissionCount = testMonitoringCount - 1;
            testRetransmissionCount = 0;
            noTopologyChange = false;
        } else if (expectedRole == MANAGER_AUTO
                && !isBetterThanOwnPrio(managerPrio, sourceAddress)) {
            testMgrNackReq(ringPort, managerPrio, sourceAddress);
        }
        break;
    case DE:
    case DE_IDLE:
    case PT:
    case PT_IDLE:
        if (expectedRole == MANAGER_AUTO && sourceAddress != localBridgeAddress
                && sourceAddress == hostBestMRMSourceAddress) {
            if (managerPrio < localManagerPrio
                    || (managerPrio == localManagerPrio
                            && sourceAddress < localBridgeAddress)) {
                monNReturn = 0;
            }
            hostBestMRMPriority = managerPrio;
        }
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void Mrp::topologyChangeInd(MacAddress sourceAddress, double time) {
    switch (currentState) {
    case POWER_ON:
    case AC_STAT1:
        break;
    case PRM_UP:
    case CHK_RO:
    case CHK_RC:
        if (sourceAddress != localBridgeAddress) {
            clearFDB(time);
        }
        break;
    case PT:
        linkChangeCount = linkMaxChange;
        cancelEvent(linkUpTimer);
        setPortState(secondaryRingPort, MrpInterfaceData::FORWARDING);
        clearFDB(time);
        currentState = PT_IDLE;
        EV_DETAIL << "Switching State from PT to PT_IDLE" << EV_FIELD(currentState) << EV_ENDL;
        break;
    case DE:
        linkChangeCount = linkMaxChange;
        cancelEvent(linkDownTimer);
        clearFDB(time);
        currentState = DE_IDLE;
        EV_DETAIL << "Switching State from DE to DE_IDLE" << EV_FIELD(currentState) << EV_ENDL;
        break;
    case DE_IDLE:
        clearFDB(time);
        if (expectedRole == MANAGER_AUTO
                && linkUpHysteresisTimer->isScheduled()) {
            setPortState(secondaryRingPort, MrpInterfaceData::FORWARDING);
            currentState = PT_IDLE;
            EV_DETAIL << "Switching State from DE_IDLE to PT_IDLE" << EV_FIELD(currentState) << EV_ENDL;
        }
    case PT_IDLE:
        clearFDB(time);
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void Mrp::linkChangeInd(uint16_t portState, uint16_t linkState) {
    switch (currentState) {
    case POWER_ON:
    case AC_STAT1:
    case DE_IDLE:
    case PT:
    case DE:
    case PT_IDLE:
        break;
    case PRM_UP:
        if (!addTest) {
            if (nonBlockingMRC) { //15
                addTest = true;
                testRingReq(shortTestInterval);
                break;
            } else if (linkState == NetworkInterface::UP) {
                addTest = true;
                testRingReq(shortTestInterval);
                double time = 0;
                topologyChangeReq(time);
                break;
            }
        } else {
            if (!nonBlockingMRC && linkState == NetworkInterface::UP) { //18
                double time = 0;
                topologyChangeReq(time);
            }
            break;
        }
        //all other cases : ignore
        break;
    case CHK_RO:
        if (!addTest) {
            if (linkState == NetworkInterface::DOWN) {
                addTest = true;
                testRingReq(shortTestInterval);
                break;
            } else if (linkState == NetworkInterface::UP) {
                if (nonBlockingMRC) {
                    addTest = true;
                    testRingReq(shortTestInterval);
                } else {
                    setPortState(secondaryRingPort, MrpInterfaceData::BLOCKED);
                    testMaxRetransmissionCount = testMonitoringExtendedCount - 1;
                    testRetransmissionCount = 0;
                    addTest = true;
                    testRingReq(shortTestInterval);
                    double time = 0;
                    topologyChangeReq(time);
                    currentState = CHK_RC;
                    currentRingState = CLOSED;
                    emit(ringStateChangedSignal, currentRingState);
                    EV_DETAIL << "Switching State from CHK_RO to CHK_RC" << EV_FIELD(currentState) << EV_ENDL;
                }
            }
        } else {
            if (!nonBlockingMRC && linkState == NetworkInterface::UP) {
                setPortState(secondaryRingPort, MrpInterfaceData::BLOCKED);
                testMaxRetransmissionCount = testMonitoringExtendedCount - 1;
                testRetransmissionCount = 0;
                testRingReq(defaultTestInterval);
                double time = 0;
                topologyChangeReq(time);
                currentState = CHK_RC;
                currentRingState = CLOSED;
                emit(ringStateChangedSignal, currentRingState);
                EV_DETAIL << "Switching State from CHK_RO to CHK_RC" << EV_FIELD(currentState) << EV_ENDL;
            }
        }
        //all other cases: ignore
        break;
    case CHK_RC:
        if (reactOnLinkChange) {
            if (linkState == NetworkInterface::DOWN) {
                setPortState(secondaryRingPort, MrpInterfaceData::FORWARDING);
                double time = 0;
                topologyChangeReq(time);
                currentState = CHK_RO;
                currentRingState = OPEN;
                emit(ringStateChangedSignal, currentRingState);
                EV_DETAIL << "Switching State from CHK_RC to CHK_RO" << EV_FIELD(currentState) << EV_ENDL;
                break;
            } else if (linkState == NetworkInterface::UP) {
                if (nonBlockingMRC) {
                    testMaxRetransmissionCount = testMonitoringCount - 1;
                } else {
                    testMaxRetransmissionCount = testMonitoringExtendedCount - 1;
                }
                double time = 0;
                topologyChangeReq(time);
            }
        } else if (nonBlockingMRC) {
            if (!addTest) {
                addTest = true;
                testRingReq(shortTestInterval);
            }
        }
        break;
    default:
        throw cRuntimeError("Unknown state of MRM");
    }
}

void Mrp::testMgrNackInd(int ringPort, MacAddress sourceAddress, MrpPriority managerPrio, MacAddress bestMRMSourceAddress) {
    switch (currentState) {
    case POWER_ON:
    case AC_STAT1:
    case DE:
    case DE_IDLE:
    case PT:
    case PT_IDLE:
        break;
    case PRM_UP:
        if (expectedRole == MANAGER_AUTO && sourceAddress != localBridgeAddress
                && bestMRMSourceAddress == localBridgeAddress) {
            if (isBetterThanBestPrio(managerPrio, sourceAddress)) {
                hostBestMRMSourceAddress = sourceAddress;
                hostBestMRMPriority = managerPrio;
            }
            cancelEvent(topologyChangeTimer);
            mrcInit();
            testPropagateReq(ringPort, hostBestMRMPriority, hostBestMRMSourceAddress);
            currentState = DE_IDLE;
            EV_DETAIL << "Switching State from PRM_UP to DE_IDLE" << EV_FIELD(currentState) << EV_ENDL;
        }
        break;
    case CHK_RO:
        if (expectedRole == MANAGER_AUTO && sourceAddress != localBridgeAddress
                && bestMRMSourceAddress == localBridgeAddress) {
            if (isBetterThanBestPrio(managerPrio, sourceAddress)) {
                hostBestMRMSourceAddress = sourceAddress;
                hostBestMRMPriority = managerPrio;
            }
            cancelEvent(topologyChangeTimer);
            mrcInit();
            testPropagateReq(ringPort, hostBestMRMPriority, hostBestMRMSourceAddress);
            currentState = PT_IDLE;
            EV_DETAIL << "Switching State from CHK_RO to PT_IDLE" << EV_FIELD(currentState) << EV_ENDL;
        }
        break;
    case CHK_RC:
        if (expectedRole == MANAGER_AUTO && sourceAddress != localBridgeAddress
                && bestMRMSourceAddress == localBridgeAddress) {
            if (isBetterThanBestPrio(managerPrio, sourceAddress)) {
                hostBestMRMSourceAddress = sourceAddress;
                hostBestMRMPriority = managerPrio;
            }
            cancelEvent(topologyChangeTimer);
            mrcInit();
            testPropagateReq(ringPort, hostBestMRMPriority, hostBestMRMSourceAddress);
            setPortState(secondaryRingPort, MrpInterfaceData::FORWARDING);
            currentState = PT_IDLE;
            EV_DETAIL << "Switching State from CHK_RC to PT_IDLE" << EV_FIELD(currentState) << EV_ENDL;
        }
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void Mrp::testPropagateInd(int ringPort, MacAddress sourceAddress, MrpPriority managerPrio, MacAddress bestMRMSourceAddress, MrpPriority bestMRMPrio) {
    switch (currentState) {
    case POWER_ON:
    case AC_STAT1:
    case PRM_UP:
    case CHK_RO:
    case CHK_RC:
        break;
    case DE:
    case DE_IDLE:
    case PT:
    case PT_IDLE:
        if (expectedRole == MANAGER_AUTO && sourceAddress != localBridgeAddress
                && sourceAddress == hostBestMRMSourceAddress) {
            hostBestMRMSourceAddress = bestMRMSourceAddress;
            hostBestMRMPriority = bestMRMPrio;
            monNReturn = 0;
        }
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void Mrp::mauTypeChangeInd(int ringPort, uint16_t linkState) {
    switch (currentState) {
    case POWER_ON:
        //all cases: ignore
        break;
    case AC_STAT1:
        //Client
        if (expectedRole == CLIENT) {
            if (linkState == NetworkInterface::UP) {
                if (ringPort == primaryRingPort) {
                    setPortState(primaryRingPort, MrpInterfaceData::FORWARDING);
                    EV_DETAIL << "Switching State from AC_STAT1 to DE_IDLE" << EV_ENDL;
                    currentState = DE_IDLE;
                    break;
                } else if (ringPort == secondaryRingPort) {
                    toggleRingPorts();
                    setPortState(primaryRingPort, MrpInterfaceData::FORWARDING);
                    EV_DETAIL << "Switching State from AC_STAT1 to DE_IDLE" << EV_ENDL;
                    currentState = DE_IDLE;
                    break;
                }
            }
            //all other cases: ignore
            break;
        }
        //Manager
        if (expectedRole == MANAGER || expectedRole == MANAGER_AUTO) {
            if (linkState == NetworkInterface::UP) {
                if (ringPort == primaryRingPort) {
                    setPortState(primaryRingPort, MrpInterfaceData::FORWARDING);
                    testRingReq(defaultTestInterval);
                    EV_DETAIL << "Switching State from AC_STAT1 to PRM_UP" << EV_ENDL;
                    currentState = PRM_UP;
                    currentRingState = OPEN;
                    emit(ringStateChangedSignal, currentRingState);
                    break;
                } else if (ringPort == secondaryRingPort) {
                    toggleRingPorts();
                    setPortState(primaryRingPort, MrpInterfaceData::FORWARDING);
                    testRingReq(defaultTestInterval);
                    EV_DETAIL << "Switching State from AC_STAT1 to PRM_UP" << EV_ENDL;
                    currentState = PRM_UP;
                    currentRingState = OPEN;
                    emit(ringStateChangedSignal, currentRingState);
                    break;
                }
            }
            //all other cases: ignore
            break;
        }
        //all other roles: ignore
        break;
    case PRM_UP:
        if (ringPort == primaryRingPort
                && linkState == NetworkInterface::DOWN) {
            cancelEvent(testTimer);
            setPortState(primaryRingPort, MrpInterfaceData::BLOCKED);
            currentState = AC_STAT1;
            currentRingState = OPEN;
            emit(ringStateChangedSignal, currentRingState);
            EV_DETAIL << "Switching State from PRM_UP to AC_STAT1" << EV_FIELD(currentState) << EV_ENDL;
            break;
        }
        if (ringPort == secondaryRingPort
                && linkState == NetworkInterface::UP) {
            testMaxRetransmissionCount = testMonitoringCount - 1;
            testRetransmissionCount = 0;
            noTopologyChange = true;
            testRingReq(defaultTestInterval);
            currentState = CHK_RC;
            currentRingState = CLOSED;
            emit(ringStateChangedSignal, currentRingState);
            EV_DETAIL << "Switching State from PRM_UP to CHK_RC" << EV_FIELD(currentState) << EV_ENDL;
            break;
        }
        //all other Cases: ignore
        break;
    case CHK_RO:
        if (linkState == NetworkInterface::DOWN) {
            if (ringPort == primaryRingPort) {
                toggleRingPorts();
                setPortState(secondaryRingPort, MrpInterfaceData::BLOCKED);
                testRingReq(defaultTestInterval);
                topologyChangeReq(topologyChangeInterval);
                currentState = PRM_UP;
                currentRingState = OPEN;
                emit(ringStateChangedSignal, currentRingState);
                EV_DETAIL << "Switching State from CHK_RO to PRM_UP" << EV_FIELD(currentState) << EV_ENDL;
                break;
            }
            if (ringPort == secondaryRingPort) {
                setPortState(secondaryRingPort, MrpInterfaceData::BLOCKED);
                currentState = PRM_UP;
                currentRingState = OPEN;
                emit(ringStateChangedSignal, currentRingState);
                EV_DETAIL << "Switching State from CHK_RO to PRM_UP" << EV_FIELD(currentState) << EV_ENDL;
                break;
            }
        }
        //all other cases: ignore
        break;
    case CHK_RC:
        if (linkState == NetworkInterface::DOWN) {
            if (ringPort == primaryRingPort) {
                toggleRingPorts();
                testRingReq(defaultTestInterval);
                topologyChangeReq(topologyChangeInterval);
                currentState = PRM_UP;
                currentRingState = OPEN;
                emit(ringStateChangedSignal, currentRingState);
                EV_DETAIL << "Switching State from CHK_RC to PRM_UP" << EV_FIELD(currentState) << EV_ENDL;
                break;
            }
            if (ringPort == secondaryRingPort) {
                currentState = PRM_UP;
                currentRingState = OPEN;
                emit(ringStateChangedSignal, currentRingState);
                EV_DETAIL << "Switching State from CHK_RC to PRM_UP" << EV_FIELD(currentState) << EV_ENDL;
            }
        }
        //all other Cases: ignore
        break;
    case DE_IDLE:
        if (ringPort == secondaryRingPort
                && linkState == NetworkInterface::UP) {
            linkChangeReq(primaryRingPort, NetworkInterface::UP);
            currentState = PT;
            EV_DETAIL << "Switching State from DE_IDLE to PT" << EV_FIELD(currentState) << EV_ENDL;
        }
        if (ringPort == primaryRingPort
                && linkState == NetworkInterface::DOWN) {
            setPortState(primaryRingPort, MrpInterfaceData::BLOCKED);
            currentState = AC_STAT1;
            EV_DETAIL << "Switching State from DE_IDLE to AC_STAT1" << EV_FIELD(currentState) << EV_ENDL;
        }
        //all other cases: ignore
        break;
    case PT:
        if (linkState == NetworkInterface::DOWN) {
            if (ringPort == secondaryRingPort) {
                cancelEvent(linkUpTimer);
                setPortState(secondaryRingPort, MrpInterfaceData::BLOCKED);
                linkChangeReq(primaryRingPort, NetworkInterface::DOWN);
                currentState = DE;
                EV_DETAIL << "Switching State from PT to DE" << EV_FIELD(currentState) << EV_ENDL;
                break;
            }
            if (ringPort == primaryRingPort) {
                cancelEvent(linkUpTimer);
                toggleRingPorts();
                setPortState(primaryRingPort, MrpInterfaceData::FORWARDING);
                setPortState(secondaryRingPort, MrpInterfaceData::BLOCKED);
                linkChangeReq(primaryRingPort, NetworkInterface::DOWN);
                currentState = DE;
                EV_DETAIL << "Switching State from PT to DE" << EV_FIELD(currentState) << EV_ENDL;
                break;
            }
        }
        //all other cases: ignore
        break;
    case DE:
        if (ringPort == secondaryRingPort
                && linkState == NetworkInterface::UP) {
            cancelEvent(linkDownTimer);
            linkChangeReq(primaryRingPort, NetworkInterface::UP);
            currentState = PT;
            EV_DETAIL << "Switching State from DE to PT" << EV_FIELD(currentState) << EV_ENDL;
            break;
        }
        if (ringPort == primaryRingPort
                && linkState == NetworkInterface::DOWN) {
            linkChangeCount = linkMaxChange;
            setPortState(primaryRingPort, MrpInterfaceData::BLOCKED);
            currentState = AC_STAT1;
            EV_DETAIL << "Switching State from DE to AC_STAT1" << EV_FIELD(currentState) << EV_ENDL;
        }
        //all other cases: ignore
        break;
    case PT_IDLE:
        if (linkState == NetworkInterface::DOWN) {
            if (ringPort == secondaryRingPort) {
                setPortState(secondaryRingPort, MrpInterfaceData::BLOCKED);
                linkChangeReq(primaryRingPort, NetworkInterface::DOWN);
                currentState = DE;
                EV_DETAIL << "Switching State from PT_IDLE to DE" << EV_FIELD(currentState) << EV_ENDL;
                break;
            }
            if (ringPort == primaryRingPort) {
                primaryRingPort = secondaryRingPort;
                secondaryRingPort = ringPort;
                setPortState(secondaryRingPort, MrpInterfaceData::BLOCKED);
                linkChangeReq(primaryRingPort, NetworkInterface::DOWN);
                currentState = DE;
                EV_DETAIL << "Switching State from PT_IDLE to DE" << EV_FIELD(currentState) << EV_ENDL;
                break;
            }
        }
        //all other cases: ignore
        break;
    default:
        throw cRuntimeError("Unknown Node-State");
    }
}

void Mrp::interconnTopologyChangeInd(MacAddress sourceAddress, double time, uint16_t inID, int ringPort, Packet *packet) {
    switch (currentState) {
    case POWER_ON:
    case AC_STAT1:
    case DE_IDLE:
    case PT:
    case PT_IDLE:
        delete packet;
        break;
    case PRM_UP:
    case CHK_RC:
        if (!topologyChangeTimer->isScheduled()) {
            topologyChangeReq(time);
        }
        delete packet;
        break;
    case CHK_RO:
        if (!topologyChangeTimer->isScheduled()) {
            topologyChangeReq(time);
            delete packet;
        }
        if (ringPort == primaryRingPort) {
            interconnForwardReq(secondaryRingPort, packet);
        } else if (ringPort == secondaryRingPort) {
            interconnForwardReq(primaryRingPort, packet);
        }
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void Mrp::interconnLinkChangeInd(uint16_t inID, uint16_t linkstate, int ringPort, Packet *packet) {
    switch (currentState) {
    case POWER_ON:
    case AC_STAT1:
    case DE_IDLE:
    case PT:
    case PT_IDLE:
    case PRM_UP:
    case CHK_RC:
        delete packet;
        break;
    case CHK_RO:
        if (ringPort == primaryRingPort) {
            interconnForwardReq(secondaryRingPort, packet);
        } else if (ringPort == secondaryRingPort) {
            interconnForwardReq(primaryRingPort, packet);
        }
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void Mrp::interconnLinkStatusPollInd(uint16_t inID, int ringPort, Packet *packet) {
    switch (currentState) {
    case POWER_ON:
    case AC_STAT1:
    case DE_IDLE:
    case PT:
    case PT_IDLE:
    case PRM_UP:
    case CHK_RC:
        delete packet;
        break;
    case CHK_RO:
        if (ringPort == primaryRingPort) {
            interconnForwardReq(secondaryRingPort, packet);
        } else if (ringPort == secondaryRingPort) {
            interconnForwardReq(primaryRingPort, packet);
        }
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void Mrp::interconnTestInd(MacAddress sourceAddress, int ringPort, uint16_t InID, Packet *packet) {
    switch (currentState) {
    case POWER_ON:
    case AC_STAT1:
    case DE_IDLE:
    case PT:
    case PT_IDLE:
    case PRM_UP:
    case CHK_RC:
        delete packet;
        break;
    case CHK_RO:
        if (ringPort == primaryRingPort) {
            interconnForwardReq(secondaryRingPort, packet);
        } else if (ringPort == secondaryRingPort) {
            interconnForwardReq(primaryRingPort, packet);
        }
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void Mrp::interconnForwardReq(int ringPort, Packet *packet) {
    auto macAddressInd = packet->findTag<MacAddressInd>();
    auto sourceAddress = macAddressInd->getSrcAddress();
    auto destinationAddress = macAddressInd->getDestAddress();
    packet->trim();
    packet->clearTags();
    sendFrameReq(ringPort, destinationAddress, sourceAddress, priority, MRP_LT, packet);
}

void Mrp::sendFrameReq(int portId, const MacAddress &destinationAddress, const MacAddress &sourceAddress, int prio, uint16_t lt, Packet *mrpPDU) {
    mrpPDU->addTag<InterfaceReq>()->setInterfaceId(portId);
    mrpPDU->addTag<PacketProtocolTag>()->setProtocol(&Protocol::mrp);
    mrpPDU->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ieee8022llc);
    auto macAddressReq = mrpPDU->addTag<MacAddressReq>();
    macAddressReq->setSrcAddress(sourceAddress);
    macAddressReq->setDestAddress(destinationAddress);
    EV_INFO << "Sending packet down" << EV_FIELD(mrpPDU) << EV_FIELD(destinationAddress) << EV_ENDL;
    send(mrpPDU, "relayOut");
}

void Mrp::sendCCM(int portId, Packet *ccm) {
    ccm->addTag<InterfaceReq>()->setInterfaceId(portId);
    ccm->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ieee8021qCFM);
    ccm->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ieee8022llc);
    auto macAddressReq = ccm->addTag<MacAddressReq>();
    macAddressReq->setSrcAddress(getPortNetworkInterface(portId)->getMacAddress());
    macAddressReq->setDestAddress(ccmMulticastAddress);
    EV_INFO << "Sending packet down" << EV_FIELD(ccm) << EV_ENDL;
    send(ccm, "relayOut");
}

void Mrp::handleStartOperation(LifecycleOperation *operation) {
    //start();
}

void Mrp::handleStopOperation(LifecycleOperation *operation) {
    stop();
}

void Mrp::handleCrashOperation(LifecycleOperation *operation) {
    stop();
}

void Mrp::colorLink(NetworkInterface *ie, bool forwarding) const {
    if (visualize) {
        cGate *inGate = switchModule->gate(ie->getNodeInputGateId());
        cGate *outGate = switchModule->gate(ie->getNodeOutputGateId());
        cGate *outGateNext = outGate ? outGate->getNextGate() : nullptr;
        cGate *outGatePrev = outGate ? outGate->getPreviousGate() : nullptr;
        cGate *inGatePrev = inGate ? inGate->getPreviousGate() : nullptr;
        cGate *inGatePrev2 = inGatePrev ? inGatePrev->getPreviousGate() : nullptr;

        // TODO The Gate::getDisplayString() has a side effect: create a channel when gate currently not connected.
        //      Should check the channel existing with Gate::getChannel() before use the Gate::getDisplayString().
        if (outGate && inGate && inGatePrev && outGateNext && outGatePrev && inGatePrev2) {
            if (forwarding) {
                outGatePrev->getDisplayString().setTagArg("ls", 0, ENABLED_LINK_COLOR);
                inGate->getDisplayString().setTagArg("ls", 0, ENABLED_LINK_COLOR);
            } else {
                outGatePrev->getDisplayString().setTagArg("ls", 0, DISABLED_LINK_COLOR);
                inGate->getDisplayString().setTagArg("ls", 0, DISABLED_LINK_COLOR);
            }

            if ((!inGatePrev2->getDisplayString().containsTag("ls")
                    || strcmp(inGatePrev2->getDisplayString().getTagArg("ls", 0), ENABLED_LINK_COLOR) == 0)
                    && forwarding) {
                outGate->getDisplayString().setTagArg("ls", 0, ENABLED_LINK_COLOR);
                inGatePrev->getDisplayString().setTagArg("ls", 0, ENABLED_LINK_COLOR);
            } else {
                outGate->getDisplayString().setTagArg("ls", 0, DISABLED_LINK_COLOR);
                inGatePrev->getDisplayString().setTagArg("ls", 0, DISABLED_LINK_COLOR);
            }
        }
    }
}

void Mrp::refreshDisplay() const {
    if (visualize) {
        for (unsigned int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
            NetworkInterface *ie = interfaceTable->getInterface(i);
            cModule *nicModule = ie;
            if (isUp()) {
                const MrpInterfaceData *port = getPortInterfaceData(ie->getInterfaceId());
                // color link
                colorLink(ie, isUp() && (port->getState() == MrpInterfaceData::FORWARDING));
                // label ethernet interface with port status and role
                if (nicModule != nullptr) {
                    char buf[32];
                    sprintf(buf, "%s\n%s", port->getRoleName(), port->getStateName());
                    nicModule->getDisplayString().setTagArg("t", 0, buf);
                }
            } else {
                // color link
                colorLink(ie, false);
                // label ethernet interface with port status and role
                if (nicModule != nullptr) {
                    nicModule->getDisplayString().setTagArg("t", 0, "");
                }
            }
        }
    }
}

} // namespace inet

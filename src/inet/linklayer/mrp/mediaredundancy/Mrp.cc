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
#include "inet/linklayer/mrp/common/MrpPdu_m.h"
#include "inet/linklayer/mrp/common/ContinuityCheckMessage_m.h"
#include "inet/linklayer/mrp/common/MrpRelay.h"
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
        managerPrio = static_cast<MrpPriority>(par("mrpPriority").intValue());
        nonBlockingMRC = par("nonBlockingMRC");
        reactOnLinkChange = par("reactOnLinkChange");
        checkMediaRedundancy = par("checkMediaRedundancy");
        noTopologyChange = par("noTopologyChange");
        //client variables
        blockedStateSupported = par("blockedStateSupported");

        //signals
        linkChangeSignal = registerSignal("LinkChangeSignal");
        topologyChangeSignal = registerSignal("TopologyChangeSignal");
        testSignal = registerSignal("TestSignal");
        continuityCheckSignal = registerSignal("ContinuityCheckSignal");
        receivedChangeSignal = registerSignal("ReceivedChangeSignal");
        receivedTestSignal = registerSignal("ReceivedTestSignal");
        receivedContinuityCheckSignal = registerSignal("ReceivedContinuityCheckSignal");
        ringStateChangedSignal = registerSignal("RingStateChangedSignal");
        portStateChangedSignal = registerSignal("PortStateChangedSignal");
        clearFDBSignal = registerSignal("ClearFDBSignal");
        switchModule->subscribe(interfaceStateChangedSignal, this);

    }
    if (stage == INITSTAGE_LINK_LAYER) { // "auto" MAC addresses assignment takes place in stage 0
        registerProtocol(Protocol::mrp, gate("relayOut"), gate("relayIn"), nullptr, nullptr);
        registerProtocol(Protocol::ieee8021qCFM, gate("relayOut"), gate("relayIn"), nullptr, nullptr);
        initPortTable();
        //set interface and change Port-Indexes to Port-IDs
        setRingInterfaces(primaryRingPort, secondaryRingPort);
        sourceAddress = relay->getBridgeAddress();
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

void Mrp::read() {
    //TODO
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
    managerPrio = DEFAULT;
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
    managerPrio = MRADEFAULT;
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

void Mrp::clearFDB(double Time) {
    if (!fdbClearTimer->isScheduled())
        scheduleAt(simTime() + SimTime(Time, SIMTIME_MS), fdbClearTimer);
    else if (fdbClearTimer->getArrivalTime() > (simTime() + SimTime(Time, SIMTIME_MS))) {
        cancelEvent(fdbClearTimer);
        scheduleAt(simTime() + SimTime(Time, SIMTIME_MS), fdbClearTimer);
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
            ProcessDelayTimer *DelayTimer = new ProcessDelayTimer("DelayTimer");
            DelayTimer->setPort(interface->getInterfaceId());
            DelayTimer->setField(field);
            DelayTimer->setKind(0);
            if (field == NetworkInterface::F_STATE
                    || field == NetworkInterface::F_CARRIER) {
                if (interface->isUp() && interface->hasCarrier())
                    linkDetectionDelay = SimTime(1, SIMTIME_US); //linkUP is handled faster than linkDown
                else
                    linkDetectionDelay = SimTime(par("linkDetectionDelay").doubleValue(), SIMTIME_MS);
                if (linkUpHysteresisTimer->isScheduled())
                    cancelEvent(linkUpHysteresisTimer);
                scheduleAt(simTime() + linkDetectionDelay, linkUpHysteresisTimer);
                scheduleAt(simTime() + linkDetectionDelay, DelayTimer);
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

void Mrp::handleMrpPDU(Packet* Packet) {
    auto interfaceInd = Packet->findTag<InterfaceInd>();
    auto macAddressInd = Packet->findTag<MacAddressInd>();
    auto SourceAddress = macAddressInd->getSrcAddress();
    auto DestinationAddress = macAddressInd->getDestAddress();
    int RingPort = interfaceInd->getInterfaceId();
    auto incomingInterface = interfaceTable->getInterfaceById(RingPort);
    //unsigned int vlanId = 0;
    //if (auto vlanInd = Packet->findTag<VlanInd>())
    //    vlanId = vlanInd->getVlanId();

    auto version = Packet->peekAtFront<MrpVersionField>();
    auto offset = version->getChunkLength();
    auto firstTLV = Packet->peekDataAt<TlvHeader>(offset);
    offset = offset + B(firstTLV->getHeaderLength()) + B(2);
    auto commonTLV = Packet->peekDataAt<CommonHeader>(offset);
    auto sequence = commonTLV->getSequenceID();
    bool ringID = false;
    if (commonTLV->getUuid0() == domainID.uuid0
            && commonTLV->getUuid1() == domainID.uuid1)
        ringID = true;
    offset = offset + commonTLV->getChunkLength();

    switch (firstTLV->getHeaderType()) {
    case TEST: {
        EV_DETAIL << "Received Test-Frame" << EV_ENDL;
        auto testTLV = dynamicPtrCast<const TestFrame>(firstTLV);
        if (ringID) {
            if (testTLV->getSa() == sourceAddress) {
                auto ringTime = simTime().inUnit(SIMTIME_MS) - testTLV->getTimeStamp();
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
            testRingInd(RingPort, testTLV->getSa(), static_cast<MrpPriority>(testTLV->getPrio()));
        } else {
            EV_DETAIL << "Received packet from other Mrp-Domain"
                             << EV_FIELD(incomingInterface) << EV_FIELD(Packet)
                             << EV_ENDL;
        }
        break;
    }
    case TOPOLOGYCHANGE: {
        EV_DETAIL << "Received TopologyChange-Frame" << EV_ENDL;
        auto topologyTLV = dynamicPtrCast<const TopologyChangeFrame>(firstTLV);
        if (ringID) {
            if (sequence > lastTopologyId) {
                topologyChangeInd(topologyTLV->getSa(), topologyTLV->getInterval());
                emit(receivedChangeSignal, topologyTLV->getInterval());
            } else {
                EV_DETAIL << "Received same Frame already" << EV_ENDL;
                delete Packet;
            }
        } else {
            EV_DETAIL << "Received packet from other Mrp-Domain"
                             << EV_FIELD(incomingInterface) << EV_FIELD(Packet)
                             << EV_ENDL;
        }
        break;
    }
    case LINKDOWN:
    case LINKUP: {
        EV_DETAIL << "Received LinkChange-Frame" << EV_ENDL;
        auto linkTLV = dynamicPtrCast<const LinkChangeFrame>(firstTLV);
        if (ringID) {
            linkChangeInd(linkTLV->getPortRole(), linkTLV->getBlocked());
            emit(receivedChangeSignal, linkTLV->getInterval());
        } else {
            EV_DETAIL << "Received packet from other Mrp-Domain"
                             << EV_FIELD(incomingInterface) << EV_FIELD(Packet)
                             << EV_ENDL;
        }
        break;
    }
    case OPTION: {
        EV_DETAIL << "Received Option-Frame" << EV_ENDL;
        if (ringID) {
            auto optionTLV = dynamicPtrCast<const OptionHeader>(firstTLV);
            b subOffset = version->getChunkLength() + optionTLV->getChunkLength();
            //handle if manufactorerData is present
            if (optionTLV->getOuiType() != MrpOuiType::IEC
                    && (optionTLV->getEd1Type() == 0x00
                            || optionTLV->getEd1Type() == 0x04)) {
                if (optionTLV->getEd1Type() == 0x00) {
                    auto dataChunk = Packet->peekDataAt<FieldsChunk>(subOffset);
                    subOffset = subOffset + B(Ed1DataLength::LENGTH0);
                } else if (optionTLV->getEd1Type() == 0x04) {
                    auto dataChunk = Packet->peekDataAt<FieldsChunk>(subOffset);
                    subOffset = subOffset + B(Ed1DataLength::LENGTH4);
                }
            }

            //handle suboption2 if present
            if ((optionTLV->getOuiType() == MrpOuiType::IEC
                    && optionTLV->getHeaderLength() > 4)
                    || (optionTLV->getEd1Type() == 0x00
                            && optionTLV->getHeaderLength()
                                    > (4 + Ed1DataLength::LENGTH0))
                    || (optionTLV->getEd1Type() == 0x04
                            && optionTLV->getHeaderLength()
                                    > (4 + Ed1DataLength::LENGTH4))) {
                auto subTLV = Packet->peekDataAt<SubTlvHeader>(subOffset);
                switch (subTLV->getSubType()) {
                case RESERVED: {
                    auto subOptionTLV = dynamicPtrCast<const ManufacturerFktHeader>(subTLV);
                    //not implemented
                    break;
                }
                case TEST_MGR_NACK: {
                    if (expectedRole == MANAGER_AUTO) {
                        auto subOptionTLV = dynamicPtrCast<const SubTlvTestFrame>(subTLV);
                        testMgrNackInd(RingPort, subOptionTLV->getSa(), static_cast<MrpPriority>(subOptionTLV->getPrio()), subOptionTLV->getOtherMRMSa());
                    }
                    break;
                }
                case TEST_PROPAGATE: {
                    if (expectedRole == MANAGER_AUTO) {
                        auto subOptionTLV = dynamicPtrCast<const SubTlvTestFrame>(subTLV);
                        testPropagateInd(RingPort, subOptionTLV->getSa(), static_cast<MrpPriority>(subOptionTLV->getPrio()), subOptionTLV->getOtherMRMSa(), static_cast<MrpPriority>(subOptionTLV->getOtherMRMPrio()));
                    }
                    break;
                }
                case AUTOMGR: {
                    //nothing else to do
                    EV_DETAIL << "Received TestFrame from Automanager" << EV_ENDL;
                    break;
                }
                default:
                    throw cRuntimeError("unknown subTLV TYPE: %d", subTLV->getSubType());
                }
            }
        } else {
            EV_DETAIL << "Received packet from other Mrp-Domain" << EV_FIELD(incomingInterface) << EV_FIELD(Packet) << EV_ENDL;
        }
        break;
    }
    case INTEST: {
        EV_DETAIL << "Received inTest-Frame" << EV_ENDL;
        auto inTestTLV = dynamicPtrCast<const InTestFrame>(firstTLV);
        interconnTestInd(inTestTLV->getSa(), RingPort, inTestTLV->getInID(), Packet->dup());
        break;
    }
    case INTOPOLOGYCHANGE: {
        EV_DETAIL << "Received inTopologyChange-Frame" << EV_ENDL;
        auto inTopologyTLV = dynamicPtrCast<const InTopologyChangeFrame>(firstTLV);
        interconnTopologyChangeInd(inTopologyTLV->getSa(), inTopologyTLV->getInterval(), inTopologyTLV->getInID(), RingPort, Packet->dup());
        break;
    }
    case INLINKDOWN:
    case INLINKUP: {
        EV_DETAIL << "Received inLinkChange-Frame" << EV_ENDL;
        auto inLinkTLV = dynamicPtrCast<const InLinkChangeFrame>(firstTLV);
        interconnLinkChangeInd(inLinkTLV->getInID(), inLinkTLV->getLinkInfo(), RingPort, Packet->dup());
        break;
    }
    case INLINKSTATUSPOLL: {
        EV_DETAIL << "Received inLinkStatusPoll" << EV_ENDL;
        auto inLinkStatusTLV = dynamicPtrCast<const InLinkStatusPollFrame>(firstTLV);
        interconnLinkStatusPollInd(inLinkStatusTLV->getInID(), RingPort, Packet->dup());
        break;
    }
    default:
        throw cRuntimeError("unknown TLV TYPE: %d", firstTLV->getHeaderType());
    }

    //addtional Option-Frame
    auto thirdTLV = Packet->peekDataAt<TlvHeader>(offset);
    if (thirdTLV->getHeaderType() != END && ringID) {
        EV_DETAIL << "Received additional Option-Frame" << EV_ENDL;
        auto optionTLV = dynamicPtrCast<const OptionHeader>(thirdTLV);
        b subOffset = offset + optionTLV->getChunkLength();
        //handle if manufactorerData is present
        if (optionTLV->getOuiType() != MrpOuiType::IEC
                && (optionTLV->getEd1Type() == 0x00
                        || optionTLV->getEd1Type() == 0x04)) {
            if (optionTLV->getEd1Type() == 0x00) {
                auto dataChunk = Packet->peekDataAt<FieldsChunk>(subOffset);
                subOffset = subOffset + B(Ed1DataLength::LENGTH0);
            } else if (optionTLV->getEd1Type() == 0x04) {
                auto dataChunk = Packet->peekDataAt<FieldsChunk>(subOffset);
                subOffset = subOffset + B(Ed1DataLength::LENGTH4);
            }
        }

        //handle suboption2 if present
        if ((optionTLV->getOuiType() == MrpOuiType::IEC
                && optionTLV->getHeaderLength() > 4)
                || (optionTLV->getEd1Type() == 0x00
                        && optionTLV->getHeaderLength()
                                > (4 + Ed1DataLength::LENGTH0))
                || (optionTLV->getEd1Type() == 0x04
                        && optionTLV->getHeaderLength()
                                > (4 + Ed1DataLength::LENGTH4))) {
            auto subTLV = Packet->peekDataAt<SubTlvHeader>(subOffset);
            switch (subTLV->getSubType()) {
            case RESERVED: {
                auto subOptionTLV = dynamicPtrCast<const ManufacturerFktHeader>(subTLV);
                //not implemented
                break;
            }
            case TEST_MGR_NACK: {
                if (expectedRole == MANAGER_AUTO) {
                    auto subOptionTLV = dynamicPtrCast<const SubTlvTestFrame>(subTLV);
                    testMgrNackInd(RingPort, subOptionTLV->getSa(), static_cast<MrpPriority>(subOptionTLV->getPrio()), subOptionTLV->getOtherMRMSa());
                }
                break;
            }
            case TEST_PROPAGATE: {
                if (expectedRole == MANAGER_AUTO) {
                    auto subOptionTLV = dynamicPtrCast<const SubTlvTestFrame>(subTLV);
                    testPropagateInd(RingPort, subOptionTLV->getSa(), static_cast<MrpPriority>(subOptionTLV->getPrio()), subOptionTLV->getOtherMRMSa(), static_cast<MrpPriority>(subOptionTLV->getOtherMRMPrio()));
                }
                break;
            }
            case AUTOMGR: {
                //nothing else to do
                EV_DETAIL << "Received TestFrame from Automanager" << EV_ENDL;
                break;
            }
            default:
                throw cRuntimeError("unknown subTLV TYPE: %d", subTLV->getSubType());
            }
        }
        offset = offset + B(thirdTLV->getHeaderLength()) + B(2);
    }
    //auto endTLV=Packet->peekDataAt<TlvHeader>(offset);
    delete Packet;
}

void Mrp::handleContinuityCheckMessage(Packet* Packet) {
    EV_DETAIL << "Handling CCM" << EV_ENDL;
    auto interfaceInd = Packet->getTag<InterfaceInd>();
    auto macAddressInd = Packet->getTag<MacAddressInd>();
    auto SourceAddress = macAddressInd->getSrcAddress();
    int RingPort = interfaceInd->getInterfaceId();
    auto incomingInterface = interfaceTable->getInterfaceById(RingPort);
    auto ccm = Packet->popAtFront<ContinuityCheckMessage>();
    auto portData = getPortInterfaceDataForUpdate(RingPort);
    if (ccm->getEndpointIdentifier() == portData->getCfmEndpointID()) {
        int i = SourceAddress.compareTo(incomingInterface->getMacAddress());
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
            << EV_FIELD(SourceAddress)
            << EV_FIELD(incomingInterface->getMacAddress())
            << EV_ENDL;
    mauTypeChangeInd(RingPort, NetworkInterface::UP);
    emit(receivedContinuityCheckSignal, RingPort);
    delete Packet;
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

bool Mrp::isBetterThanOwnPrio(MrpPriority RemotePrio, MacAddress RemoteAddress) {
    if (RemotePrio < managerPrio)
        return true;
    if (RemotePrio == managerPrio && RemoteAddress < sourceAddress)
        return true;
    return false;
}

bool Mrp::isBetterThanBestPrio(MrpPriority RemotePrio, MacAddress RemoteAddress) {
    if (RemotePrio < hostBestMRMPriority)
        return true;
    if (RemotePrio == hostBestMRMPriority && RemoteAddress < hostBestMRMSourceAddress)
        return true;
    return false;
}

void Mrp::handleContinuityCheckTimer(int RingPort) {
    auto portData = getPortInterfaceDataForUpdate(RingPort);
    EV_DETAIL << "Checktimer:" << EV_FIELD(simTime()) << EV_FIELD(RingPort)
                     << EV_FIELD(portData->getNextUpdate()) << EV_ENDL;
    if (simTime() >= portData->getNextUpdate()) {
        //no Message received within Lifetime
        EV_DETAIL << "Checktimer: Link considered down" << EV_ENDL;
        mauTypeChangeInd(RingPort, NetworkInterface::DOWN);
    }
    setupContinuityCheck(RingPort);
    class ContinuityCheckTimer *checkTimer = new ContinuityCheckTimer("continuityCheckTimer");
    checkTimer->setPort(RingPort);
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

void Mrp::setupContinuityCheck(int RingPort) {
    auto CCM = makeShared<ContinuityCheckMessage>();
    auto portData = getPortInterfaceDataForUpdate(RingPort);
    if (portData->getContinuityCheckInterval() == 3.3) {
        CCM->setFlags(0x00000001);
    } else if (portData->getContinuityCheckInterval() == 10) {
        CCM->setFlags(0x00000002);
    }
    if (RingPort == primaryRingPort) {
        CCM->setSequenceNumber(sequenceCCM1);
        sequenceCCM1++;
    } else if (RingPort == secondaryRingPort) {
        CCM->setSequenceNumber(sequenceCCM2);
        sequenceCCM2++;
    }
    CCM->setEndpointIdentifier(portData->getCfmEndpointID());
    auto name = portData->getCfmName();
    CCM->setMessageName(name.c_str());
    auto packet = new Packet("ContinuityCheck", CCM);
    sendCCM(RingPort, packet);
    emit(continuityCheckSignal, RingPort);
}

void Mrp::testRingReq(double Time) {
    if (addTest)
        cancelEvent(testTimer);
    if (!testTimer->isScheduled()) {
        scheduleAt(simTime() + SimTime(Time, SIMTIME_MS), testTimer);
        setupTestRingReq();
    } else
        EV_DETAIL << "Testtimer already scheduled" << EV_ENDL;
}

void Mrp::topologyChangeReq(double Time) {
    if (Time == 0) {
        clearLocalFDB();
        setupTopologyChangeReq(Time * topologyChangeMaxRepeatCount);
    } else if (!topologyChangeTimer->isScheduled()) {
        scheduleAt(simTime() + SimTime(Time, SIMTIME_MS), topologyChangeTimer);
        setupTopologyChangeReq(Time * topologyChangeMaxRepeatCount);
    } else
        EV_DETAIL << "TopologyChangeTimer already scheduled" << EV_ENDL;

}

void Mrp::linkChangeReq(int RingPort, uint16_t LinkState) {
    if (LinkState == NetworkInterface::DOWN) {
        if (!linkDownTimer->isScheduled()) {
            scheduleAt(simTime() + SimTime(linkDownInterval, SIMTIME_MS), linkDownTimer);
            setupLinkChangeReq(primaryRingPort, NetworkInterface::DOWN, linkChangeCount * linkDownInterval);
            linkChangeCount--;
        }
    } else if (LinkState == NetworkInterface::UP) {
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
    auto Version = makeShared<MrpVersionField>();
    auto TestTLV1 = makeShared<TestFrame>();
    auto TestTLV2 = makeShared<TestFrame>();
    auto CommonTLV = makeShared<CommonHeader>();
    auto EndTLV = makeShared<TlvHeader>();

    auto timestamp = simTime().inUnit(SIMTIME_MS);
    auto lastTestFrameSent = simTime().inUnit(SIMTIME_US);
    testFrameSent.insert( { sequenceID, lastTestFrameSent });

    TestTLV1->setPrio(managerPrio);
    TestTLV1->setSa(sourceAddress);
    TestTLV1->setPortRole(MrpInterfaceData::PRIMARY);
    TestTLV1->setRingState(currentRingState);
    TestTLV1->setTransition(transition);
    TestTLV1->setTimeStamp(timestamp);

    TestTLV2->setPrio(managerPrio);
    TestTLV2->setSa(sourceAddress);
    TestTLV2->setPortRole(MrpInterfaceData::PRIMARY);
    TestTLV2->setRingState(currentRingState);
    TestTLV2->setTransition(transition);
    TestTLV2->setTimeStamp(timestamp);

    CommonTLV->setSequenceID(sequenceID);
    sequenceID++;
    CommonTLV->setUuid0(domainID.uuid0);
    CommonTLV->setUuid1(domainID.uuid1);

    Packet *packet1 = new Packet("mrpTestFrame");
    packet1->insertAtBack(Version);
    packet1->insertAtBack(TestTLV1);
    packet1->insertAtBack(CommonTLV);

    Packet *packet2 = new Packet("mrpTestFrame");
    packet2->insertAtBack(Version);
    packet2->insertAtBack(TestTLV2);
    packet2->insertAtBack(CommonTLV);

    //MRA only
    if (expectedRole == MANAGER_AUTO) {
        auto OptionTLV = makeShared<OptionHeader>();
        auto AutoMgrTLV = makeShared<SubTlvHeader>();
        uint8_t headerLength = OptionTLV->getHeaderLength() + AutoMgrTLV->getSubHeaderLength() + 2;
        OptionTLV->setHeaderLength(headerLength);
        packet1->insertAtBack(OptionTLV);
        packet1->insertAtBack(AutoMgrTLV);
        packet2->insertAtBack(OptionTLV);
        packet2->insertAtBack(AutoMgrTLV);
    }
    packet1->insertAtBack(EndTLV);
    MacAddress SourceAddress1 = getPortNetworkInterface(primaryRingPort)->getMacAddress();
    sendFrameReq(primaryRingPort, static_cast<MacAddress>(MC_TEST), SourceAddress1, priority, MRP_LT, packet1);

    packet2->insertAtBack(EndTLV);
    MacAddress SourceAddress2 = getPortNetworkInterface(secondaryRingPort)->getMacAddress();
    sendFrameReq(secondaryRingPort, static_cast<MacAddress>(MC_TEST), SourceAddress2, priority, MRP_LT, packet2);
    emit(testSignal, lastTestFrameSent);
}

void Mrp::setupTopologyChangeReq(uint32_t Interval) {
    //Create MRP-PDU according MRP_TopologyChange
    auto Version = makeShared<MrpVersionField>();
    auto TopologyChangeTLV = makeShared<TopologyChangeFrame>();
    auto TopologyChangeTLV2 = makeShared<TopologyChangeFrame>();
    auto CommonTLV = makeShared<CommonHeader>();
    auto EndTLV = makeShared<TlvHeader>();

    TopologyChangeTLV->setPrio(managerPrio);
    TopologyChangeTLV->setSa(sourceAddress);
    TopologyChangeTLV->setPortRole(MrpInterfaceData::PRIMARY);
    TopologyChangeTLV->setInterval(Interval);
    TopologyChangeTLV2->setPrio(managerPrio);
    TopologyChangeTLV2->setPortRole(MrpInterfaceData::SECONDARY);
    TopologyChangeTLV2->setSa(sourceAddress);
    TopologyChangeTLV2->setInterval(Interval);

    CommonTLV->setSequenceID(sequenceID);
    sequenceID++;
    CommonTLV->setUuid0(domainID.uuid0);
    CommonTLV->setUuid1(domainID.uuid1);

    auto packet1 = new Packet("mrpTopologyChange");
    packet1->insertAtBack(Version);
    packet1->insertAtBack(TopologyChangeTLV);
    packet1->insertAtBack(CommonTLV);
    packet1->insertAtBack(EndTLV);
    MacAddress SourceAddress1 = getPortNetworkInterface(primaryRingPort)->getMacAddress();
    sendFrameReq(primaryRingPort, static_cast<MacAddress>(MC_CONTROL), SourceAddress1, priority, MRP_LT, packet1);

    auto packet2 = new Packet("mrpTopologyChange");
    packet2->insertAtBack(Version);
    packet2->insertAtBack(TopologyChangeTLV2);
    packet2->insertAtBack(CommonTLV);
    packet2->insertAtBack(EndTLV);
    MacAddress SourceAddress2 = getPortNetworkInterface(secondaryRingPort)->getMacAddress();
    sendFrameReq(secondaryRingPort, static_cast<MacAddress>(MC_CONTROL), SourceAddress2, priority, MRP_LT, packet2);
    emit(topologyChangeSignal, Interval);
}

void Mrp::setupLinkChangeReq(int RingPort, uint16_t LinkState, double Time) {
    //Create MRP-PDU according MRP_LinkChange
    auto Version = makeShared<MrpVersionField>();
    auto LinkChangeTLV = makeShared<LinkChangeFrame>();
    auto CommonTLV = makeShared<CommonHeader>();
    auto EndTLV = makeShared<TlvHeader>();

    if (LinkState == NetworkInterface::UP) {
        LinkChangeTLV->setHeaderType(LINKUP);
    } else if (LinkState == NetworkInterface::DOWN) {
        LinkChangeTLV->setHeaderType(LINKDOWN);
    } else {
        throw cRuntimeError("Unknown LinkState in linkChangeRequest");
    }
    LinkChangeTLV->setSa(sourceAddress);
    LinkChangeTLV->setPortRole(getPortRole(RingPort));
    LinkChangeTLV->setInterval(Time);
    LinkChangeTLV->setBlocked(LinkState);

    CommonTLV->setSequenceID(sequenceID);
    sequenceID++;
    CommonTLV->setUuid0(domainID.uuid0);
    CommonTLV->setUuid1(domainID.uuid1);

    auto packet1 = new Packet("mrpLinkChange");
    packet1->insertAtBack(Version);
    packet1->insertAtBack(LinkChangeTLV);
    packet1->insertAtBack(CommonTLV);
    packet1->insertAtBack(EndTLV);
    MacAddress SourceAddress1 = getPortNetworkInterface(RingPort)->getMacAddress();
    sendFrameReq(RingPort, static_cast<MacAddress>(MC_CONTROL), SourceAddress1, priority, MRP_LT, packet1);
    emit(linkChangeSignal, Time);
}

void Mrp::testMgrNackReq(int RingPort, MrpPriority ManagerPrio, MacAddress SourceAddress) {
    //Create MRP-PDU according MRP_Option and Suboption2 MRP-TestMgrNack
    auto Version = makeShared<MrpVersionField>();
    auto OptionTLV = makeShared<OptionHeader>();
    auto TestMgrTLV = makeShared<SubTlvTestFrame>();
    auto CommonTLV = makeShared<CommonHeader>();
    auto EndTLV = makeShared<TlvHeader>();

    TestMgrTLV->setSubType(SubTlvHeaderType::TEST_MGR_NACK);
    TestMgrTLV->setPrio(managerPrio);
    TestMgrTLV->setSa(sourceAddress);
    TestMgrTLV->setOtherMRMPrio(0x00);
    TestMgrTLV->setOtherMRMSa(SourceAddress);

    OptionTLV->setHeaderLength(OptionTLV->getHeaderLength() + TestMgrTLV->getSubHeaderLength() + 2);

    CommonTLV->setSequenceID(sequenceID);
    sequenceID++;
    CommonTLV->setUuid0(domainID.uuid0);
    CommonTLV->setUuid1(domainID.uuid1);

    Packet *packet1 = new Packet("mrpTestMgrNackFrame");
    packet1->insertAtBack(Version);
    packet1->insertAtBack(OptionTLV);
    packet1->insertAtBack(TestMgrTLV);
    packet1->insertAtBack(CommonTLV);
    packet1->insertAtBack(EndTLV);

    //Standard request sending out both packets, but defines a handshake.
    //sending the answer back would be enough
    //Code will remain comment out for easy optimization
    //MacAddress SourceAddress1 = getPortNetworkInterface(RingPort)->getMacAddress();
    //sendFrameReq(RingPort, static_cast<MacAddress>(MC_TEST), SourceAddress1, priority, MRP_LT , packet1);

    Packet *packet2 = new Packet("mrpTestMgrNackFrame");
    packet2->insertAtBack(Version);
    packet2->insertAtBack(OptionTLV);
    packet2->insertAtBack(TestMgrTLV);
    packet2->insertAtBack(CommonTLV);
    packet2->insertAtBack(EndTLV);

    MacAddress SourceAddress1 = getPortNetworkInterface(primaryRingPort)->getMacAddress();
    sendFrameReq(primaryRingPort, static_cast<MacAddress>(MC_TEST), SourceAddress1, priority, MRP_LT, packet1);
    MacAddress SourceAddress2 = getPortNetworkInterface(secondaryRingPort)->getMacAddress();
    sendFrameReq(secondaryRingPort, static_cast<MacAddress>(MC_TEST), SourceAddress2, priority, MRP_LT, packet2);
}

void Mrp::testPropagateReq(int RingPort, MrpPriority ManagerPrio, MacAddress SourceAddress) {
    //Create MRP-PDU according MRP_Option and Suboption2 MRP-TestPropagate
    auto Version = makeShared<MrpVersionField>();
    auto OptionTLV = makeShared<OptionHeader>();
    auto TestMgrTLV = makeShared<SubTlvTestFrame>();
    auto CommonTLV = makeShared<CommonHeader>();
    auto EndTLV = makeShared<TlvHeader>();

    TestMgrTLV->setPrio(managerPrio);
    TestMgrTLV->setSa(sourceAddress);
    TestMgrTLV->setOtherMRMPrio(ManagerPrio);
    TestMgrTLV->setOtherMRMSa(SourceAddress);

    OptionTLV->setHeaderLength(OptionTLV->getHeaderLength() + TestMgrTLV->getSubHeaderLength() + 2);

    CommonTLV->setSequenceID(sequenceID);
    sequenceID++;
    CommonTLV->setUuid0(domainID.uuid0);
    CommonTLV->setUuid1(domainID.uuid1);

    Packet *packet1 = new Packet("mrpTestPropagateFrame");
    packet1->insertAtBack(Version);
    packet1->insertAtBack(OptionTLV);
    packet1->insertAtBack(TestMgrTLV);
    packet1->insertAtBack(CommonTLV);
    packet1->insertAtBack(EndTLV);

    //Standard request sending out both packets, but defines a handshake.
    //sending the answer back would be enough
    //Code will remain comment out for easy optimization
    //MacAddress SourceAddress1 = getPortNetworkInterface(RingPort)->getMacAddress();
    //sendFrameReq(RingPort, static_cast<MacAddress>(MC_TEST), SourceAddress1, priority, MRP_LT , packet1);

    Packet *packet2 = new Packet("mrpTestPropagateFrame");
    packet2->insertAtBack(Version);
    packet2->insertAtBack(OptionTLV);
    packet2->insertAtBack(TestMgrTLV);
    packet2->insertAtBack(CommonTLV);
    packet2->insertAtBack(EndTLV);

    MacAddress SourceAddress1 = getPortNetworkInterface(primaryRingPort)->getMacAddress();
    sendFrameReq(primaryRingPort, static_cast<MacAddress>(MC_TEST), SourceAddress1, priority, MRP_LT, packet1);
    MacAddress SourceAddress2 = getPortNetworkInterface(secondaryRingPort)->getMacAddress();
    sendFrameReq(secondaryRingPort, static_cast<MacAddress>(MC_TEST), SourceAddress2, priority, MRP_LT, packet2);
}

void Mrp::testRingInd(int RingPort, MacAddress SourceAddress, MrpPriority ManagerPrio) {
    switch (currentState) {
    case POWER_ON:
    case AC_STAT1:
        break;
    case PRM_UP:
        if (SourceAddress == sourceAddress) {
            testMaxRetransmissionCount = testMonitoringCount - 1;
            testRetransmissionCount = 0;
            noTopologyChange = false;
            testRingReq(defaultTestInterval);
            currentState = CHK_RC;
            EV_DETAIL << "Switching State from PRM_UP to CHK_RC" << EV_FIELD(currentState) << EV_ENDL;
            currentRingState = CLOSED;
            emit(ringStateChangedSignal, currentRingState);
        } else if (expectedRole == MANAGER_AUTO
                && !isBetterThanOwnPrio(ManagerPrio, SourceAddress)) {
            testMgrNackReq(RingPort, ManagerPrio, SourceAddress);
        }
        //all other cases: ignore
        break;
    case CHK_RO:
        if (SourceAddress == sourceAddress) {
            setPortState(secondaryRingPort, MrpInterfaceData::BLOCKED);
            testMaxRetransmissionCount = testMonitoringCount - 1;
            testRetransmissionCount = 0;
            noTopologyChange = false;
            testRingReq(defaultTestInterval);
            if (!reactOnLinkChange) {
                topologyChangeReq(topologyChangeInterval);
            } else {
                double Time = 0;
                topologyChangeReq(Time);
            }
            currentState = CHK_RC;
            EV_DETAIL << "Switching State from CHK_RO to CHK_RC" << EV_FIELD(currentState) << EV_ENDL;
            currentRingState = CLOSED;
            emit(ringStateChangedSignal, currentRingState);
        } else if (expectedRole == MANAGER_AUTO
                && !isBetterThanOwnPrio(ManagerPrio, SourceAddress)) {
            testMgrNackReq(RingPort, ManagerPrio, SourceAddress);
        }
        //all other cases: ignore
        break;
    case CHK_RC:
        if (SourceAddress == sourceAddress) {
            testMaxRetransmissionCount = testMonitoringCount - 1;
            testRetransmissionCount = 0;
            noTopologyChange = false;
        } else if (expectedRole == MANAGER_AUTO
                && !isBetterThanOwnPrio(ManagerPrio, SourceAddress)) {
            testMgrNackReq(RingPort, ManagerPrio, SourceAddress);
        }
        break;
    case DE:
    case DE_IDLE:
    case PT:
    case PT_IDLE:
        if (expectedRole == MANAGER_AUTO && SourceAddress != sourceAddress
                && SourceAddress == hostBestMRMSourceAddress) {
            if (ManagerPrio < managerPrio
                    || (ManagerPrio == managerPrio
                            && SourceAddress < sourceAddress)) {
                monNReturn = 0;
            }
            hostBestMRMPriority = ManagerPrio;
        }
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void Mrp::topologyChangeInd(MacAddress SourceAddress, double Time) {
    switch (currentState) {
    case POWER_ON:
    case AC_STAT1:
        break;
    case PRM_UP:
    case CHK_RO:
    case CHK_RC:
        if (SourceAddress != sourceAddress) {
            clearFDB(Time);
        }
        break;
    case PT:
        linkChangeCount = linkMaxChange;
        cancelEvent(linkUpTimer);
        setPortState(secondaryRingPort, MrpInterfaceData::FORWARDING);
        clearFDB(Time);
        currentState = PT_IDLE;
        EV_DETAIL << "Switching State from PT to PT_IDLE" << EV_FIELD(currentState) << EV_ENDL;
        break;
    case DE:
        linkChangeCount = linkMaxChange;
        cancelEvent(linkDownTimer);
        clearFDB(Time);
        currentState = DE_IDLE;
        EV_DETAIL << "Switching State from DE to DE_IDLE" << EV_FIELD(currentState) << EV_ENDL;
        break;
    case DE_IDLE:
        clearFDB(Time);
        if (expectedRole == MANAGER_AUTO
                && linkUpHysteresisTimer->isScheduled()) {
            setPortState(secondaryRingPort, MrpInterfaceData::FORWARDING);
            currentState = PT_IDLE;
            EV_DETAIL << "Switching State from DE_IDLE to PT_IDLE" << EV_FIELD(currentState) << EV_ENDL;
        }
    case PT_IDLE:
        clearFDB(Time);
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void Mrp::linkChangeInd(uint16_t PortState, uint16_t LinkState) {
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
            } else if (LinkState == NetworkInterface::UP) {
                addTest = true;
                testRingReq(shortTestInterval);
                double Time = 0;
                topologyChangeReq(Time);
                break;
            }
        } else {
            if (!nonBlockingMRC && LinkState == NetworkInterface::UP) { //18
                double Time = 0;
                topologyChangeReq(Time);
            }
            break;
        }
        //all other cases : ignore
        break;
    case CHK_RO:
        if (!addTest) {
            if (LinkState == NetworkInterface::DOWN) {
                addTest = true;
                testRingReq(shortTestInterval);
                break;
            } else if (LinkState == NetworkInterface::UP) {
                if (nonBlockingMRC) {
                    addTest = true;
                    testRingReq(shortTestInterval);
                } else {
                    setPortState(secondaryRingPort, MrpInterfaceData::BLOCKED);
                    testMaxRetransmissionCount = testMonitoringExtendedCount - 1;
                    testRetransmissionCount = 0;
                    addTest = true;
                    testRingReq(shortTestInterval);
                    double Time = 0;
                    topologyChangeReq(Time);
                    currentState = CHK_RC;
                    currentRingState = CLOSED;
                    emit(ringStateChangedSignal, currentRingState);
                    EV_DETAIL << "Switching State from CHK_RO to CHK_RC" << EV_FIELD(currentState) << EV_ENDL;
                }
            }
        } else {
            if (!nonBlockingMRC && LinkState == NetworkInterface::UP) {
                setPortState(secondaryRingPort, MrpInterfaceData::BLOCKED);
                testMaxRetransmissionCount = testMonitoringExtendedCount - 1;
                testRetransmissionCount = 0;
                testRingReq(defaultTestInterval);
                double Time = 0;
                topologyChangeReq(Time);
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
            if (LinkState == NetworkInterface::DOWN) {
                setPortState(secondaryRingPort, MrpInterfaceData::FORWARDING);
                double Time = 0;
                topologyChangeReq(Time);
                currentState = CHK_RO;
                currentRingState = OPEN;
                emit(ringStateChangedSignal, currentRingState);
                EV_DETAIL << "Switching State from CHK_RC to CHK_RO" << EV_FIELD(currentState) << EV_ENDL;
                break;
            } else if (LinkState == NetworkInterface::UP) {
                if (nonBlockingMRC) {
                    testMaxRetransmissionCount = testMonitoringCount - 1;
                } else {
                    testMaxRetransmissionCount = testMonitoringExtendedCount - 1;
                }
                double Time = 0;
                topologyChangeReq(Time);
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

void Mrp::testMgrNackInd(int RingPort, MacAddress SourceAddress, MrpPriority ManagerPrio, MacAddress BestMRMSourceAddress) {
    switch (currentState) {
    case POWER_ON:
    case AC_STAT1:
    case DE:
    case DE_IDLE:
    case PT:
    case PT_IDLE:
        break;
    case PRM_UP:
        if (expectedRole == MANAGER_AUTO && SourceAddress != sourceAddress
                && BestMRMSourceAddress == sourceAddress) {
            if (isBetterThanBestPrio(ManagerPrio, SourceAddress)) {
                hostBestMRMSourceAddress = SourceAddress;
                hostBestMRMPriority = ManagerPrio;
            }
            cancelEvent(topologyChangeTimer);
            mrcInit();
            testPropagateReq(RingPort, hostBestMRMPriority, hostBestMRMSourceAddress);
            currentState = DE_IDLE;
            EV_DETAIL << "Switching State from PRM_UP to DE_IDLE" << EV_FIELD(currentState) << EV_ENDL;
        }
        break;
    case CHK_RO:
        if (expectedRole == MANAGER_AUTO && SourceAddress != sourceAddress
                && BestMRMSourceAddress == sourceAddress) {
            if (isBetterThanBestPrio(ManagerPrio, SourceAddress)) {
                hostBestMRMSourceAddress = SourceAddress;
                hostBestMRMPriority = ManagerPrio;
            }
            cancelEvent(topologyChangeTimer);
            mrcInit();
            testPropagateReq(RingPort, hostBestMRMPriority, hostBestMRMSourceAddress);
            currentState = PT_IDLE;
            EV_DETAIL << "Switching State from CHK_RO to PT_IDLE" << EV_FIELD(currentState) << EV_ENDL;
        }
        break;
    case CHK_RC:
        if (expectedRole == MANAGER_AUTO && SourceAddress != sourceAddress
                && BestMRMSourceAddress == sourceAddress) {
            if (isBetterThanBestPrio(ManagerPrio, SourceAddress)) {
                hostBestMRMSourceAddress = SourceAddress;
                hostBestMRMPriority = ManagerPrio;
            }
            cancelEvent(topologyChangeTimer);
            mrcInit();
            testPropagateReq(RingPort, hostBestMRMPriority, hostBestMRMSourceAddress);
            setPortState(secondaryRingPort, MrpInterfaceData::FORWARDING);
            currentState = PT_IDLE;
            EV_DETAIL << "Switching State from CHK_RC to PT_IDLE" << EV_FIELD(currentState) << EV_ENDL;
        }
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void Mrp::testPropagateInd(int RingPort, MacAddress SourceAddress, MrpPriority ManagerPrio, MacAddress BestMRMSourceAddress, MrpPriority BestMRMPrio) {
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
        if (expectedRole == MANAGER_AUTO && SourceAddress != sourceAddress
                && SourceAddress == hostBestMRMSourceAddress) {
            hostBestMRMSourceAddress = BestMRMSourceAddress;
            hostBestMRMPriority = BestMRMPrio;
            monNReturn = 0;
        }
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void Mrp::mauTypeChangeInd(int RingPort, uint16_t LinkState) {
    switch (currentState) {
    case POWER_ON:
        //all cases: ignore
        break;
    case AC_STAT1:
        //Client
        if (expectedRole == CLIENT) {
            if (LinkState == NetworkInterface::UP) {
                if (RingPort == primaryRingPort) {
                    setPortState(primaryRingPort, MrpInterfaceData::FORWARDING);
                    EV_DETAIL << "Switching State from AC_STAT1 to DE_IDLE" << EV_ENDL;
                    currentState = DE_IDLE;
                    break;
                } else if (RingPort == secondaryRingPort) {
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
            if (LinkState == NetworkInterface::UP) {
                if (RingPort == primaryRingPort) {
                    setPortState(primaryRingPort, MrpInterfaceData::FORWARDING);
                    testRingReq(defaultTestInterval);
                    EV_DETAIL << "Switching State from AC_STAT1 to PRM_UP" << EV_ENDL;
                    currentState = PRM_UP;
                    currentRingState = OPEN;
                    emit(ringStateChangedSignal, currentRingState);
                    break;
                } else if (RingPort == secondaryRingPort) {
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
        if (RingPort == primaryRingPort
                && LinkState == NetworkInterface::DOWN) {
            cancelEvent(testTimer);
            setPortState(primaryRingPort, MrpInterfaceData::BLOCKED);
            currentState = AC_STAT1;
            currentRingState = OPEN;
            emit(ringStateChangedSignal, currentRingState);
            EV_DETAIL << "Switching State from PRM_UP to AC_STAT1" << EV_FIELD(currentState) << EV_ENDL;
            break;
        }
        if (RingPort == secondaryRingPort
                && LinkState == NetworkInterface::UP) {
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
        if (LinkState == NetworkInterface::DOWN) {
            if (RingPort == primaryRingPort) {
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
            if (RingPort == secondaryRingPort) {
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
        if (LinkState == NetworkInterface::DOWN) {
            if (RingPort == primaryRingPort) {
                toggleRingPorts();
                testRingReq(defaultTestInterval);
                topologyChangeReq(topologyChangeInterval);
                currentState = PRM_UP;
                currentRingState = OPEN;
                emit(ringStateChangedSignal, currentRingState);
                EV_DETAIL << "Switching State from CHK_RC to PRM_UP" << EV_FIELD(currentState) << EV_ENDL;
                break;
            }
            if (RingPort == secondaryRingPort) {
                currentState = PRM_UP;
                currentRingState = OPEN;
                emit(ringStateChangedSignal, currentRingState);
                EV_DETAIL << "Switching State from CHK_RC to PRM_UP" << EV_FIELD(currentState) << EV_ENDL;
            }
        }
        //all other Cases: ignore
        break;
    case DE_IDLE:
        if (RingPort == secondaryRingPort
                && LinkState == NetworkInterface::UP) {
            linkChangeReq(primaryRingPort, NetworkInterface::UP);
            currentState = PT;
            EV_DETAIL << "Switching State from DE_IDLE to PT" << EV_FIELD(currentState) << EV_ENDL;
        }
        if (RingPort == primaryRingPort
                && LinkState == NetworkInterface::DOWN) {
            setPortState(primaryRingPort, MrpInterfaceData::BLOCKED);
            currentState = AC_STAT1;
            EV_DETAIL << "Switching State from DE_IDLE to AC_STAT1" << EV_FIELD(currentState) << EV_ENDL;
        }
        //all other cases: ignore
        break;
    case PT:
        if (LinkState == NetworkInterface::DOWN) {
            if (RingPort == secondaryRingPort) {
                cancelEvent(linkUpTimer);
                setPortState(secondaryRingPort, MrpInterfaceData::BLOCKED);
                linkChangeReq(primaryRingPort, NetworkInterface::DOWN);
                currentState = DE;
                EV_DETAIL << "Switching State from PT to DE" << EV_FIELD(currentState) << EV_ENDL;
                break;
            }
            if (RingPort == primaryRingPort) {
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
        if (RingPort == secondaryRingPort
                && LinkState == NetworkInterface::UP) {
            cancelEvent(linkDownTimer);
            linkChangeReq(primaryRingPort, NetworkInterface::UP);
            currentState = PT;
            EV_DETAIL << "Switching State from DE to PT" << EV_FIELD(currentState) << EV_ENDL;
            break;
        }
        if (RingPort == primaryRingPort
                && LinkState == NetworkInterface::DOWN) {
            linkChangeCount = linkMaxChange;
            setPortState(primaryRingPort, MrpInterfaceData::BLOCKED);
            currentState = AC_STAT1;
            EV_DETAIL << "Switching State from DE to AC_STAT1" << EV_FIELD(currentState) << EV_ENDL;
        }
        //all other cases: ignore
        break;
    case PT_IDLE:
        if (LinkState == NetworkInterface::DOWN) {
            if (RingPort == secondaryRingPort) {
                setPortState(secondaryRingPort, MrpInterfaceData::BLOCKED);
                linkChangeReq(primaryRingPort, NetworkInterface::DOWN);
                currentState = DE;
                EV_DETAIL << "Switching State from PT_IDLE to DE" << EV_FIELD(currentState) << EV_ENDL;
                break;
            }
            if (RingPort == primaryRingPort) {
                primaryRingPort = secondaryRingPort;
                secondaryRingPort = RingPort;
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

void Mrp::interconnTopologyChangeInd(MacAddress SourceAddress, double Time, uint16_t InID, int RingPort, Packet *Packet) {
    switch (currentState) {
    case POWER_ON:
    case AC_STAT1:
    case DE_IDLE:
    case PT:
    case PT_IDLE:
        delete Packet;
        break;
    case PRM_UP:
    case CHK_RC:
        if (!topologyChangeTimer->isScheduled()) {
            topologyChangeReq(Time);
        }
        delete Packet;
        break;
    case CHK_RO:
        if (!topologyChangeTimer->isScheduled()) {
            topologyChangeReq(Time);
            delete Packet;
        }
        if (RingPort == primaryRingPort) {
            interconnForwardReq(secondaryRingPort, Packet);
        } else if (RingPort == secondaryRingPort) {
            interconnForwardReq(primaryRingPort, Packet);
        }
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }

}

void Mrp::interconnLinkChangeInd(uint16_t InID, uint16_t Linkstate, int RingPort, Packet *Packet) {
    switch (currentState) {
    case POWER_ON:
    case AC_STAT1:
    case DE_IDLE:
    case PT:
    case PT_IDLE:
    case PRM_UP:
    case CHK_RC:
        delete Packet;
        break;
    case CHK_RO:
        if (RingPort == primaryRingPort) {
            interconnForwardReq(secondaryRingPort, Packet);
        } else if (RingPort == secondaryRingPort) {
            interconnForwardReq(primaryRingPort, Packet);
        }
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void Mrp::interconnLinkStatusPollInd(uint16_t InID, int RingPort, Packet *Packet) {
    switch (currentState) {
    case POWER_ON:
    case AC_STAT1:
    case DE_IDLE:
    case PT:
    case PT_IDLE:
    case PRM_UP:
    case CHK_RC:
        delete Packet;
        break;
    case CHK_RO:
        if (RingPort == primaryRingPort) {
            interconnForwardReq(secondaryRingPort, Packet);
        } else if (RingPort == secondaryRingPort) {
            interconnForwardReq(primaryRingPort, Packet);
        }
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void Mrp::interconnTestInd(MacAddress SourceAddress, int RingPort, uint16_t InID, Packet *Packet) {
    switch (currentState) {
    case POWER_ON:
    case AC_STAT1:
    case DE_IDLE:
    case PT:
    case PT_IDLE:
    case PRM_UP:
    case CHK_RC:
        delete Packet;
        break;
    case CHK_RO:
        if (RingPort == primaryRingPort) {
            interconnForwardReq(secondaryRingPort, Packet);
        } else if (RingPort == secondaryRingPort) {
            interconnForwardReq(primaryRingPort, Packet);
        }
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void Mrp::interconnForwardReq(int RingPort, Packet *Packet) {
    auto macAddressInd = Packet->findTag<MacAddressInd>();
    auto SourceAddress = macAddressInd->getSrcAddress();
    auto DestinationAddress = macAddressInd->getDestAddress();
    Packet->trim();
    Packet->clearTags();
    sendFrameReq(RingPort, DestinationAddress, SourceAddress, priority, MRP_LT, Packet);
}

void Mrp::sendFrameReq(int portId, const MacAddress &DestinationAddress, const MacAddress &SourceAddress, int Prio, uint16_t LT, Packet *MRPPDU) {
    MRPPDU->addTag<InterfaceReq>()->setInterfaceId(portId);
    MRPPDU->addTag<PacketProtocolTag>()->setProtocol(&Protocol::mrp);
    MRPPDU->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ieee8022llc);
    auto macAddressReq = MRPPDU->addTag<MacAddressReq>();
    macAddressReq->setSrcAddress(SourceAddress);
    macAddressReq->setDestAddress(DestinationAddress);
    EV_INFO << "Sending packet down" << EV_FIELD(MRPPDU) << EV_FIELD(DestinationAddress) << EV_ENDL;
    send(MRPPDU, "relayOut");
}

void Mrp::sendCCM(int PortId, Packet *CCM) {
    CCM->addTag<InterfaceReq>()->setInterfaceId(PortId);
    CCM->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ieee8021qCFM);
    CCM->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ieee8022llc);
    auto macAddressReq = CCM->addTag<MacAddressReq>();
    macAddressReq->setSrcAddress(getPortNetworkInterface(PortId)->getMacAddress());
    macAddressReq->setDestAddress(ccmMulticastAddress);
    EV_INFO << "Sending packet down" << EV_FIELD(CCM) << EV_ENDL;
    send(CCM, "relayOut");
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

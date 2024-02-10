// Copyright (C) 2024 Daniel Zeitler
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "MediaRedundancyNode.h"

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

Define_Module(MediaRedundancyNode);

MediaRedundancyNode::MediaRedundancyNode()
{
}

MediaRedundancyNode::~MediaRedundancyNode()
{
    cancelAndDelete(linkDownTimer);
    cancelAndDelete(linkUpTimer);
    cancelAndDelete(fdbClearTimer);
    cancelAndDelete(fdbClearDelay);
    cancelAndDelete(topologyChangeTimer);
    cancelAndDelete(testTimer);
    cancelAndDelete(startUpTimer);
    cancelAndDelete(linkUpHysterisisTimer);
}

void MediaRedundancyNode::setRingInterfaces(int InterfaceIndex1, int InterfaceIndex2){
    ringInterface1 = interfaceTable->getInterface(InterfaceIndex1);
    if (ringInterface1->isLoopback()){
        ringInterface1=nullptr;
        EV_DEBUG << "Chosen Interface is Loopback-Interface" << EV_ENDL;
    }
    else
        primaryRingPort = ringInterface1->getInterfaceId();
    ringInterface2 = interfaceTable->getInterface(InterfaceIndex2);
    if (ringInterface2->isLoopback()){
        ringInterface2 = nullptr;
        EV_DEBUG << "Chosen Interface is Loopback-Interface" << EV_ENDL;
    }
    else
        secondaryRingPort = ringInterface2->getInterfaceId();
}

void MediaRedundancyNode::setRingInterface(int InterfaceNumber, int InterfaceIndex){
    if (InterfaceNumber == 1){
        ringInterface1 = interfaceTable->getInterface(InterfaceIndex);
        if (ringInterface1->isLoopback()){
            ringInterface1=nullptr;
            EV_DEBUG << "Chosen Interface is Loopback-Interface" << EV_ENDL;
        }
        else
            primaryRingPort = ringInterface1->getInterfaceId();
    }else if (InterfaceNumber == 2){
        ringInterface2 = interfaceTable->getInterface(InterfaceIndex);
        if (ringInterface2->isLoopback()){
            ringInterface2 = nullptr;
            EV_DEBUG << "Chosen Interface is Loopback-Interface" << EV_ENDL;
        }
        else
            secondaryRingPort = ringInterface2->getInterfaceId();
    }else
        EV_DEBUG << "only 2 MRp Ring-Interfaces per Node allowed" << EV_ENDL;
}

void MediaRedundancyNode::setPortState(int InterfaceId, MrpInterfaceData::PortState State)
{
    auto portData = getPortInterfaceDataForUpdate(InterfaceId);
    portData->setState(State);
    emit(PortStateChangedSignal, simTime().inUnit(SIMTIME_US));
    EV_INFO << "Setting Port State" <<EV_FIELD(InterfaceId) <<EV_FIELD(State) << EV_ENDL;
}

void MediaRedundancyNode::setPortRole(int InterfaceId, MrpInterfaceData::PortRole Role)
{
    auto portData = getPortInterfaceDataForUpdate(InterfaceId);
    portData->setRole(Role);
}

MrpInterfaceData::PortState MediaRedundancyNode::getPortState(int InterfaceId)
{
    auto portData = getPortInterfaceDataForUpdate(InterfaceId);
    return portData->getState();
}

const MrpInterfaceData *MediaRedundancyNode::getPortInterfaceData(unsigned int interfaceId) const
{
    return getPortNetworkInterface(interfaceId)->getProtocolData<MrpInterfaceData>();
}

MrpInterfaceData *MediaRedundancyNode::getPortInterfaceDataForUpdate(unsigned int interfaceId)
{
    return getPortNetworkInterface(interfaceId)->getProtocolDataForUpdate<MrpInterfaceData>();
}

NetworkInterface *MediaRedundancyNode::getPortNetworkInterface(unsigned int interfaceId) const
{
    NetworkInterface *gateIfEntry = interfaceTable->getInterfaceById(interfaceId);
    if (!gateIfEntry)
        throw cRuntimeError("gate's Interface is nullptr");

    return gateIfEntry;
}

MrpInterfaceData::PortRole MediaRedundancyNode::getPortRole(int InterfaceId)
{
    auto portData = getPortInterfaceDataForUpdate(InterfaceId);
    return portData->getRole();
}

void MediaRedundancyNode::toggleRingPorts()
{
    int RingPort = secondaryRingPort;
    secondaryRingPort = primaryRingPort;
    primaryRingPort = RingPort;
    setPortRole(primaryRingPort, MrpInterfaceData::PRIMARY);
    setPortRole(secondaryRingPort, MrpInterfaceData::SECONDARY);
}

void MediaRedundancyNode::initialize(int stage)
{
    OperationalBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        visualize = par("visualize");
        mrpMacForwardingTable.reference(this, "macTableModule", true);
        interfaceTable.reference(this, "interfaceTableModule", true);
        switchModule = getContainingNode(this);
        relay.reference(this, "mrpRelayModule", true);
        int expectedRoleByNum = par("expectedRoleByNum");
        expectedRole = static_cast<mrpRole>(expectedRoleByNum);
        LinkChangeSignal = registerSignal("LinkChangeSignal");
        TopologyChangeSignal = registerSignal("TopologyChangeSignal");
        TestSignal = registerSignal("TestSignal");
        ContinuityCheckSignal = registerSignal("ContinuityCheckSignal");
        ReceivedChangeSignal = registerSignal("ReceivedChangeSignal");
        ReceivedTestSignal = registerSignal("ReceivedTestSignal");
        ReceivedContinuityCheckSignal = registerSignal("ReceivedContinuityCheckSignal");
        RingStateChangedSignal = registerSignal("RingStateChangedSignal");
        PortStateChangedSignal = registerSignal("PortStateChangedSignal");
        ClearFDBSignal =registerSignal("ClearFDBSignal");
        switchModule->subscribe(interfaceStateChangedSignal, this);
        //currently only inferfaceIndex
        primaryRingPort = par("ringPort1");
        secondaryRingPort = par("ringPort2");
        domainID.uuid0 = par("uuid0");
        domainID.uuid1 = par("uuid1");
        timingProfile = par("timingProfile");
        ccmInterval = par("ccmInterval");
        interconnectionLinkCheckAware = par("interconnectionLinkCheckAware");
        interConnectionRingCheckAware = par("interconnectionRingCheckAware");
        enableLinkCheckOnRing = par("enableLinkCheckOnRing");
        //manager variables
        nonBlockingMRC=par("nonBlockingMRC");
        reactOnLinkChange = par("reactOnLinkChange");
        checkMediaRedundancy= par("checkMediaRedundancy");
        noTopologyChange = par("noTopologyChange");
        //client variables
        blockedStateSupported=par("blockedStateSupported");
    }
    if (stage == INITSTAGE_LINK_LAYER) { // "auto" MAC addresses assignment takes place in stage 0
        initPortTable();
        registerProtocol(Protocol::mrp, gate("relayOut"), gate("relayIn"), nullptr, nullptr);
        registerProtocol(Protocol::ieee8021qCFM, gate("relayOut"), gate("relayIn"), nullptr, nullptr);
    }
    if(stage ==INITSTAGE_LAST){
        //set interface and change Port-Indexes to Port-IDs
        setRingInterfaces(primaryRingPort,secondaryRingPort);
        sourceAddress = relay->getBridgeAddress();
        initRingPorts();
        startUpTimer = new cMessage("startUpTimer");
        scheduleAt(SimTime(0,SIMTIME_MS),startUpTimer);
    }
}

void MediaRedundancyNode::initPortTable()
{
    EV_DEBUG << "MRP Interface Data initialization. Setting port infos to the protocol defaults." << EV_ENDL;
    for (unsigned int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
        auto ie = interfaceTable->getInterface(i);
        if (!ie->isLoopback() && ie->isWired() && ie->isMulticast() && ie->getProtocol() == &Protocol::ethernetMac) {
            initInterfacedata(ie->getInterfaceId());
        }
    }
}

void MediaRedundancyNode::initInterfacedata(unsigned int interfaceId)
{
    auto ifd = getPortInterfaceDataForUpdate(interfaceId);
    ifd->setRole(MrpInterfaceData::NOTASSIGNED);
    ifd->setState(MrpInterfaceData::FORWARDING);
    ifd->setLostPDU(0);
    ifd->setContinuityCheckInterval(SimTime(ccmInterval,SIMTIME_MS));
    ifd->setContinuityCheck(false);
    ifd->setNextUpdate(SimTime(ccmInterval*3.5,SIMTIME_MS));
}

void MediaRedundancyNode::initRingPorts()
{
    if (ringInterface1 == nullptr)
        setRingInterface(1,primaryRingPort);
    if (ringInterface2 == nullptr)
        setRingInterface(2,secondaryRingPort);

    auto ifd = getPortInterfaceDataForUpdate(ringInterface1->getInterfaceId());
    ifd->setRole(MrpInterfaceData::PRIMARY);
    ifd->setState(MrpInterfaceData::BLOCKED);
    ifd->setContinuityCheck(enableLinkCheckOnRing);
    ifd->setContinuityCheckInterval(SimTime(ccmInterval,SIMTIME_MS));

    ifd = getPortInterfaceDataForUpdate(ringInterface2->getInterfaceId());
    ifd->setRole(MrpInterfaceData::SECONDARY);
    ifd->setState(MrpInterfaceData::BLOCKED);
    ifd->setContinuityCheck(enableLinkCheckOnRing);
    ifd->setContinuityCheckInterval(SimTime(ccmInterval,SIMTIME_MS));
}

void MediaRedundancyNode::initContinuityCheck()
{
    for (unsigned int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
        auto ie = interfaceTable->getInterface(i);
        if (!ie->isLoopback() && ie->isWired() && ie->isMulticast() && ie->getProtocol() == &Protocol::ethernetMac) {
            int interfaceId=ie->getInterfaceId();
            auto portData = getPortInterfaceDataForUpdate(interfaceId);
            if (portData->getContinuityCheck()){
                auto interface = getPortNetworkInterface(interfaceId);
                MacAddress address = interface->getMacAddress();
                std::string namePort = "CFM CCM " + address.str();
                portData->setCfmName(namePort);
                EV_DETAIL << "CFM-name port set:" <<EV_FIELD(portData->getCfmName()) << EV_ENDL;
                setupContinuityCheck(interfaceId);
                class continuityCheckTimer* checkTimer = new continuityCheckTimer("continuityCheckTimer");
                checkTimer->setPort(interfaceId);
                checkTimer->setKind(1);
                scheduleAt(simTime()+portData->getContinuityCheckInterval(), checkTimer);
                EV_DETAIL << "Next CCM-Interval:" <<EV_FIELD(simTime()+portData->getContinuityCheckInterval()) <<EV_ENDL;
            }
        }
    }
    relay->registerAddress(ccmMulticastAddress);
}

void MediaRedundancyNode::setTimingProfile(int maxRecoveryTime)
{
    //maxrecoverytime in ms,
    switch (maxRecoveryTime){
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

void MediaRedundancyNode::start()
{
    fdbClearTimer = new cMessage("fdbClearTimer");
    fdbClearDelay = new cMessage("fdbClearDelay");
    linkDownTimer = new cMessage("LinkDownTimer");
    linkUpTimer = new cMessage("LinkUpTimer");
    topologyChangeTimer = new cMessage("topologyChangeTimer");
    testTimer = new cMessage("testTimer");
    linkUpHysterisisTimer = new cMessage("linkUpHysterisisTimer");
    setTimingProfile(timingProfile);
    topologyChangeRepeatCount = topologyChangeMaxRepeatCount -1;
    if (enableLinkCheckOnRing || interconnectionLinkCheckAware){
        initContinuityCheck();
    }
    //Client
    if (expectedRole== CLIENT ){
        mrcInit();
    }
    else if (expectedRole == MANAGER){
        mrmInit();
    }
    else if (expectedRole == MANAGER_AUTO){
        mraInit();
    }
}

void MediaRedundancyNode::stop()
{
    setPortRole(primaryRingPort, MrpInterfaceData::NOTASSIGNED);
    setPortRole(secondaryRingPort, MrpInterfaceData::NOTASSIGNED);
    setPortState(primaryRingPort, MrpInterfaceData::DISABLED);
    setPortState(secondaryRingPort, MrpInterfaceData::DISABLED);
    expectedRole=DISABLED;
    cancelAndDelete(fdbClearTimer);
    cancelAndDelete(fdbClearDelay);
    cancelAndDelete(topologyChangeTimer);
    cancelAndDelete(testTimer);
    cancelAndDelete(linkDownTimer);
    cancelAndDelete(linkUpTimer);
    cancelAndDelete(linkUpHysterisisTimer);
    cancelAndDelete(startUpTimer);
}

void MediaRedundancyNode::read()
{
    //TODO
}

void MediaRedundancyNode::mrcInit()
{
    linkChangeCount=linkMaxChange;
    currentRingState = UNDEFINED;
    mrpMacForwardingTable->addMrpForwardingInterface(primaryRingPort,static_cast<MacAddress>(MC_CONTROL), vlanID);
    mrpMacForwardingTable->addMrpForwardingInterface(secondaryRingPort,static_cast<MacAddress>(MC_CONTROL),vlanID);
    mrpMacForwardingTable->addMrpForwardingInterface(primaryRingPort,static_cast<MacAddress>(MC_TEST), vlanID);
    mrpMacForwardingTable->addMrpForwardingInterface(secondaryRingPort,static_cast<MacAddress>(MC_TEST), vlanID);
    relay->registerAddress(static_cast<MacAddress>(MC_CONTROL));

    if(interConnectionRingCheckAware || interconnectionLinkCheckAware)
    {
        mrpMacForwardingTable->addMrpForwardingInterface(primaryRingPort, static_cast<MacAddress>(MC_INCONTROL),vlanID);
        mrpMacForwardingTable->addMrpForwardingInterface(secondaryRingPort, static_cast<MacAddress>(MC_INCONTROL),vlanID);
    }
    if(interConnectionRingCheckAware)
    {
        mrpMacForwardingTable->addMrpForwardingInterface(primaryRingPort, static_cast<MacAddress>(MC_INTEST), vlanID);
        mrpMacForwardingTable->addMrpForwardingInterface(secondaryRingPort, static_cast<MacAddress>(MC_INTEST), vlanID);
    }
    if (expectedRole == CLIENT){
        currentState = AC_STAT1; //only for client, managerAuto is keeping the current state
    }
    mauTypeChangeInd(primaryRingPort, getPortNetworkInterface(primaryRingPort)->getState());
    mauTypeChangeInd(secondaryRingPort, getPortNetworkInterface(secondaryRingPort)->getState());
}

void MediaRedundancyNode::mrmInit()
{
    managerPrio = DEFAULT;
    currentRingState = OPEN;
    addTest = false;
    testRetransmissionCount = 0;
    relay->registerAddress(static_cast<MacAddress>(MC_TEST));
    relay->registerAddress(static_cast<MacAddress>(MC_CONTROL));
    if (interConnectionRingCheckAware || interconnectionLinkCheckAware)
        relay->registerAddress(static_cast<MacAddress>(MC_INCONTROL));
    if (interConnectionRingCheckAware)
        relay->registerAddress(static_cast<MacAddress>(MC_INTEST));
    testMaxRetransmissionCount=testMonitoringCount -1;
    testRetransmissionCount = 0;
    if (expectedRole== MANAGER)
        currentState = AC_STAT1;
    if (expectedRole == MANAGER_AUTO){
        //in managerRole no Forwarding on RingPorts may take place
        mrpMacForwardingTable->removeMrpForwardingInterface(primaryRingPort, static_cast<MacAddress>(MC_TEST), vlanID);
        mrpMacForwardingTable->removeMrpForwardingInterface(secondaryRingPort, static_cast<MacAddress>(MC_TEST), vlanID);
        mrpMacForwardingTable->removeMrpForwardingInterface(primaryRingPort, static_cast<MacAddress>(MC_CONTROL), vlanID);
        mrpMacForwardingTable->removeMrpForwardingInterface(secondaryRingPort, static_cast<MacAddress>(MC_CONTROL), vlanID);
    }
    mauTypeChangeInd(primaryRingPort, getPortNetworkInterface(primaryRingPort)->getState());
    mauTypeChangeInd(secondaryRingPort, getPortNetworkInterface(secondaryRingPort)->getState());
}

void MediaRedundancyNode::mraInit()
{
    managerPrio = DEFAULT;
    currentRingState = OPEN;
    addTest = false;
    testRetransmissionCount = 0;
    relay->registerAddress(static_cast<MacAddress>(MC_TEST));
    relay->registerAddress(static_cast<MacAddress>(MC_CONTROL));
    if (interConnectionRingCheckAware || interconnectionLinkCheckAware)
        relay->registerAddress(static_cast<MacAddress>(MC_INCONTROL));
    if (interConnectionRingCheckAware)
        relay->registerAddress(static_cast<MacAddress>(MC_INTEST));
    testMaxRetransmissionCount=testMonitoringCount -1;
    testRetransmissionCount = 0;
    addTest =false;
    reactOnLinkChange = false;
    hostBestMRMSourceAddress = static_cast<MacAddress>(0xFFFFFFFFFFFF);
    hostBestMRMPriority = static_cast<mrpPriority>(0xFFFF);
    monNReturn =0;
    currentState = AC_STAT1;
    mauTypeChangeInd(primaryRingPort, getPortNetworkInterface(primaryRingPort)->getState());
    mauTypeChangeInd(secondaryRingPort, getPortNetworkInterface(secondaryRingPort)->getState());
}

void MediaRedundancyNode::clearFDB(double Time)
{
    if (!fdbClearTimer->isScheduled())
        scheduleAt(simTime()+SimTime(Time,SIMTIME_MS), fdbClearTimer);
    else if (fdbClearTimer->getArrivalTime() > (simTime()+ SimTime(Time,SIMTIME_MS))){
        cancelEvent(fdbClearTimer);
        scheduleAt(simTime()+SimTime(Time,SIMTIME_MS), fdbClearTimer);
    }
}

void MediaRedundancyNode::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("receiveSignal");
    EV_DETAIL << "Received Signal:" <<EV_FIELD(signalID) << EV_ENDL;
    if (signalID == interfaceStateChangedSignal){
        NetworkInterfaceChangeDetails* interfaceDetails = static_cast<NetworkInterfaceChangeDetails*>(obj);
        auto interface=interfaceDetails->getNetworkInterface();
        auto field = interfaceDetails->getFieldId();
        auto interfaceId= interface->getInterfaceId();
        EV_DETAIL << "Received Signal:" <<EV_FIELD(field) <<EV_FIELD(interfaceId) << EV_ENDL;
        if(interfaceId >= 0){
            processDelayTimer* DelayTimer = new processDelayTimer("DelayTimer");
            DelayTimer->setPort(interface->getInterfaceId());
            DelayTimer->setField(field);
            DelayTimer->setKind(0);
            if (field == NetworkInterface::F_STATE || field == NetworkInterface::F_CARRIER) {
                linkDetectionDelay = truncnormal(linkDetectionDelayMean,linkDetectionDelayDev);
                if(linkUpHysterisisTimer->isScheduled())
                    cancelEvent(linkUpHysterisisTimer);
                scheduleAt(simTime()+SimTime(linkDetectionDelay,SIMTIME_US), linkUpHysterisisTimer);
                scheduleAt(simTime()+SimTime(linkDetectionDelay,SIMTIME_US), DelayTimer);
            }
        }
    }
}

void MediaRedundancyNode::handleMessageWhenUp(cMessage *msg)
{
    if (!msg->isSelfMessage()) {
        msg->setKind(2);
        EV_INFO << "Received Message on MrpNode, Rescheduling:" << EV_FIELD(msg) << EV_ENDL;
        processingDelay = truncnormal(processingDelayMean, processingDelayDev);
        scheduleAt(simTime()+SimTime(processingDelay,SIMTIME_US), msg);
    }
    else {
        EV_INFO << "Received Self-Message:" << EV_FIELD(msg) << EV_ENDL;
        if(msg == testTimer)
            handleTestTimer();
        else if(msg == topologyChangeTimer)
            handleTopologyChangeTimer();
        else if(msg == linkUpTimer)
            handleLinkUpTimer();
        else if(msg == linkDownTimer)
            handleLinkDownTimer();
        else if(msg == fdbClearTimer)
            clearLocalFDB();
        else if(msg == fdbClearDelay)
            clearLocalFDBDelayed();
        else if (msg == startUpTimer){
            start();
        }
        else if (msg == linkUpHysterisisTimer){
            //action done by handleDelayTimer, linkUpHysterisisTimer requested by standard
            //but not further descripted
        }
        else if (msg->getKind() == 0){
            processDelayTimer* timer = check_and_cast<processDelayTimer *>(msg);
            if (timer != nullptr){
                handleDelayTimer(timer->getPort(), timer->getField());
            }
            delete timer;
        }
        else if (msg->getKind() == 1){
            continuityCheckTimer* timer = check_and_cast<continuityCheckTimer *>(msg);
            handleContinuityCheckTimer(timer->getPort());
            delete timer;
        }
        else if (msg->getKind() == 2){
            Packet *packet = check_and_cast<Packet *>(msg);
            auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
            if (protocol == &Protocol::ieee8021qCFM){
                handleContinuityCheckMessage(packet);
            }
            if (protocol == &Protocol::mrp){
                handleMrpPDU(packet);
            }
        }
        else
            throw cRuntimeError("Unknown self-message received");
    }
}

void MediaRedundancyNode::handleMrpPDU(Packet *packet)
{
    auto interfaceInd = packet->findTag<InterfaceInd>();
    auto macAddressInd = packet->findTag<MacAddressInd>();
    auto SourceAddress = macAddressInd->getSrcAddress();
    auto DestinationAddress = macAddressInd->getDestAddress();
    int RingPort = interfaceInd->getInterfaceId();
    auto incomingInterface = interfaceTable->getInterfaceById(RingPort);
    //unsigned int vlanId = 0;
    //if (auto vlanInd = packet->findTag<VlanInd>())
    //    vlanId = vlanInd->getVlanId();

    auto version = packet->peekAtFront<mrpVersionField>();
    auto offset = version->getChunkLength();
    auto firstTLV=packet->peekDataAt<tlvHeader>(offset);
    offset = offset + B(firstTLV->getHeaderLength()) + B(2);
    auto commonTLV= packet->peekDataAt<commonHeader>(offset);
    auto sequence = commonTLV->getSequenceID();
    bool ringID= false;
    if (commonTLV->getUuid0() == domainID.uuid0 && commonTLV->getUuid1()== domainID.uuid1)
        ringID = true;
    offset = offset + commonTLV->getChunkLength();

    switch (firstTLV->getHeaderType()){
    case TEST:
    {
        EV_DETAIL << "Received Test-Frame" << EV_ENDL;
        auto testTLV = dynamicPtrCast<const testFrame>(firstTLV);
        if (ringID){
            if (sequence > lastTestId){
                lastTestId = sequence;
                testRingInd(testTLV->getSa(), static_cast<mrpPriority>(testTLV->getPrio()));
                int ringTime = simTime().inUnit(SIMTIME_MS) - testTLV->getTimeStamp();
                auto lastTestFrameSent = testFrameSent.find(sequence);
                if (lastTestFrameSent != testFrameSent.end()){
                    int64_t ringTimePrecise = simTime().inUnit(SIMTIME_US) - lastTestFrameSent->second;
                    emit(ReceivedTestSignal,ringTimePrecise);
                    EV_DETAIL << "RingTime" <<EV_FIELD(ringTime) <<EV_FIELD(ringTimePrecise) << EV_ENDL;
                }
                else{
                    emit(ReceivedTestSignal,ringTime*1000);
                    EV_DETAIL << "RingTime" <<EV_FIELD(ringTime) << EV_ENDL;
                }
            }
        }
        else{
            EV_DETAIL << "Received packet from other Mrp-Domain" << EV_FIELD(incomingInterface) << EV_FIELD(packet) << EV_ENDL;
        }
        break;
    }
    case TOPOLOGYCHANGE:
    {
        EV_DETAIL << "Received TopologyChange-Frame" << EV_ENDL;
        auto topologyTLV = dynamicPtrCast<const topologyChangeFrame>(firstTLV);
        if (ringID){
            if (sequence > lastTopologyId){
                lastTopologyId = sequence;
                topologyChangeInd(topologyTLV->getSa(), topologyTLV->getInterval());
            }
            emit(ReceivedChangeSignal,topologyTLV->getInterval());
        }
        else{
            EV_DETAIL << "Received packet from other Mrp-Domain" << EV_FIELD(incomingInterface) << EV_FIELD(packet) << EV_ENDL;
        }
        break;
    }
    case LINKDOWN:
    case LINKUP:
    {
        EV_DETAIL << "Received LinkChange-Frame" << EV_ENDL;
        auto linkTLV = dynamicPtrCast<const linkChangeFrame>(firstTLV);
        if (ringID){
            linkChangeInd(linkTLV->getPortRole(), linkTLV->getBlocked());
            emit(ReceivedChangeSignal,linkTLV->getInterval());
        }
        else{
            EV_DETAIL << "Received packet from other Mrp-Domain" << EV_FIELD(incomingInterface) << EV_FIELD(packet) << EV_ENDL;
        }
        break;
    }
    case OPTION:
    {
        EV_DETAIL << "Received Option-Frame" << EV_ENDL;
        if (ringID){
            auto optionTLV = dynamicPtrCast<const optionHeader>(firstTLV);
            b subOffset=version->getChunkLength() + optionTLV->getChunkLength();
            //handle if manufactorerData is present
            if (optionTLV->getOuiType() != mrpOuiType::IEC && (optionTLV->getEd1Type() == 0x00 || optionTLV->getEd1Type() == 0x04)){
                if (optionTLV->getEd1Type() == 0x00){
                    auto dataChunk=packet->peekDataAt<FieldsChunk>(subOffset);
                    subOffset = subOffset + B(ed1DataLength::LENGTH0);
                }
                else if (optionTLV->getEd1Type() == 0x04){
                    auto dataChunk=packet->peekDataAt<FieldsChunk>(subOffset);
                    subOffset = subOffset + B(ed1DataLength::LENGTH4);
                }
            }

            //handle suboption2 if present
            if ((optionTLV->getOuiType() == mrpOuiType::IEC && optionTLV->getHeaderLength() > 4) ||
                    (optionTLV->getEd1Type() == 0x00 && optionTLV->getHeaderLength() > (4+ed1DataLength::LENGTH0)) ||
                    (optionTLV->getEd1Type() == 0x04 && optionTLV->getHeaderLength() > (4+ed1DataLength::LENGTH4))){
                auto subTLV = packet->peekDataAt<subTlvHeader>(subOffset);
                switch (subTLV->getSubType()){
                case RESERVED:
                {
                    auto subOptionTLV = dynamicPtrCast<const manufacturerFktHeader>(subTLV);
                    //not implemented
                    break;
                }
                case TEST_MGR_NACK:
                {
                    if (expectedRole == MANAGER_AUTO){
                        auto subOptionTLV = dynamicPtrCast<const subTlvTestFrame>(subTLV);
                        testMgrNackInd(RingPort, subOptionTLV->getSa(),  static_cast<mrpPriority>(subOptionTLV->getPrio()), subOptionTLV->getOtherMRMSa());
                    }
                    break;
                }
                case TEST_PROPAGATE:
                {
                    if (expectedRole == MANAGER_AUTO){
                        auto subOptionTLV = dynamicPtrCast<const subTlvTestFrame>(subTLV);
                        testPropagateInd(RingPort, subOptionTLV->getSa(),  static_cast<mrpPriority>(subOptionTLV->getPrio()), subOptionTLV->getOtherMRMSa(),  static_cast<mrpPriority>(subOptionTLV->getOtherMRMPrio()));
                    }
                    break;
                }
                case AUTOMGR:
                {
                    //nothing else to do
                    EV_DETAIL << "Received TestFrame from Automanager" << EV_ENDL;
                    break;
                }
                default:
                    throw cRuntimeError("unknown subTLV TYPE: %d", subTLV->getSubType());
                }
            }
        }
        else{
            EV_DETAIL << "Received packet from other Mrp-Domain" << EV_FIELD(incomingInterface) << EV_FIELD(packet) << EV_ENDL;
        }
        break;
    }
    case INTEST:
    {
        EV_DETAIL << "Received inTest-Frame" << EV_ENDL;
        auto inTestTLV = dynamicPtrCast<const inTestFrame>(firstTLV);
        interconnTestInd(inTestTLV->getSa(), RingPort, inTestTLV->getInID(),packet->dup());
        break;
    }
    case INTOPOLOGYCHANGE:
    {
        EV_DETAIL << "Received inTopologyChange-Frame" << EV_ENDL;
        auto inTopologyTLV = dynamicPtrCast<const inTopologyChangeFrame>(firstTLV);
        interconnTopologyChangeInd(inTopologyTLV->getSa(), inTopologyTLV->getInterval(),inTopologyTLV->getInID(), RingPort, packet->dup());
        break;
    }
    case INLINKDOWN:
    case INLINKUP:
    {
        EV_DETAIL << "Received inLinkChange-Frame" << EV_ENDL;
        auto inLinkTLV = dynamicPtrCast<const inLinkChangeFrame>(firstTLV);
        interconnLinkChangeInd(inLinkTLV->getInID(), inLinkTLV->getLinkInfo(),RingPort, packet->dup());
        break;
    }
    case INLINKSTATUSPOLL:
    {
        EV_DETAIL << "Received inLinkStatusPoll" << EV_ENDL;
        auto inLinkStatusTLV = dynamicPtrCast<const inLinkStatusPollFrame>(firstTLV);
        interconnLinkStatusPollInd(inLinkStatusTLV->getInID(), RingPort, packet->dup());
        break;
    }
    default:
        throw cRuntimeError("unknown TLV TYPE: %d", firstTLV->getHeaderType());
    }

    //addtional Option-Frame
    auto thirdTLV=packet->peekDataAt<tlvHeader>(offset);
    if (thirdTLV->getHeaderType() != END && ringID){
        EV_DETAIL << "Received additional Option-Frame" << EV_ENDL;
        auto optionTLV = dynamicPtrCast<const optionHeader>(thirdTLV);
        b subOffset=offset + optionTLV->getChunkLength();
        //handle if manufactorerData is present
        if (optionTLV->getOuiType() != mrpOuiType::IEC && (optionTLV->getEd1Type() == 0x00 || optionTLV->getEd1Type() == 0x04)){
            if (optionTLV->getEd1Type() == 0x00){
                auto dataChunk=packet->peekDataAt<FieldsChunk>(subOffset);
                subOffset = subOffset + B(ed1DataLength::LENGTH0);
            }
            else if (optionTLV->getEd1Type() == 0x04){
                auto dataChunk=packet->peekDataAt<FieldsChunk>(subOffset);
                subOffset = subOffset + B(ed1DataLength::LENGTH4);
            }
        }

        //handle suboption2 if present
        if ((optionTLV->getOuiType() == mrpOuiType::IEC && optionTLV->getHeaderLength() > 4) ||
                (optionTLV->getEd1Type() == 0x00 && optionTLV->getHeaderLength() > (4+ed1DataLength::LENGTH0)) ||
                (optionTLV->getEd1Type() == 0x04 && optionTLV->getHeaderLength() > (4+ed1DataLength::LENGTH4))){
            auto subTLV = packet->peekDataAt<subTlvHeader>(subOffset);
            switch (subTLV->getSubType()){
            case RESERVED:
            {
                auto subOptionTLV = dynamicPtrCast<const manufacturerFktHeader>(subTLV);
                //not implemented
                break;
            }
            case TEST_MGR_NACK:
            {
                if (expectedRole == MANAGER_AUTO){
                    auto subOptionTLV = dynamicPtrCast<const subTlvTestFrame>(subTLV);
                    testMgrNackInd(RingPort, subOptionTLV->getSa(),  static_cast<mrpPriority>(subOptionTLV->getPrio()), subOptionTLV->getOtherMRMSa());
                }
                break;
            }
            case TEST_PROPAGATE:
            {
                if (expectedRole == MANAGER_AUTO){
                    auto subOptionTLV = dynamicPtrCast<const subTlvTestFrame>(subTLV);
                    testPropagateInd(RingPort, subOptionTLV->getSa(),  static_cast<mrpPriority>(subOptionTLV->getPrio()), subOptionTLV->getOtherMRMSa(),  static_cast<mrpPriority>(subOptionTLV->getOtherMRMPrio()));
                }
                break;
            }
            case AUTOMGR:
            {
                //nothing else to do
                EV_DETAIL << "Received TestFrame from Automanager" << EV_ENDL;
                break;
            }
            default:
                throw cRuntimeError("unknown subTLV TYPE: %d", subTLV->getSubType());
            }
        }
        offset = offset + B(thirdTLV->getHeaderLength())+B(2);
    }
    //auto endTLV=packet->peekDataAt<tlvHeader>(offset);
    delete packet;
}

void MediaRedundancyNode::handleContinuityCheckMessage(Packet *packet)
{
    EV_DETAIL <<"Handling CCM"<<EV_ENDL;
    auto interfaceInd = packet->getTag<InterfaceInd>();
    auto macAddressInd = packet->getTag<MacAddressInd>();
    auto SourceAddress = macAddressInd->getSrcAddress();
    int RingPort = interfaceInd->getInterfaceId();
    auto incomingInterface = interfaceTable->getInterfaceById(RingPort);
    auto ccm=packet->popAtFront<continuityCheckMessage>();
    auto portData = getPortInterfaceDataForUpdate(RingPort);
    if (ccm->getEndpointIdentifier() == portData->getCfmEndpointID()){
        int i = SourceAddress.compareTo(incomingInterface->getMacAddress());
        if (i == -1){
            portData->setCfmEndpointID(2);
            MacAddress address = incomingInterface->getMacAddress();
            std::string namePort = "CFM CCM " + address.str();
            portData->setCfmName(namePort);
        }
    }
    portData->setNextUpdate(simTime() + portData->getContinuityCheckInterval()*3.5);
    EV_DETAIL <<"new ccm-Data" <<EV_FIELD(portData->getNextUpdate()) <<EV_FIELD(portData->getCfmEndpointID()) <<EV_FIELD(SourceAddress) <<EV_FIELD(incomingInterface->getMacAddress()) <<EV_ENDL;
    mauTypeChangeInd(RingPort, NetworkInterface::UP);
    emit(ReceivedContinuityCheckSignal,RingPort);
    delete packet;
}

void MediaRedundancyNode::handleDelayTimer(int interfaceId, int field)
{
    EV_INFO << "handling DelayTimer" << EV_ENDL;
    auto interface = getPortNetworkInterface(interfaceId);
    if (field == NetworkInterface::F_STATE || (field == NetworkInterface::F_CARRIER && interface->isUp())){
        auto portData = getPortInterfaceDataForUpdate(interfaceId);
        if (portData->getContinuityCheck()){
            auto nextUpdate =simTime()+portData->getContinuityCheckInterval()*3.5;
            portData->setNextUpdate(nextUpdate);
        }
        if (interface->isUp())
            mauTypeChangeInd(interfaceId,  NetworkInterface::UP);
        else
            mauTypeChangeInd(interfaceId, NetworkInterface::DOWN);
    }
}

void MediaRedundancyNode::clearLocalFDB()
{
    //mrpMacForwardingTable->removeForwardingInterface(primaryRingPort);
    //mrpMacForwardingTable->removeForwardingInterface(secondaryRingPort);
    EV_DETAIL << "clearing FDB" << EV_ENDL;
    if (fdbClearDelay->isScheduled())
        cancelEvent(fdbClearDelay);
    processingDelay = truncnormal(processingDelayMean,processingDelayDev);
    scheduleAt(simTime()+SimTime(processingDelay,SIMTIME_US), fdbClearDelay );
    emit(ClearFDBSignal, simTime().inUnit(SIMTIME_US));
}

void MediaRedundancyNode::clearLocalFDBDelayed()
{
    mrpMacForwardingTable->clearTable();
    EV_DETAIL << "FDB cleared" << EV_ENDL;
}

void MediaRedundancyNode::handleContinuityCheckTimer(int RingPort)
{
    auto portData = getPortInterfaceDataForUpdate(RingPort);
    EV_DETAIL <<"Checktimer:" <<EV_FIELD(simTime()) <<EV_FIELD(portData->getNextUpdate())<<EV_ENDL;
    if (simTime() >= portData->getNextUpdate()){
        //no Message received within Lifetime
        EV_DETAIL <<"Checktimer: Link considered down" <<EV_FIELD(simTime()) <<EV_FIELD(portData->getNextUpdate())<<EV_ENDL;
        mauTypeChangeInd(RingPort, NetworkInterface::DOWN);
    }
    setupContinuityCheck(RingPort);
    EV_DETAIL <<"Checktimer:" <<EV_FIELD(RingPort) <<EV_ENDL;
    class continuityCheckTimer* checkTimer = new continuityCheckTimer("continuityCheckTimer");
    checkTimer->setPort(RingPort);
    checkTimer->setKind(1);
    scheduleAt(simTime()+portData->getContinuityCheckInterval(), checkTimer);
}

void MediaRedundancyNode::handleTestTimer()
{
    switch (currentState)
    {
    case POWER_ON:
    case AC_STAT1:
        if (expectedRole == MANAGER){
            mauTypeChangeInd(primaryRingPort, getPortNetworkInterface(primaryRingPort)->getState());
        }
        break;
    case PRM_UP:
        addTest= false;
        testRingReq(defaultTestInterval);
        break;
    case CHK_RO:
        addTest = false;
        testRingReq(defaultTestInterval);
        break;
    case CHK_RC:
        if (testRetransmissionCount >= testMaxRetransmissionCount){
            setPortState(secondaryRingPort, MrpInterfaceData::FORWARDING);
            testMaxRetransmissionCount = testMonitoringCount - 1;
            testRetransmissionCount = 0;
            addTest =false;
            if (!noTopologyChange){
                topologyChangeReq(topologyChangeInterval);
            }
            testRingReq(defaultTestInterval);
            currentState = CHK_RO;
            EV_INFO << "Switching State from CHK_RC to CHK_RO" << EV_FIELD(currentState) << EV_ENDL;
            currentRingState = OPEN;
            emit(RingStateChangedSignal, simTime().inUnit(SIMTIME_US));
        }else{
            testRetransmissionCount++;
            addTest= false;
            testRingReq(defaultTestInterval);
        }
        break;
    case DE:
    case DE_IDLE:
        if (expectedRole == MANAGER_AUTO){
            scheduleAt(simTime()+SimTime(shortTestInterval,SIMTIME_MS), testTimer);
            if (monNReturn <= monNRmax){
                monNReturn++;
            }
            else{
                mrmInit();
                currentState = PRM_UP;
                EV_INFO << "Switching State from DE_IDLE to PRM_UP" << EV_FIELD(currentState) << EV_ENDL;
            }
        }
        break;
    case PT:
        if (expectedRole == MANAGER_AUTO){
            scheduleAt(simTime()+SimTime(shortTestInterval,SIMTIME_MS), testTimer);
            if (monNReturn <= monNRmax){
                monNReturn++;
            }
            else{
                mrmInit();
                currentState = CHK_RC;
                EV_INFO << "Switching State from PT to CHK_RC" << EV_FIELD(currentState) << EV_ENDL;
            }
        }
        break;
    case PT_IDLE:
        if (expectedRole == MANAGER_AUTO){
            scheduleAt(simTime()+SimTime(shortTestInterval,SIMTIME_MS), testTimer);
            if (monNReturn <= monNRmax){
                monNReturn++;
            }
            else{
                mrmInit();
                currentState = CHK_RO;
                EV_INFO << "Switching State from PT_IDLE to CHK_RO" << EV_FIELD(currentState) << EV_ENDL;
            }
        }
        break;
    default:
        throw cRuntimeError("Unknown Node State");
    }
}

void MediaRedundancyNode::handleTopologyChangeTimer()
{
    if (topologyChangeRepeatCount > 0){
        setupTopologyChangeReq(topologyChangeRepeatCount * topologyChangeInterval);
        topologyChangeRepeatCount--;
        scheduleAt(simTime()+SimTime(topologyChangeInterval,SIMTIME_MS), topologyChangeTimer);
    }
    else{
        topologyChangeRepeatCount = topologyChangeMaxRepeatCount -1;
        clearLocalFDB();
    }
}

void MediaRedundancyNode::handleLinkUpTimer()
{
    switch (currentState)
    {
    case PT:
        if (linkChangeCount == 0){
            setPortState(secondaryRingPort,MrpInterfaceData::FORWARDING);
            linkChangeCount= linkMaxChange;
            currentState = PT_IDLE;
            EV_INFO << "Switching State from PT to PT_IDLE" << EV_FIELD(currentState) << EV_ENDL;
        }
        else{
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

void MediaRedundancyNode::handleLinkDownTimer()
{
    switch (currentState)
    {
    case DE:
        if (linkChangeCount == 0){
            linkChangeCount = linkMaxChange;
            currentState = DE_IDLE;
            EV_INFO << "Switching State from DE to DE_IDLE" << EV_FIELD(currentState) << EV_ENDL;
        }
        else{
            linkChangeReq(primaryRingPort,NetworkInterface::DOWN);
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

void MediaRedundancyNode::setupContinuityCheck(int RingPort)
{
    auto CCM = makeShared<continuityCheckMessage>();
    auto portData = getPortInterfaceDataForUpdate(RingPort);
    if (portData->getContinuityCheckInterval() == 3.3){
        CCM->setFlags(0x00000001);
    }
    else if (portData->getContinuityCheckInterval() == 10){
        CCM->setFlags(0x00000002);
    }
    if (RingPort == primaryRingPort){
        CCM->setSequenceNumber(sequenceCCM1);
        sequenceCCM1++;
    }
    else if (RingPort == secondaryRingPort){
        CCM->setSequenceNumber(sequenceCCM2);
        sequenceCCM2++;
    }
    CCM->setEndpointIdentifier(portData->getCfmEndpointID());
    auto name = portData->getCfmName();
    EV_INFO << "CCM-Name" << EV_FIELD(name)<< EV_ENDL;
    CCM->setMessageName(name.c_str());
    auto packet = new Packet("ContinuityCheck", CCM);
    sendCCM(RingPort, packet);
    emit(ContinuityCheckSignal, RingPort);
}

void MediaRedundancyNode::testRingReq(double Time)
{
    setupTestRingReq();
    if (!testTimer->isScheduled()){
        scheduleAt(simTime()+SimTime(Time,SIMTIME_MS), testTimer);
    }
    else if (addTest){
        cancelEvent(testTimer);
        scheduleAt(simTime()+SimTime(Time,SIMTIME_MS), testTimer);
    }
    else
        EV_INFO << "Testtimer already scheduled" << EV_ENDL;
}

void MediaRedundancyNode::topologyChangeReq(double Time)
{
    setupTopologyChangeReq(Time*topologyChangeMaxRepeatCount);
    if (Time == 0){
        clearLocalFDB();
    }
    else {
        scheduleAt(simTime() + SimTime(Time,SIMTIME_MS), topologyChangeTimer);
    }
}

void MediaRedundancyNode::linkChangeReq(int RingPort, uint16_t LinkState)
{
    if (LinkState == NetworkInterface::DOWN){
        if (!linkDownTimer->isScheduled()) {
            scheduleAt(simTime()+SimTime(linkDownInterval,SIMTIME_MS),linkDownTimer);
            setupLinkChangeReq(primaryRingPort,NetworkInterface::DOWN,linkChangeCount*linkDownInterval);
            linkChangeCount--;
        }
    }
    else if (LinkState == NetworkInterface::UP){
        if (!linkUpTimer->isScheduled()){
            scheduleAt(simTime()+ SimTime(linkUpInterval, SIMTIME_MS), linkUpTimer);
            setupLinkChangeReq(primaryRingPort, NetworkInterface::UP, linkChangeCount*linkUpInterval);
            linkChangeCount--;
        }
    }
    else
        throw cRuntimeError("Unknown LinkState in LinkChangeReq");
}

void MediaRedundancyNode::setupTestRingReq()
{
    //Create MRP-PDU according MRP_Test
    auto Version = makeShared<mrpVersionField>();
    auto TestTLV1 = makeShared<testFrame>();
    auto TestTLV2 = makeShared<testFrame>();
    auto CommonTLV = makeShared<commonHeader>();
    auto EndTLV = makeShared<tlvHeader>();

    uint32_t timestamp= simTime().inUnit(SIMTIME_MS);
    int64_t lastTestFrameSent = simTime().inUnit(SIMTIME_US);
    testFrameSent.insert({sequenceID, lastTestFrameSent});

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

    Packet* packet1 = new Packet("mrpTestFrame");
    packet1->insertAtBack(Version);
    packet1->insertAtBack(TestTLV1);
    packet1->insertAtBack(CommonTLV);

    Packet* packet2 = new Packet("mrpTestFrame");
    packet2->insertAtBack(Version);
    packet2->insertAtBack(TestTLV2);
    packet2->insertAtBack(CommonTLV);

    //MRA only
    if (expectedRole == MANAGER_AUTO){
        auto OptionTLV = makeShared<optionHeader>();
        auto AutoMgrTLV = makeShared<subTlvHeader>();
        uint8_t headerLength = OptionTLV->getHeaderLength()+AutoMgrTLV->getSubHeaderLength()+2;
        OptionTLV->setHeaderLength(headerLength);
        packet1->insertAtBack(OptionTLV);
        packet1->insertAtBack(AutoMgrTLV);
        packet2->insertAtBack(OptionTLV);
        packet2->insertAtBack(AutoMgrTLV);
    }
    packet1->insertAtBack(EndTLV);
    MacAddress SourceAddress1 = getPortNetworkInterface(primaryRingPort)->getMacAddress();
    sendFrameReq(primaryRingPort, static_cast<MacAddress>(MC_TEST), SourceAddress1, priority, MRP_LT , packet1);

    packet2->insertAtBack(EndTLV);
    MacAddress SourceAddress2 = getPortNetworkInterface(secondaryRingPort)->getMacAddress();
    sendFrameReq(secondaryRingPort, static_cast<MacAddress>(MC_TEST), SourceAddress2, priority, MRP_LT ,packet2);
    emit(TestSignal, lastTestFrameSent);
}

void MediaRedundancyNode::setupTopologyChangeReq(uint32_t Interval)
{
    //Create MRP-PDU according MRP_TopologyChange
    auto Version = makeShared<mrpVersionField>();
    auto TopologyChangeTLV = makeShared<topologyChangeFrame>();
    auto TopologyChangeTLV2 = makeShared<topologyChangeFrame>();
    auto CommonTLV = makeShared<commonHeader>();
    auto EndTLV = makeShared<tlvHeader>();

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
    emit(TopologyChangeSignal,simTime().inUnit(SIMTIME_US));
}

void MediaRedundancyNode::setupLinkChangeReq(int RingPort, uint16_t LinkState, double Time)
{
    //Create MRP-PDU according MRP_LinkChange
    auto Version = makeShared<mrpVersionField>();
    auto LinkChangeTLV = makeShared<linkChangeFrame>();
    auto CommonTLV = makeShared<commonHeader>();
    auto EndTLV = makeShared<tlvHeader>();

    if (LinkState == NetworkInterface::UP){
        LinkChangeTLV->setHeaderType(LINKUP);
    }
    else if (LinkState == NetworkInterface::DOWN){
        LinkChangeTLV->setHeaderType(LINKDOWN);
    }
    else{
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
    emit(LinkChangeSignal, simTime().inUnit(SIMTIME_US));
}

void MediaRedundancyNode::testMgrNackReq(mrpPriority ManagerPrio, MacAddress SourceAddress)
{
    //Create MRP-PDU according MRP_Option and Suboption2 MRP-TestMgrNack
    auto Version = makeShared<mrpVersionField>();
    auto OptionTLV = makeShared<optionHeader>();
    auto TestMgrTLV = makeShared<subTlvTestFrame>();
    auto CommonTLV = makeShared<commonHeader>();
    auto EndTLV = makeShared<tlvHeader>();

    TestMgrTLV->setSubType(subTlvHeaderType::TEST_MGR_NACK);
    TestMgrTLV->setPrio(managerPrio);
    TestMgrTLV->setSa(sourceAddress);
    TestMgrTLV->setOtherMRMPrio(ManagerPrio);
    TestMgrTLV->setOtherMRMSa(SourceAddress);

    OptionTLV->setHeaderLength(OptionTLV->getHeaderLength()+TestMgrTLV->getSubHeaderLength()+2);

    CommonTLV->setSequenceID(sequenceID);
    sequenceID++;
    CommonTLV->setUuid0(domainID.uuid0);
    CommonTLV->setUuid1(domainID.uuid1);

    Packet* packet1 = new Packet("mrpTestMgrNackFrame");
    packet1->insertAtBack(Version);
    packet1->insertAtBack(OptionTLV);
    packet1->insertAtBack(TestMgrTLV);
    packet1->insertAtBack(CommonTLV);
    packet1->insertAtBack(EndTLV);

    Packet* packet2 = new Packet("mrpTestMgrNackFrame");
    packet2->insertAtBack(Version);
    packet2->insertAtBack(OptionTLV);
    packet2->insertAtBack(TestMgrTLV);
    packet2->insertAtBack(CommonTLV);
    packet2->insertAtBack(EndTLV);

    MacAddress SourceAddress1 = getPortNetworkInterface(primaryRingPort)->getMacAddress();
    sendFrameReq(primaryRingPort, static_cast<MacAddress>(MC_TEST), SourceAddress1, priority, MRP_LT , packet1);
    MacAddress SourceAddress2 = getPortNetworkInterface(secondaryRingPort)->getMacAddress();
    sendFrameReq(secondaryRingPort, static_cast<MacAddress>(MC_TEST), SourceAddress2, priority, MRP_LT ,packet2);
}

void MediaRedundancyNode::testPropagateReq(mrpPriority ManagerPrio, MacAddress SourceAddress)
{
    //Create MRP-PDU according MRP_Option and Suboption2 MRP-TestPropagate
    auto Version = makeShared<mrpVersionField>();
    auto OptionTLV = makeShared<optionHeader>();
    auto TestMgrTLV = makeShared<subTlvTestFrame>();
    auto CommonTLV = makeShared<commonHeader>();
    auto EndTLV = makeShared<tlvHeader>();

    TestMgrTLV->setPrio(managerPrio);
    TestMgrTLV->setSa(sourceAddress);
    TestMgrTLV->setOtherMRMPrio(ManagerPrio);
    TestMgrTLV->setOtherMRMSa(SourceAddress);

    OptionTLV->setHeaderLength(OptionTLV->getHeaderLength()+TestMgrTLV->getSubHeaderLength()+2);

    CommonTLV->setSequenceID(sequenceID);
    sequenceID++;
    CommonTLV->setUuid0(domainID.uuid0);
    CommonTLV->setUuid1(domainID.uuid1);

    Packet* packet1 = new Packet("mrpTestPropagateFrame");
    packet1->insertAtBack(Version);
    packet1->insertAtBack(OptionTLV);
    packet1->insertAtBack(TestMgrTLV);
    packet1->insertAtBack(CommonTLV);
    packet1->insertAtBack(EndTLV);

    Packet* packet2 = new Packet("mrpTestPropagateFrame");
    packet2->insertAtBack(Version);
    packet2->insertAtBack(OptionTLV);
    packet2->insertAtBack(TestMgrTLV);
    packet2->insertAtBack(CommonTLV);
    packet2->insertAtBack(EndTLV);

    MacAddress SourceAddress1 = getPortNetworkInterface(primaryRingPort)->getMacAddress();
    sendFrameReq(primaryRingPort, static_cast<MacAddress>(MC_TEST), SourceAddress1, priority, MRP_LT , packet1);
    MacAddress SourceAddress2 = getPortNetworkInterface(secondaryRingPort)->getMacAddress();
    sendFrameReq(secondaryRingPort, static_cast<MacAddress>(MC_TEST), SourceAddress2, priority, MRP_LT ,packet2);
}

void MediaRedundancyNode::testRingInd(MacAddress SourceAddress, mrpPriority ManagerPrio)
{
    switch (currentState)
    {
    case POWER_ON:
    case AC_STAT1:
        break;
    case PRM_UP:
        if(SourceAddress == sourceAddress){
            testMaxRetransmissionCount = testMonitoringCount -1;
            testRetransmissionCount = 0;
            noTopologyChange = false;
            testRingReq(defaultTestInterval);
            currentState = CHK_RC;
            EV_INFO << "Switching State from PRM_UP to CHK_RC" << EV_FIELD(currentState) << EV_ENDL;
            currentRingState = CLOSED;
            emit(RingStateChangedSignal, simTime().inUnit(SIMTIME_US));
        }
        else if(expectedRole == MANAGER_AUTO && (ManagerPrio > managerPrio || (ManagerPrio == managerPrio && SourceAddress >sourceAddress))){
            testMgrNackReq(ManagerPrio,SourceAddress);
        }
        //all other cases: ignore
        break;
    case CHK_RO:
        if (SourceAddress ==sourceAddress){
            setPortState(secondaryRingPort, MrpInterfaceData::BLOCKED);
            testMaxRetransmissionCount = testMonitoringCount -1;
            testRetransmissionCount = 0;
            noTopologyChange= false;
            testRingReq(defaultTestInterval);
            if(!reactOnLinkChange){
                topologyChangeReq(topologyChangeInterval);
            }
            else{
                double Time=0;
                topologyChangeReq(Time);
            }
            currentState= CHK_RC;
            EV_INFO << "Switching State from CHK_RO to CHK_RC" << EV_FIELD(currentState) << EV_ENDL;
            currentRingState = CLOSED;
            emit(RingStateChangedSignal, simTime().inUnit(SIMTIME_US));
        }
        else if (expectedRole == MANAGER_AUTO && (ManagerPrio > managerPrio || (ManagerPrio == managerPrio && SourceAddress >sourceAddress))){
            testMgrNackReq(ManagerPrio, SourceAddress);
        }
        //all other cases: ignore
        break;
    case CHK_RC:
        if (SourceAddress == sourceAddress){
            testMaxRetransmissionCount = testMonitoringCount -1;
            testRetransmissionCount = 0;
            noTopologyChange= false;
        }
        else if (expectedRole == MANAGER_AUTO && (ManagerPrio > managerPrio || (ManagerPrio == managerPrio && SourceAddress >sourceAddress))){
            testMgrNackReq(ManagerPrio, SourceAddress);
        }
        break;
    case DE:
    case DE_IDLE:
    case PT:
    case PT_IDLE:
        if (expectedRole == MANAGER_AUTO && SourceAddress != sourceAddress && SourceAddress == hostBestMRMSourceAddress){
            if (ManagerPrio < managerPrio || (ManagerPrio == managerPrio && SourceAddress < sourceAddress)){
                monNReturn =0;
            }
            hostBestMRMPriority = ManagerPrio;
        }
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void MediaRedundancyNode::topologyChangeInd(MacAddress SourceAddress, double Time)
{
    switch (currentState)
    {
    case POWER_ON:
    case AC_STAT1:
        break;
    case PRM_UP:
    case CHK_RO:
    case CHK_RC:
        if(SourceAddress != sourceAddress){
            clearFDB(Time);
        }
        break;
    case PT:
        linkChangeCount = linkMaxChange;
        cancelEvent(linkUpTimer);
        setPortState(secondaryRingPort, MrpInterfaceData::FORWARDING);
        clearFDB(Time);
        currentState = PT_IDLE;
        EV_INFO << "Switching State from PT to PT_IDLE" << EV_FIELD(currentState) << EV_ENDL;
        break;
    case DE:
        linkChangeCount = linkMaxChange;
        cancelEvent(linkDownTimer);
        clearFDB(Time);
        currentState=DE_IDLE;
        EV_INFO << "Switching State from DE to DE_IDLE" << EV_FIELD(currentState) << EV_ENDL;
        break;
    case DE_IDLE:
        clearFDB(Time);
        if (expectedRole == MANAGER_AUTO && linkUpHysterisisTimer->isScheduled()){
            setPortState(secondaryRingPort, MrpInterfaceData::FORWARDING);
            currentState=PT_IDLE;
            EV_INFO << "Switching State from DE_IDLE to PT_IDLE" << EV_FIELD(currentState) << EV_ENDL;
        }
    case PT_IDLE:
        clearFDB(Time);
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void MediaRedundancyNode::linkChangeInd(uint16_t PortState, uint16_t LinkState)
{
    switch (currentState)
    {
    case POWER_ON:
    case AC_STAT1:
    case DE_IDLE:
    case PT:
    case DE:
    case PT_IDLE:
        break;
    case PRM_UP:
        if (!addTest) {
            if(nonBlockingMRC){ //15
                addTest = true;
                testRingReq(shortTestInterval);
                break;
            }
            else if (LinkState == NetworkInterface::UP){
                addTest = true;
                testRingReq(shortTestInterval);
                double Time =0;
                topologyChangeReq(Time);
                break;
            }
        }else{
            if(!nonBlockingMRC && LinkState == NetworkInterface::UP){ //18
                double Time =0;
                topologyChangeReq(Time);
            }
            break;
        }
        //all other cases : ignore
        break;
    case CHK_RO:
        if (!addTest){
            if (LinkState ==NetworkInterface::DOWN){
                addTest = true;
                testRingReq(shortTestInterval);
                break;
            }
            else if (LinkState == NetworkInterface::UP){
                if (nonBlockingMRC){
                    addTest =true;
                    testRingReq(shortTestInterval);
                }
                else{
                    setPortState(secondaryRingPort, MrpInterfaceData::BLOCKED);
                    testMaxRetransmissionCount = testMonitoringExtendedCount - 1;
                    testRetransmissionCount = 0;
                    addTest=true;
                    testRingReq(shortTestInterval);
                    double Time = 0;
                    topologyChangeReq(Time);
                    currentState = CHK_RC;
                    currentRingState = CLOSED;
                    emit(RingStateChangedSignal, simTime().inUnit(SIMTIME_US));
                    EV_INFO << "Switching State from CHK_RO to CHK_RC" << EV_FIELD(currentState) << EV_ENDL;
                }
            }
        }
        else{
            if (!nonBlockingMRC && LinkState == NetworkInterface::UP){
                setPortState(secondaryRingPort, MrpInterfaceData::BLOCKED);
                testMaxRetransmissionCount = testMonitoringExtendedCount - 1;
                testRetransmissionCount = 0;
                testRingReq(defaultTestInterval);
                double Time = 0;
                topologyChangeReq(Time);
                currentState = CHK_RC;
                currentRingState = CLOSED;
                emit(RingStateChangedSignal, simTime().inUnit(SIMTIME_US));
                EV_INFO << "Switching State from CHK_RO to CHK_RC" << EV_FIELD(currentState) << EV_ENDL;
            }
        }
        //all other cases: ignore
        break;
    case CHK_RC:
        if (reactOnLinkChange){
            if (LinkState == NetworkInterface::DOWN){
                setPortState(secondaryRingPort, MrpInterfaceData::FORWARDING);
                double Time = 0;
                topologyChangeReq(Time);
                currentState = CHK_RO;
                currentRingState = OPEN;
                emit(RingStateChangedSignal, simTime().inUnit(SIMTIME_US));
                EV_INFO << "Switching State from CHK_RC to CHK_RO" << EV_FIELD(currentState) << EV_ENDL;
                break;
            }
            else if (LinkState ==NetworkInterface::UP){
                if(nonBlockingMRC){
                    testMaxRetransmissionCount = testMonitoringCount - 1;
                }
                else{
                    testMaxRetransmissionCount = testMonitoringExtendedCount - 1;
                }
                double Time = 0;
                topologyChangeReq(Time);
            }
        }
        else if (nonBlockingMRC){
            if (!addTest){
                addTest = true;
                testRingReq(shortTestInterval);
            }
        }
        break;
    default:
        throw cRuntimeError("Unknown state of MRM");
    }
}

void MediaRedundancyNode::testMgrNackInd(int RingPort, MacAddress SourceAddress, mrpPriority ManagerPrio, MacAddress BestMRMSourceAddress)
{
    switch (currentState){
    case POWER_ON:
    case AC_STAT1:
    case DE:
    case DE_IDLE:
    case PT:
    case PT_IDLE:
        break;
    case PRM_UP:
        if (expectedRole == MANAGER_AUTO && SourceAddress != sourceAddress && BestMRMSourceAddress == sourceAddress){
            if (ManagerPrio < hostBestMRMPriority || (ManagerPrio == hostBestMRMPriority && SourceAddress < sourceAddress)){
                hostBestMRMSourceAddress = SourceAddress;
                hostBestMRMPriority = ManagerPrio;
            }
            cancelEvent(topologyChangeTimer);
            mrcInit();
            testPropagateReq(hostBestMRMPriority, hostBestMRMSourceAddress);
            currentState= DE_IDLE;
            EV_INFO << "Switching State from PRM_UP to DE_IDLE" << EV_FIELD(currentState) << EV_ENDL;
        }
        break;
    case CHK_RO:
        if (expectedRole == MANAGER_AUTO && SourceAddress != sourceAddress && BestMRMSourceAddress == sourceAddress){
            if (ManagerPrio < hostBestMRMPriority || (ManagerPrio == hostBestMRMPriority && SourceAddress < sourceAddress)){
                hostBestMRMSourceAddress = SourceAddress;
                hostBestMRMPriority = ManagerPrio;
            }
            cancelEvent(topologyChangeTimer);
            mrcInit();
            testPropagateReq(hostBestMRMPriority, hostBestMRMSourceAddress);
            currentState= PT_IDLE;
            EV_INFO << "Switching State from CHK_RO to PT_IDLE" << EV_FIELD(currentState) << EV_ENDL;
        }
        break;
    case CHK_RC:
        if (expectedRole== MANAGER_AUTO && SourceAddress != sourceAddress && BestMRMSourceAddress == sourceAddress){
            if (ManagerPrio < hostBestMRMPriority || (ManagerPrio == hostBestMRMPriority && SourceAddress < sourceAddress)){
                hostBestMRMSourceAddress = SourceAddress;
                hostBestMRMPriority = ManagerPrio;
            }
            cancelEvent(topologyChangeTimer);
            mrcInit();
            testPropagateReq(hostBestMRMPriority, hostBestMRMSourceAddress);
            setPortState(secondaryRingPort, MrpInterfaceData::FORWARDING);
            currentState= PT_IDLE;
            EV_INFO << "Switching State from CHK_RC to PT_IDLE" << EV_FIELD(currentState) << EV_ENDL;
        }
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void MediaRedundancyNode::testPropagateInd(int RingPort, MacAddress SourceAddress, mrpPriority ManagerPrio, MacAddress BestMRMSourceAddress, mrpPriority BestMRMPrio)
{
    switch (currentState){
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
        if (expectedRole== MANAGER_AUTO && SourceAddress != sourceAddress && SourceAddress == hostBestMRMSourceAddress){
            hostBestMRMSourceAddress = BestMRMSourceAddress;
            hostBestMRMPriority = BestMRMPrio;
            monNReturn = 0;
        }
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void MediaRedundancyNode::mauTypeChangeInd(int RingPort, uint16_t LinkState)
{
    switch (currentState)
    {
    case POWER_ON:
        //all cases: ignore
        break;
    case AC_STAT1:
        //Client
        if (expectedRole == CLIENT){
            if (LinkState == NetworkInterface::UP){
                if (RingPort == primaryRingPort){
                    setPortState(primaryRingPort, MrpInterfaceData::FORWARDING);
                    EV_INFO << "Switching State from AC_STAT1 to DE_IDLE" << EV_ENDL;
                    currentState=DE_IDLE;
                    break;
                }
                else if (RingPort == secondaryRingPort){
                    toggleRingPorts();
                    setPortState(primaryRingPort, MrpInterfaceData::FORWARDING);
                    EV_INFO << "Switching State from AC_STAT1 to DE_IDLE" << EV_ENDL;
                    currentState=DE_IDLE;
                    break;
                }
            }
            //all other cases: ignore
            break;
        }
        //Manager
        if (expectedRole == MANAGER || expectedRole == MANAGER_AUTO){
            if (LinkState == NetworkInterface::UP){
                if (RingPort == primaryRingPort){
                    setPortState(primaryRingPort, MrpInterfaceData::FORWARDING);
                    testRingReq(defaultTestInterval);
                    EV_INFO << "Switching State from AC_STAT1 to PRM_UP" << EV_ENDL;
                    currentState = PRM_UP;
                    currentRingState = OPEN;
                    emit(RingStateChangedSignal, simTime().inUnit(SIMTIME_US));
                    break;
                }
                else if (RingPort == secondaryRingPort){
                    toggleRingPorts();
                    setPortState(primaryRingPort, MrpInterfaceData::FORWARDING);
                    testRingReq(defaultTestInterval);
                    EV_INFO << "Switching State from AC_STAT1 to PRM_UP" << EV_ENDL;
                    currentState = PRM_UP;
                    currentRingState = OPEN;
                    emit(RingStateChangedSignal, simTime().inUnit(SIMTIME_US));
                    break;
                }
            }
            //all other cases: ignore
            break;
        }
        //all other roles: ignore
        break;
    case PRM_UP:
        if (RingPort == primaryRingPort && LinkState == NetworkInterface::DOWN){
            cancelEvent(testTimer);
            setPortState(primaryRingPort, MrpInterfaceData::BLOCKED);
            currentState = AC_STAT1;
            currentRingState = OPEN;
            emit(RingStateChangedSignal, simTime().inUnit(SIMTIME_US));
            EV_INFO << "Switching State from PRM_UP to AC_STAT1" << EV_FIELD(currentState) << EV_ENDL;
            break;
        }
        if (RingPort == secondaryRingPort && LinkState == NetworkInterface::UP){
            testMaxRetransmissionCount = testMonitoringCount -1;
            testRetransmissionCount = 0;
            noTopologyChange = true;
            testRingReq(defaultTestInterval);
            currentState = CHK_RC;
            currentRingState = CLOSED;
            emit(RingStateChangedSignal, simTime().inUnit(SIMTIME_US));
            EV_INFO << "Switching State from PRM_UP to CHK_RC" << EV_FIELD(currentState) << EV_ENDL;
            break;
        }
        //all other Cases: ignore
        break;
    case CHK_RO:
        if (LinkState == NetworkInterface::DOWN){
            if (RingPort == primaryRingPort){
                toggleRingPorts();
                setPortState(secondaryRingPort,MrpInterfaceData::BLOCKED);
                testRingReq(defaultTestInterval);
                topologyChangeReq(topologyChangeInterval);
                currentState= PRM_UP;
                currentRingState = OPEN;
                emit(RingStateChangedSignal, simTime().inUnit(SIMTIME_US));
                EV_INFO << "Switching State from CHK_RO to PRM_UP" << EV_FIELD(currentState) << EV_ENDL;
                break;
            }
            if (RingPort == secondaryRingPort){
                setPortState(secondaryRingPort, MrpInterfaceData::BLOCKED);
                currentState = PRM_UP;
                currentRingState = OPEN;
                emit(RingStateChangedSignal, simTime().inUnit(SIMTIME_US));
                EV_INFO << "Switching State from CHK_RO to PRM_UP" << EV_FIELD(currentState) << EV_ENDL;
                break;
            }
        }
        //all other cases: ignore
        break;
    case CHK_RC:
        if (LinkState == NetworkInterface::DOWN){
            if (RingPort == primaryRingPort){
                toggleRingPorts();
                testRingReq(defaultTestInterval);
                topologyChangeReq(topologyChangeInterval);
                currentState=PRM_UP;
                currentRingState = OPEN;
                emit(RingStateChangedSignal, simTime().inUnit(SIMTIME_US));
                EV_INFO << "Switching State from CHK_RC to PRM_UP" << EV_FIELD(currentState) << EV_ENDL;
                break;
            }
            if (RingPort == secondaryRingPort){
                currentState= PRM_UP;
                currentRingState = OPEN;
                emit(RingStateChangedSignal, simTime().inUnit(SIMTIME_US));
                EV_INFO << "Switching State from CHK_RC to PRM_UP" << EV_FIELD(currentState) << EV_ENDL;
            }
        }
        //all other Cases: ignore
        break;
    case DE_IDLE:
        if (RingPort == secondaryRingPort && LinkState == NetworkInterface::UP){
            linkChangeReq(primaryRingPort, NetworkInterface::UP);
            currentState=PT;
            EV_INFO << "Switching State from DE_IDLE to PT" << EV_FIELD(currentState) << EV_ENDL;
        }
        if (RingPort == primaryRingPort && LinkState == NetworkInterface::DOWN){
            setPortState(primaryRingPort, MrpInterfaceData::BLOCKED);
            currentState=AC_STAT1;
            EV_INFO << "Switching State from DE_IDLE to AC_STAT1" << EV_FIELD(currentState) << EV_ENDL;
        }
        //all other cases: ignore
        break;
    case PT:
        if (LinkState == NetworkInterface::DOWN){
            if (RingPort == secondaryRingPort){
                cancelEvent(linkUpTimer);
                setPortState(secondaryRingPort, MrpInterfaceData::BLOCKED);
                linkChangeReq(primaryRingPort,NetworkInterface::DOWN);
                currentState=DE;
                EV_INFO << "Switching State from PT to DE" << EV_FIELD(currentState) << EV_ENDL;
                break;
            }
            if (RingPort == primaryRingPort){
                cancelEvent(linkUpTimer);
                toggleRingPorts();
                setPortState(primaryRingPort, MrpInterfaceData::FORWARDING);
                setPortState(secondaryRingPort, MrpInterfaceData::BLOCKED);
                linkChangeReq(primaryRingPort,NetworkInterface::DOWN);
                currentState=DE;
                EV_INFO << "Switching State from PT to DE" << EV_FIELD(currentState) << EV_ENDL;
                break;
            }
        }
        //all other cases: ignore
        break;
    case DE:
        if (RingPort == secondaryRingPort && LinkState == NetworkInterface::UP){
            cancelEvent(linkDownTimer);
            linkChangeReq(primaryRingPort,NetworkInterface::UP);
            currentState=PT;
            EV_INFO << "Switching State from DE to PT" << EV_FIELD(currentState) << EV_ENDL;
            break;
        }
        if (RingPort == primaryRingPort && LinkState == NetworkInterface::DOWN){
            linkChangeCount = linkMaxChange;
            setPortState(primaryRingPort, MrpInterfaceData::BLOCKED);
            currentState=AC_STAT1;
            EV_INFO << "Switching State from DE to AC_STAT1" << EV_FIELD(currentState) << EV_ENDL;
        }
        //all other cases: ignore
        break;
    case PT_IDLE:
        if (LinkState == NetworkInterface::DOWN){
            if (RingPort == secondaryRingPort){
                setPortState(secondaryRingPort, MrpInterfaceData::BLOCKED);
                linkChangeReq(primaryRingPort,NetworkInterface::DOWN);
                currentState = DE;
                EV_INFO << "Switching State from PT_IDLE to DE" << EV_FIELD(currentState) << EV_ENDL;
                break;
            }
            if (RingPort == primaryRingPort){
                primaryRingPort= secondaryRingPort;
                secondaryRingPort = RingPort;
                setPortState(secondaryRingPort, MrpInterfaceData::BLOCKED);
                linkChangeReq(primaryRingPort,NetworkInterface::DOWN);
                currentState = DE;
                EV_INFO << "Switching State from PT_IDLE to DE" << EV_FIELD(currentState) << EV_ENDL;
                break;
            }
        }
        //all other cases: ignore
        break;
    default:
        throw cRuntimeError("Unknown Node-State");
    }
}

void MediaRedundancyNode::interconnTopologyChangeInd(MacAddress SourceAddress, double Time, uint16_t InID, int RingPort, Packet* packet)
{
    if (expectedRole==CLIENT && interConnectionRingCheckAware){
        if (RingPort == primaryRingPort){
            interconnForwardReq(secondaryRingPort, packet);
        }
        else if (RingPort == secondaryRingPort){
            interconnForwardReq(primaryRingPort, packet);
        }
    }else{
        switch (currentState){
        case POWER_ON:
        case AC_STAT1:
        case DE_IDLE:
        case PT:
        case PT_IDLE:
            break;
        case PRM_UP:
        case CHK_RO:
        case CHK_RC:
            if(!topologyChangeTimer->isScheduled()){
                topologyChangeReq(Time);
            }
            break;
        default:
            throw cRuntimeError("Unknown NodeState");
        }
        delete packet;
    }
}

void MediaRedundancyNode::interconnLinkChangeInd(uint16_t InID, uint16_t Linkstate,int RingPort, Packet* packet)
{
    if ((expectedRole==CLIENT && interConnectionRingCheckAware) || currentState == CHK_RO){
        if (RingPort == primaryRingPort){
            interconnForwardReq(secondaryRingPort, packet);
        }
        else if (RingPort == secondaryRingPort){
            interconnForwardReq(primaryRingPort, packet);
        }
    }
    else{
        delete packet;
    }
}

void MediaRedundancyNode::interconnLinkStatusPollInd(uint16_t InID, int RingPort, Packet* packet)
{
    if ((expectedRole==CLIENT && interConnectionRingCheckAware) || currentState==CHK_RO){
        if (RingPort == primaryRingPort){
            interconnForwardReq(secondaryRingPort, packet);
        }
        else if (RingPort == secondaryRingPort){
            interconnForwardReq(primaryRingPort, packet);
        }
    }
    else{
        delete packet;
    }
}

void MediaRedundancyNode::interconnTestInd(MacAddress SourceAddress, int RingPort, uint16_t InID, Packet* packet)
{
    if ((expectedRole==CLIENT && interConnectionRingCheckAware) || currentState == CHK_RO){
        if (RingPort == primaryRingPort){
            interconnForwardReq(secondaryRingPort, packet);
        }
        else if (RingPort == secondaryRingPort){
            interconnForwardReq(primaryRingPort, packet);
        }
    }
    else{
        delete packet;
    }
}

void MediaRedundancyNode::interconnForwardReq(int RingPort, Packet* packet)
{
    auto macAddressInd = packet->getTag<MacAddressInd>();
    auto sourceAddress = macAddressInd->getSrcAddress();
    auto destinationAddress = macAddressInd->getDestAddress();
    packet->trim();
    packet->clearTags();
    sendFrameReq(RingPort, destinationAddress, sourceAddress, priority, MRP_LT ,packet);
}

void MediaRedundancyNode::sendFrameReq(int portId, const MacAddress& DestinationAddress, const MacAddress& SourceAddress,int Prio, uint16_t LT, Packet *MRPPDU)
{
    MRPPDU->addTag<InterfaceReq>()->setInterfaceId(portId);
    MRPPDU->addTag<PacketProtocolTag>()->setProtocol(&Protocol::mrp);
    MRPPDU->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ieee8022llc);
    auto macAddressReq = MRPPDU->addTag<MacAddressReq>();
    macAddressReq->setSrcAddress(SourceAddress);
    macAddressReq->setDestAddress(DestinationAddress);
    EV_INFO << "Sending packet down" << EV_FIELD(MRPPDU) <<EV_FIELD(DestinationAddress)<< EV_ENDL;
    send(MRPPDU, "relayOut");
}

void MediaRedundancyNode::sendCCM(int PortId, Packet *CCM)
{
    CCM->addTag<InterfaceReq>()->setInterfaceId(PortId);
    CCM->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ieee8021qCFM);
    CCM->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ieee8022llc);
    auto macAddressReq = CCM->addTag<MacAddressReq>();
    macAddressReq->setSrcAddress(getPortNetworkInterface(PortId)->getMacAddress());
    macAddressReq->setDestAddress(ccmMulticastAddress);
    EV_INFO << "Sending packet down" << EV_FIELD(CCM) << EV_ENDL;
    send(CCM, "relayOut");
}

void MediaRedundancyNode::handleStartOperation(LifecycleOperation *operation)
{
}

void MediaRedundancyNode::handleStopOperation(LifecycleOperation *operation)
{
    stop();
}

void MediaRedundancyNode::handleCrashOperation(LifecycleOperation *operation)
{
    stop();
}

void MediaRedundancyNode::colorLink(NetworkInterface *ie, bool forwarding) const
{
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
            }
            else {
                outGatePrev->getDisplayString().setTagArg("ls", 0, DISABLED_LINK_COLOR);
                inGate->getDisplayString().setTagArg("ls", 0, DISABLED_LINK_COLOR);
            }

            if ((!inGatePrev2->getDisplayString().containsTag("ls") || strcmp(inGatePrev2->getDisplayString().getTagArg("ls", 0), ENABLED_LINK_COLOR) == 0) && forwarding) {
                outGate->getDisplayString().setTagArg("ls", 0, ENABLED_LINK_COLOR);
                inGatePrev->getDisplayString().setTagArg("ls", 0, ENABLED_LINK_COLOR);
            }
            else {
                outGate->getDisplayString().setTagArg("ls", 0, DISABLED_LINK_COLOR);
                inGatePrev->getDisplayString().setTagArg("ls", 0, DISABLED_LINK_COLOR);
            }
        }
    }
}

void MediaRedundancyNode::refreshDisplay() const
{
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
            }
            else {
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


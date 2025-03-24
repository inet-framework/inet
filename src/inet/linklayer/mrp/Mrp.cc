//
// Copyright (C) 2024 Daniel Zeitler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

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
#include "inet/linklayer/ethernet/common/Ethernet.h"
#include "inet/linklayer/mrp/Timers_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

static const char *ENABLED_LINK_COLOR = "#000000";
static const char *DISABLED_LINK_COLOR = "#888888";

Define_Module(Mrp);

Register_Enum(Mrp::RingState, (Mrp::OPEN, Mrp::CLOSED, Mrp::UNDEFINED));
Register_Enum(Mrp::NodeState, (Mrp::POWER_ON, Mrp::AC_STAT1, Mrp::PRM_UP, Mrp::CHK_RO, Mrp::CHK_RC, Mrp::DE_IDLE, Mrp::PT, Mrp::DE, Mrp::PT_IDLE));

Mrp::Mrp()
{
}

Mrp::~Mrp()
{
    cancelAndDelete(linkDownTimer);
    cancelAndDelete(linkUpTimer);
    cancelAndDelete(fdbClearTimer);
    cancelAndDelete(fdbClearDelay);
    cancelAndDelete(topologyChangeTimer);
    cancelAndDelete(testTimer);
    cancelAndDelete(startUpTimer);
    cancelAndDelete(linkUpHysteresisTimer);
}

int Mrp::resolveInterfaceIndex(int interfaceIndex)
{
    NetworkInterface *interface = interfaceTable->getInterface(interfaceIndex);
    if (interface->getProtocol() != &Protocol::ethernetMac)
        throw cRuntimeError("Chosen interface %d is not an Ethernet interface", interfaceIndex);
    return interface->getInterfaceId();
}

void Mrp::setPortState(int interfaceId, MrpInterfaceData::PortState state)
{
    auto portData = getPortInterfaceDataForUpdate(interfaceId);
    portData->setState(state);
    simsignal_t signal = interfaceId == primaryRingPortId ? ringPort1StateChangedSignal :
                         interfaceId == secondaryRingPortId ? ringPort2StateChangedSignal :
                         SIMSIGNAL_NULL;
    if (signal != SIMSIGNAL_NULL)
        emit(signal, state);
    EV_INFO << "Setting Port State" << EV_FIELD(interfaceId) << EV_FIELD(state) << EV_ENDL;
}

void Mrp::setPortRole(int interfaceId, MrpInterfaceData::PortRole role)
{
    getPortInterfaceDataForUpdate(interfaceId)->setRole(role);
}

MrpInterfaceData::PortState Mrp::getPortState(int interfaceId) const
{
    return getPortInterfaceData(interfaceId)->getState();
}

MrpInterfaceData::PortRole Mrp::getPortRole(int interfaceId) const
{
    return getPortInterfaceData(interfaceId)->getRole();
}

const MrpInterfaceData* Mrp::getPortInterfaceData(int interfaceId) const
{
    return getPortNetworkInterface(interfaceId)->getProtocolData<MrpInterfaceData>();
}

MrpInterfaceData* Mrp::getPortInterfaceDataForUpdate(int interfaceId)
{
    return getPortNetworkInterface(interfaceId)->getProtocolDataForUpdate<MrpInterfaceData>();
}

NetworkInterface* Mrp::getPortNetworkInterface(int interfaceId) const
{
    NetworkInterface *gateIfEntry = interfaceTable->getInterfaceById(interfaceId);
    if (!gateIfEntry)
        throw cRuntimeError("gate's Interface is nullptr");
    return gateIfEntry;
}

void Mrp::toggleRingPorts()
{
    int RingPort = secondaryRingPortId;
    secondaryRingPortId = primaryRingPortId;
    primaryRingPortId = RingPort;
    setPortRole(primaryRingPortId, MrpInterfaceData::PRIMARY);
    setPortRole(secondaryRingPortId, MrpInterfaceData::SECONDARY);
}

void Mrp::initialize(int stage)
{
    OperationalBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        //modules
        mrpMacForwardingTable.reference(this, "macTableModule", true);
        interfaceTable.reference(this, "interfaceTableModule", true);
        switchModule = getContainingNode(this);
        relay.reference(this, "mrpRelayModule", true);

        //parameters
        visualize = par("visualize");
        role = parseMrpRole(par("mrpRole"));
        domainID.uuid0 = par("uuid0");
        domainID.uuid1 = par("uuid1");
        timingProfile = par("timingProfile");
        ccmInterval = par("ccmInterval");
        interconnectionLinkCheckAware = par("interconnectionLinkCheckAware");
        interconnectionRingCheckAware = par("interconnectionRingCheckAware");
        enableLinkCheckOnRing = par("enableLinkCheckOnRing");
        linkDetectionDelayPar = &par("linkDetectionDelay");
        processingDelayPar = &par("processingDelay");

        //manager variables
        localManagerPrio = static_cast<MrpPriority>(par("mrpPriority").intValue());
        nonblockingMrcSupported = par("nonblockingMrcSupported");
        reactOnLinkChange = par("reactOnLinkChange");

        //signals
        simsignal_t ringStateChangedSignal = registerSignal("ringStateChanged");
        simsignal_t nodeStateChangedSignal = registerSignal("nodeStateChanged");
        ringState.addEmitCallback(this, ringStateChangedSignal);
        nodeState.addEmitCallback(this, nodeStateChangedSignal);

        ringPort1StateChangedSignal = registerSignal("ringPort1StateChanged");
        ringPort2StateChangedSignal = registerSignal("ringPort2StateChanged");
        topologyChangeAnnouncedSignal = registerSignal("topologyChangeAnnounced");
        fdbClearedSignal = registerSignal("fdbCleared");
        linkChangeDetectedSignal = registerSignal("linkChangeDetected");
        testFrameLatencySignal = registerSignal("testFrameLatency");

        switchModule->subscribe(interfaceStateChangedSignal, this);
    }
    if (stage == INITSTAGE_LINK_LAYER) { // "auto" MAC addresses assignment takes place in stage 0
        registerProtocol(Protocol::mrp, gate("relayOut"), gate("relayIn"), nullptr, nullptr);
        registerProtocol(Protocol::ieee8021qCFM, gate("relayOut"), gate("relayIn"), nullptr, nullptr);
        initPortTable();
        primaryRingPortId = resolveInterfaceIndex(par("ringPort1"));
        secondaryRingPortId = resolveInterfaceIndex(par("ringPort2"));
        initRingPort(primaryRingPortId, MrpInterfaceData::PRIMARY, enableLinkCheckOnRing);
        initRingPort(secondaryRingPortId, MrpInterfaceData::SECONDARY, enableLinkCheckOnRing);
        localBridgeAddress = relay->getBridgeAddress();
        EV_DETAIL << "Initialize MRP link layer" << EV_ENDL;
        linkUpHysteresisTimer = new cMessage("linkUpHysteresisTimer");
        startUpTimer = new cMessage("startUpTimer");
        scheduleAt(simTime(), startUpTimer);
    }
}

void Mrp::initPortTable()
{
    EV_DEBUG << "MRP Interface Data initialization. Setting port infos to the protocol defaults." << EV_ENDL;
    for (unsigned int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
        auto ie = interfaceTable->getInterface(i);
        if (ie->getProtocol() == &Protocol::ethernetMac) {
            initInterfacedata(ie->getInterfaceId());
        }
    }
}

void Mrp::initInterfacedata(int interfaceId)
{
    setPortRole(interfaceId, MrpInterfaceData::NOTASSIGNED);
    setPortState(interfaceId, MrpInterfaceData::FORWARDING);
    auto ifd = getPortInterfaceDataForUpdate(interfaceId);
    ifd->setContinuityCheck(false);
    ifd->setContinuityCheckInterval(ccmInterval);
}

void Mrp::initRingPort(int interfaceId, MrpInterfaceData::PortRole role, bool enableLinkCheck)
{
    setPortRole(interfaceId, role);
    setPortState(interfaceId, MrpInterfaceData::BLOCKED);
    auto ifd = getPortInterfaceDataForUpdate(interfaceId);
    ifd->setContinuityCheck(enableLinkCheck);
    ifd->setContinuityCheckInterval(ccmInterval);
}

void Mrp::startContinuityCheck()
{
    for (unsigned int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
        auto ie = interfaceTable->getInterface(i);
        if (ie->getProtocol() == &Protocol::ethernetMac) {
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
                scheduleAfter(portData->getContinuityCheckInterval(), checkTimer);
                EV_DEBUG << "Next CCM-Interval:" << EV_FIELD(simTime() + portData->getContinuityCheckInterval()) << EV_ENDL;
            }
        }
    }
    relay->registerAddress(MacAddress::CFM_CCM_MULTICAST_ADDRESS);
}

Mrp::MrpRole Mrp::parseMrpRole(const char *mrpRole) const
{
    if (!strcmp(mrpRole, "disabled"))
        return DISABLED;
    else if (!strcmp(mrpRole, "MRC"))
        return CLIENT;
    else if (!strcmp(mrpRole, "MRM"))
        return MANAGER;
    else if (!strcmp(mrpRole, "MRA"))
        return MANAGER_AUTO;
    else if (!strcmp(mrpRole, "MRAc"))
        return MANAGER_AUTO_COMP;
    else
        throw cRuntimeError("Unknown MRP role '%s'", mrpRole);
}

void Mrp::setTimingProfile(int maxRecoveryTime)
{
    //maxrecoverytime in ms,
    switch (maxRecoveryTime) {
    case 500:
        topologyChangeInterval = SimTime(20, SIMTIME_MS);
        shortTestInterval = SimTime(30, SIMTIME_MS);
        defaultTestInterval = SimTime(50, SIMTIME_MS);
        testMonitoringCount = 5;
        linkUpInterval = SimTime(20, SIMTIME_MS);
        linkDownInterval = SimTime(20, SIMTIME_MS);
        break;
    case 200:
        topologyChangeInterval = SimTime(10, SIMTIME_MS);
        shortTestInterval = SimTime(10, SIMTIME_MS);
        defaultTestInterval = SimTime(20, SIMTIME_MS);
        testMonitoringCount = 3;
        linkUpInterval = SimTime(20, SIMTIME_MS);
        linkDownInterval = SimTime(20, SIMTIME_MS);
        break;
    case 30:
        topologyChangeInterval = SimTime(500, SIMTIME_US);
        shortTestInterval = SimTime(1, SIMTIME_MS);
        defaultTestInterval = SimTime(3500, SIMTIME_US);
        testMonitoringCount = 3;
        linkUpInterval = SimTime(3, SIMTIME_MS);
        linkDownInterval = SimTime(3, SIMTIME_MS);
        break;
    case 10:
        topologyChangeInterval = SimTime(500, SIMTIME_US);
        shortTestInterval = SimTime(500, SIMTIME_US);
        defaultTestInterval = SimTime(1, SIMTIME_MS);
        testMonitoringCount = 3;
        linkUpInterval = SimTime(1, SIMTIME_MS);
        linkDownInterval = SimTime(1, SIMTIME_MS);
        break;
    default:
        throw cRuntimeError("Only RecoveryTimes 500, 200, 30 and 10 ms are defined!");
    }
}

void Mrp::start()
{
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
    if (role == CLIENT) {
        mrcInit();
    } else if (role == MANAGER) {
        mrmInit();
    } else if (role == MANAGER_AUTO) {
        mraInit();
    }
}

void Mrp::stop()
{
    setPortRole(primaryRingPortId, MrpInterfaceData::NOTASSIGNED);
    setPortRole(secondaryRingPortId, MrpInterfaceData::NOTASSIGNED);
    setPortState(primaryRingPortId, MrpInterfaceData::DISABLED);
    setPortState(secondaryRingPortId, MrpInterfaceData::DISABLED);
    role = DISABLED;
    cancelAndDelete(fdbClearTimer);
    cancelAndDelete(fdbClearDelay);
    cancelAndDelete(topologyChangeTimer);
    cancelAndDelete(testTimer);
    cancelAndDelete(linkDownTimer);
    cancelAndDelete(linkUpTimer);
    cancelAndDelete(linkUpHysteresisTimer);
    cancelAndDelete(startUpTimer);
}

void Mrp::mrcInit()
{
    linkChangeCount = linkMaxChange;
    mrpMacForwardingTable->addMrpForwardingInterface(primaryRingPortId, MacAddress(MC_CONTROL), vlanID);
    mrpMacForwardingTable->addMrpForwardingInterface(secondaryRingPortId, MacAddress(MC_CONTROL), vlanID);
    mrpMacForwardingTable->addMrpForwardingInterface(primaryRingPortId, MacAddress(MC_TEST), vlanID);
    mrpMacForwardingTable->addMrpForwardingInterface(secondaryRingPortId, MacAddress(MC_TEST), vlanID);
    relay->registerAddress(MacAddress(MC_CONTROL));

    if (interconnectionRingCheckAware || interconnectionLinkCheckAware) {
        mrpMacForwardingTable->addMrpForwardingInterface(primaryRingPortId, MacAddress(MC_INCONTROL), vlanID);
        mrpMacForwardingTable->addMrpForwardingInterface(secondaryRingPortId, MacAddress(MC_INCONTROL), vlanID);
    }
    if (interconnectionRingCheckAware) {
        mrpMacForwardingTable->addMrpForwardingInterface(primaryRingPortId, MacAddress(MC_INTEST), vlanID);
        mrpMacForwardingTable->addMrpForwardingInterface(secondaryRingPortId, MacAddress(MC_INTEST), vlanID);
    }
    if (role == CLIENT) {
        ringState = UNDEFINED;
        nodeState = AC_STAT1; //only for client, managerAuto is keeping the current state
    }
    mauTypeChangeInd(primaryRingPortId, getPortNetworkInterface(primaryRingPortId)->getState());
    mauTypeChangeInd(secondaryRingPortId, getPortNetworkInterface(secondaryRingPortId)->getState());
}

void Mrp::mrmInit()
{
    localManagerPrio = DEFAULT;
    ringState = OPEN;
    addTest = false;
    testRetransmissionCount = 0;
    relay->registerAddress(MacAddress(MC_TEST));
    relay->registerAddress(MacAddress(MC_CONTROL));
    if (interconnectionRingCheckAware || interconnectionLinkCheckAware)
        relay->registerAddress(MacAddress(MC_INCONTROL));
    if (interconnectionRingCheckAware)
        relay->registerAddress(MacAddress(MC_INTEST));
    testMaxRetransmissionCount = testMonitoringCount - 1;
    testRetransmissionCount = 0;
    if (role == MANAGER)
        nodeState = AC_STAT1;
    if (role == MANAGER_AUTO) {
        //case: switching from Client to manager. in managerRole no Forwarding on RingPorts may take place
        mrpMacForwardingTable->removeMrpForwardingInterface(primaryRingPortId, MacAddress(MC_TEST), vlanID);
        mrpMacForwardingTable->removeMrpForwardingInterface(secondaryRingPortId, MacAddress(MC_TEST), vlanID);
        mrpMacForwardingTable->removeMrpForwardingInterface(primaryRingPortId, MacAddress(MC_CONTROL), vlanID);
        mrpMacForwardingTable->removeMrpForwardingInterface(secondaryRingPortId, MacAddress(MC_CONTROL), vlanID);
    }
    mauTypeChangeInd(primaryRingPortId, getPortNetworkInterface(primaryRingPortId)->getState());
    mauTypeChangeInd(secondaryRingPortId, getPortNetworkInterface(secondaryRingPortId)->getState());
}

void Mrp::mraInit()
{
    localManagerPrio = MRADEFAULT;
    ringState = OPEN;
    addTest = false;
    testRetransmissionCount = 0;
    relay->registerAddress(MacAddress(MC_TEST));
    relay->registerAddress(MacAddress(MC_CONTROL));
    if (interconnectionRingCheckAware || interconnectionLinkCheckAware)
        relay->registerAddress(MacAddress(MC_INCONTROL));
    if (interconnectionRingCheckAware)
        relay->registerAddress(MacAddress(MC_INTEST));
    testMaxRetransmissionCount = testMonitoringCount - 1;
    testRetransmissionCount = 0;
    addTest = false;
    reactOnLinkChange = false;
    hostBestMRMSourceAddress = MacAddress(0xFFFFFFFFFFFF);
    hostBestMRMPriority = static_cast<MrpPriority>(0xFFFF);
    monNReturn = 0;
    nodeState = AC_STAT1;
    mauTypeChangeInd(primaryRingPortId, getPortNetworkInterface(primaryRingPortId)->getState());
    mauTypeChangeInd(secondaryRingPortId, getPortNetworkInterface(secondaryRingPortId)->getState());
}

void Mrp::clearFDB(simtime_t time)
{
    if (!fdbClearTimer->isScheduled())
        scheduleAfter(time, fdbClearTimer);
    else if (fdbClearTimer->getArrivalTime() > (simTime() + time)) {
        cancelEvent(fdbClearTimer);
        scheduleAfter(time, fdbClearTimer);
    }
}

void Mrp::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
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
                simtime_t linkDetectionDelay;
                if (interface->isUp() && interface->hasCarrier())
                    linkDetectionDelay = SimTime(1, SIMTIME_US); //linkUP is handled faster than linkDown
                else
                    linkDetectionDelay = linkDetectionDelayPar->doubleValue();
                if (linkUpHysteresisTimer->isScheduled())
                    cancelEvent(linkUpHysteresisTimer);
                scheduleAfter(linkDetectionDelay, linkUpHysteresisTimer);
                scheduleAfter(linkDetectionDelay, delayTimer);
            }
        }
    }
}

void Mrp::handleMessageWhenUp(cMessage *msg)
{
    if (!msg->isSelfMessage()) {
        msg->setKind(2);
        EV_INFO << "Received Message on MrpNode, Rescheduling:" << EV_FIELD(msg) << EV_ENDL;
        simtime_t processingDelay = processingDelayPar->doubleValue();
        scheduleAfter(processingDelay, msg);
    }
    else {
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
                handleCfmContinuityCheckMessage(packet);
            }
            if (protocol == &Protocol::mrp) {
                handleMrpPDU(packet);
            }
        } else
            throw cRuntimeError("Unknown self-message received");
    }
}

void Mrp::handleMrpPDU(Packet* packet)
{
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
    offset = offset + B(firstTlv->getValueLength()) + B(2);
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
                simtime_t ringTime = simTime() - SimTime(testTlv->getTimeStamp(), SIMTIME_MS);
                auto it = testFrameSent.find(sequence);
                if (it != testFrameSent.end()) {
                    simtime_t ringTimePrecise = simTime() - it->second;
                    emit(testFrameLatencySignal, ringTimePrecise);
                    EV_DETAIL << "RingTime" << EV_FIELD(ringTime) << EV_FIELD(ringTimePrecise) << EV_ENDL;
                }
                else {
                    EV_DETAIL << "RingTime" << EV_FIELD(ringTime) << EV_ENDL;
                    emit(testFrameLatencySignal, ringTime);
                }
            }
            testRingInd(ringPort, testTlv->getSa(), static_cast<MrpPriority>(testTlv->getPrio()));
        }
        else {
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
                topologyChangeInd(topologyTlv->getSa(), SimTime(topologyTlv->getInterval(), SIMTIME_MS));
            }
            else {
                EV_DETAIL << "Received same Frame already" << EV_ENDL;
                delete packet;
            }
        }
        else {
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
            LinkState linkState = linkTlv->getHeaderType() == LINKDOWN ? LinkState::DOWN : LinkState::UP;
            linkChangeInd(linkState);
        }
        else {
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
                    && optionTlv->getValueLength() > 4)
                    || (optionTlv->getEd1Type() == 0x00
                            && optionTlv->getValueLength()
                                    > (4 + Ed1DataLength::LENGTH0))
                    || (optionTlv->getEd1Type() == 0x04
                            && optionTlv->getValueLength()
                                    > (4 + Ed1DataLength::LENGTH4))) {
                auto subTlv = packet->peekDataAt<MrpSubTlvHeader>(subOffset);
                switch (subTlv->getSubType()) {
                case RESERVED: {
                    auto subOptionTlv = dynamicPtrCast<const MrpManufacturerFkt>(subTlv);
                    //not implemented
                    break;
                }
                case TEST_MGR_NACK: {
                    if (role == MANAGER_AUTO) {
                        auto subOptionTlv = dynamicPtrCast<const MrpSubTlvTest>(subTlv);
                        testMgrNackInd(ringPort, subOptionTlv->getSa(), static_cast<MrpPriority>(subOptionTlv->getPrio()), subOptionTlv->getOtherMRMSa());
                    }
                    break;
                }
                case TEST_PROPAGATE: {
                    if (role == MANAGER_AUTO) {
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
        }
        else {
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
        interconnTopologyChangeInd(inTopologyTlv->getSa(), SimTime(inTopologyTlv->getInterval(), SIMTIME_MS), inTopologyTlv->getInID(), ringPort, packet->dup());
        break;
    }
    case INLINKDOWN:
    case INLINKUP: {
        EV_DETAIL << "Received inLinkChange-Frame" << EV_ENDL;
        auto inLinkTlv = dynamicPtrCast<const MrpInLinkChange>(firstTlv);
        LinkState linkState = inLinkTlv->getHeaderType() == INLINKDOWN ? LinkState::DOWN : LinkState::UP;
        interconnLinkChangeInd(inLinkTlv->getInID(), linkState, ringPort, packet->dup());
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
                && optionTlv->getValueLength() > 4)
                || (optionTlv->getEd1Type() == 0x00
                        && optionTlv->getValueLength()
                                > (4 + Ed1DataLength::LENGTH0))
                || (optionTlv->getEd1Type() == 0x04
                        && optionTlv->getValueLength()
                                > (4 + Ed1DataLength::LENGTH4))) {
            auto subTlv = packet->peekDataAt<MrpSubTlvHeader>(subOffset);
            switch (subTlv->getSubType()) {
            case RESERVED: {
                auto subOptionTlv = dynamicPtrCast<const MrpManufacturerFkt>(subTlv);
                //not implemented
                break;
            }
            case TEST_MGR_NACK: {
                if (role == MANAGER_AUTO) {
                    auto subOptionTlv = dynamicPtrCast<const MrpSubTlvTest>(subTlv);
                    testMgrNackInd(ringPort, subOptionTlv->getSa(), static_cast<MrpPriority>(subOptionTlv->getPrio()), subOptionTlv->getOtherMRMSa());
                }
                break;
            }
            case TEST_PROPAGATE: {
                if (role == MANAGER_AUTO) {
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
        offset = offset + B(thirdTlv->getValueLength()) + B(2);
    }
    //auto endTlv=Packet->peekDataAt<TlvHeader>(offset);
    delete packet;
}

void Mrp::handleCfmContinuityCheckMessage(Packet* packet)
{
    EV_DETAIL << "Handling CCM" << EV_ENDL;
    auto interfaceInd = packet->getTag<InterfaceInd>();
    auto macAddressInd = packet->getTag<MacAddressInd>();
    auto sourceAddress = macAddressInd->getSrcAddress();
    int ringPort = interfaceInd->getInterfaceId();
    auto incomingInterface = interfaceTable->getInterfaceById(ringPort);
    auto ccm = packet->popAtFront<CfmContinuityCheckMessage>();
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
    mauTypeChangeInd(ringPort, LinkState::UP);
    delete packet;
}

void Mrp::handleDelayTimer(int interfaceId, int field)
{
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
            mauTypeChangeInd(interfaceId, LinkState::UP);
        else
            mauTypeChangeInd(interfaceId, LinkState::DOWN);
    }
}

void Mrp::clearLocalFDB()
{
    EV_DETAIL << "clearing FDB" << EV_ENDL;
    if (fdbClearDelay->isScheduled())
        cancelEvent(fdbClearDelay);
    simtime_t processingDelay = processingDelayPar->doubleValue();
    scheduleAfter(processingDelay, fdbClearDelay);
}

void Mrp::clearLocalFDBDelayed()
{
    mrpMacForwardingTable->clearTable();
    emit(fdbClearedSignal, 1);
    EV_DETAIL << "FDB cleared" << EV_ENDL;
}

bool Mrp::isBetterThanOwnPrio(MrpPriority remotePrio, MacAddress remoteAddress)
{
    if (remotePrio < localManagerPrio)
        return true;
    if (remotePrio == localManagerPrio && remoteAddress < localBridgeAddress)
        return true;
    return false;
}

bool Mrp::isBetterThanBestPrio(MrpPriority remotePrio, MacAddress remoteAddress)
{
    if (remotePrio < hostBestMRMPriority)
        return true;
    if (remotePrio == hostBestMRMPriority && remoteAddress < hostBestMRMSourceAddress)
        return true;
    return false;
}

void Mrp::handleContinuityCheckTimer(int ringPort)
{
    auto portData = getPortInterfaceDataForUpdate(ringPort);
    EV_DETAIL << "Checktimer:" << EV_FIELD(simTime()) << EV_FIELD(ringPort)
                     << EV_FIELD(portData->getNextUpdate()) << EV_ENDL;
    if (simTime() >= portData->getNextUpdate()) {
        //no Message received within Lifetime
        EV_DETAIL << "Checktimer: Link considered down" << EV_ENDL;
        mauTypeChangeInd(ringPort, LinkState::DOWN);
    }
    setupContinuityCheck(ringPort);
    ContinuityCheckTimer *checkTimer = new ContinuityCheckTimer("continuityCheckTimer");
    checkTimer->setPort(ringPort);
    checkTimer->setKind(1);
    scheduleAfter(portData->getContinuityCheckInterval(), checkTimer);
}

void Mrp::handleTestTimer()
{
    switch (nodeState) {
    case POWER_ON:
    case AC_STAT1:
        if (role == MANAGER) {
            mauTypeChangeInd(primaryRingPortId, getPortNetworkInterface(primaryRingPortId)->getState());
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
            setPortState(secondaryRingPortId, MrpInterfaceData::FORWARDING);
            testMaxRetransmissionCount = testMonitoringCount - 1;
            testRetransmissionCount = 0;
            addTest = false;
            if (!noTopologyChange) {
                topologyChangeReq(topologyChangeInterval);
            }
            testRingReq(defaultTestInterval);
            nodeState = CHK_RO;
            EV_DETAIL << "Switching State from CHK_RC to CHK_RO" << EV_FIELD(nodeState) << EV_ENDL;
            ringState = OPEN;
        }
        else {
            testRetransmissionCount++;
            addTest = false;
            testRingReq(defaultTestInterval);
        }
        break;
    case DE:
    case DE_IDLE:
        if (role == MANAGER_AUTO) {
            scheduleAfter(shortTestInterval, testTimer);
            if (monNReturn <= monNRmax) {
                monNReturn++;
            }
            else {
                mrmInit();
                nodeState = PRM_UP;
                EV_DETAIL << "Switching State from DE_IDLE to PRM_UP" << EV_FIELD(nodeState) << EV_ENDL;
            }
        }
        break;
    case PT:
        if (role == MANAGER_AUTO) {
            scheduleAfter(shortTestInterval, testTimer);
            if (monNReturn <= monNRmax) {
                monNReturn++;
            }
            else {
                mrmInit();
                nodeState = CHK_RC;
                EV_DETAIL << "Switching State from PT to CHK_RC" << EV_FIELD(nodeState) << EV_ENDL;
            }
        }
        break;
    case PT_IDLE:
        if (role == MANAGER_AUTO) {
            scheduleAfter(shortTestInterval, testTimer);
            if (monNReturn <= monNRmax) {
                monNReturn++;
            }
            else {
                mrmInit();
                nodeState = CHK_RO;
                EV_DETAIL << "Switching State from PT_IDLE to CHK_RO" << EV_FIELD(nodeState) << EV_ENDL;
            }
        }
        break;
    default:
        throw cRuntimeError("Unknown Node State");
    }
}

void Mrp::handleTopologyChangeTimer()
{
    if (topologyChangeRepeatCount > 0) {
        setupTopologyChangeReq(topologyChangeRepeatCount * topologyChangeInterval);
        topologyChangeRepeatCount--;
        scheduleAfter(topologyChangeInterval, topologyChangeTimer);
    }
    else {
        topologyChangeRepeatCount = topologyChangeMaxRepeatCount - 1;
        clearLocalFDB();
    }
}

void Mrp::handleLinkUpTimer()
{
    switch (nodeState) {
    case PT:
        if (linkChangeCount == 0) {
            setPortState(secondaryRingPortId, MrpInterfaceData::FORWARDING);
            linkChangeCount = linkMaxChange;
            nodeState = PT_IDLE;
            EV_DETAIL << "Switching State from PT to PT_IDLE" << EV_FIELD(nodeState) << EV_ENDL;
        }
        else {
            linkChangeReq(primaryRingPortId, LinkState::UP);
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

void Mrp::handleLinkDownTimer()
{
    switch (nodeState) {
    case DE:
        if (linkChangeCount == 0) {
            linkChangeCount = linkMaxChange;
            nodeState = DE_IDLE;
            EV_DETAIL << "Switching State from DE to DE_IDLE" << EV_FIELD(nodeState) << EV_ENDL;
        }
        else {
            linkChangeReq(primaryRingPortId, LinkState::DOWN);
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

void Mrp::setupContinuityCheck(int ringPort)
{
    auto ccm = makeShared<CfmContinuityCheckMessage>();
    auto portData = getPortInterfaceDataForUpdate(ringPort);
    if (portData->getContinuityCheckInterval() == SimTime(3300, SIMTIME_US)) {  // SimTime(3.3, SIMTIME_MS) would be wrong, as in omnetpp-5.x this SimTime ctor only accepts integers
        ccm->setFlags(0x00000001);
    } else if (portData->getContinuityCheckInterval() == SimTime(10, SIMTIME_MS)) {
        ccm->setFlags(0x00000002);
    }
    if (ringPort == primaryRingPortId) {
        ccm->setSequenceNumber(sequenceCCM1);
        sequenceCCM1++;
    } else if (ringPort == secondaryRingPortId) {
        ccm->setSequenceNumber(sequenceCCM2);
        sequenceCCM2++;
    }
    ccm->setEndpointIdentifier(portData->getCfmEndpointID());
    auto name = portData->getCfmName();
    ccm->setMessageName(name.c_str());
    auto packet = new Packet("ContinuityCheck", ccm);
    sendCCM(ringPort, packet);
}

void Mrp::testRingReq(simtime_t time)
{
    if (addTest)
        cancelEvent(testTimer);
    if (!testTimer->isScheduled()) {
        scheduleAfter(time, testTimer);
        setupTestRingReq();
    } else
        EV_DETAIL << "Testtimer already scheduled" << EV_ENDL;
}

void Mrp::topologyChangeReq(simtime_t time)
{
    if (time == 0) {
        clearLocalFDB();
        setupTopologyChangeReq(time * topologyChangeMaxRepeatCount);
    } else if (!topologyChangeTimer->isScheduled()) {
        scheduleAfter(time, topologyChangeTimer);
        setupTopologyChangeReq(time * topologyChangeMaxRepeatCount);
    } else
        EV_DETAIL << "TopologyChangeTimer already scheduled" << EV_ENDL;

}

void Mrp::linkChangeReq(int ringPort, LinkState linkState)
{
    if (linkState == LinkState::DOWN) {
        if (!linkDownTimer->isScheduled()) {
            scheduleAfter(linkDownInterval, linkDownTimer);
            setupLinkChangeReq(primaryRingPortId, LinkState::DOWN, linkChangeCount * linkDownInterval);
            linkChangeCount--;
        }
    } else if (linkState == LinkState::UP) {
        if (!linkUpTimer->isScheduled()) {
            scheduleAfter(linkUpInterval, linkUpTimer);
            setupLinkChangeReq(primaryRingPortId, LinkState::UP, linkChangeCount * linkUpInterval);
            linkChangeCount--;
        }
    } else
        throw cRuntimeError("Unknown LinkState in LinkChangeReq");
}

void Mrp::setupTestRingReq()
{
    //Create MRP-PDU according MRP_Test
    auto version = makeShared<MrpVersion>();
    auto testTlv1 = makeShared<MrpTest>();
    auto testTlv2 = makeShared<MrpTest>();
    auto commonTlv = makeShared<MrpCommon>();
    auto endTlv = makeShared<MrpEnd>();

    auto timestamp = simTime().inUnit(SIMTIME_MS);
    testFrameSent.insert( { sequenceID, simTime() });

    testTlv1->setPrio(localManagerPrio);
    testTlv1->setSa(localBridgeAddress);
    testTlv1->setPortRole(MrpInterfaceData::PRIMARY);
    testTlv1->setRingState(ringState);
    testTlv1->setTransition(transition);
    testTlv1->setTimeStamp(timestamp);

    testTlv2->setPrio(localManagerPrio);
    testTlv2->setSa(localBridgeAddress);
    testTlv2->setPortRole(MrpInterfaceData::PRIMARY);
    testTlv2->setRingState(ringState);
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
    if (role == MANAGER_AUTO) {
        auto optionTlv = makeShared<MrpOption>();
        auto autoMgrTlv = makeShared<MrpSubTlvHeader>();
        uint8_t headerLength = optionTlv->getValueLength() + autoMgrTlv->getSubHeaderLength() + 2;
        optionTlv->setValueLength(headerLength);
        packet1->insertAtBack(optionTlv);
        packet1->insertAtBack(autoMgrTlv);
        packet2->insertAtBack(optionTlv);
        packet2->insertAtBack(autoMgrTlv);
    }
    packet1->insertAtBack(endTlv);
    MacAddress sourceAddress1 = getPortNetworkInterface(primaryRingPortId)->getMacAddress();
    sendFrameReq(primaryRingPortId, MacAddress(MC_TEST), sourceAddress1, priority, MRP_LT, packet1);

    packet2->insertAtBack(endTlv);
    MacAddress sourceAddress2 = getPortNetworkInterface(secondaryRingPortId)->getMacAddress();
    sendFrameReq(secondaryRingPortId, MacAddress(MC_TEST), sourceAddress2, priority, MRP_LT, packet2);
}

void Mrp::setupTopologyChangeReq(simtime_t interval)
{
    //Create MRP-PDU according MRP_TopologyChange
    auto version = makeShared<MrpVersion>();
    auto topologyChangeTlv = makeShared<MrpTopologyChange>();
    auto topologyChangeTlv2 = makeShared<MrpTopologyChange>();
    auto commonTlv = makeShared<MrpCommon>();
    auto endTlv = makeShared<MrpEnd>();

    topologyChangeTlv->setPrio(localManagerPrio);
    topologyChangeTlv->setSa(localBridgeAddress);
    topologyChangeTlv->setInterval(interval.inUnit(SIMTIME_MS));
    topologyChangeTlv2->setPrio(localManagerPrio);
    topologyChangeTlv2->setSa(localBridgeAddress);
    topologyChangeTlv2->setInterval(interval.inUnit(SIMTIME_MS));

    commonTlv->setSequenceID(sequenceID);
    sequenceID++;
    commonTlv->setUuid0(domainID.uuid0);
    commonTlv->setUuid1(domainID.uuid1);

    auto packet1 = new Packet("mrpTopologyChange");
    packet1->insertAtBack(version);
    packet1->insertAtBack(topologyChangeTlv);
    packet1->insertAtBack(commonTlv);
    packet1->insertAtBack(endTlv);
    MacAddress sourceAddress1 = getPortNetworkInterface(primaryRingPortId)->getMacAddress();
    sendFrameReq(primaryRingPortId, MacAddress(MC_CONTROL), sourceAddress1, priority, MRP_LT, packet1);

    auto packet2 = new Packet("mrpTopologyChange");
    packet2->insertAtBack(version);
    packet2->insertAtBack(topologyChangeTlv2);
    packet2->insertAtBack(commonTlv);
    packet2->insertAtBack(endTlv);
    MacAddress sourceAddress2 = getPortNetworkInterface(secondaryRingPortId)->getMacAddress();
    sendFrameReq(secondaryRingPortId, MacAddress(MC_CONTROL), sourceAddress2, priority, MRP_LT, packet2);
    emit(topologyChangeAnnouncedSignal, 1);
}

void Mrp::setupLinkChangeReq(int ringPort, LinkState linkState, simtime_t time)
{
    //Create MRP-PDU according MRP_LinkChange
    auto version = makeShared<MrpVersion>();
    auto linkChangeTlv = makeShared<MrpLinkChange>();
    auto commonTlv = makeShared<MrpCommon>();
    auto endTlv = makeShared<MrpEnd>();

    if (linkState == LinkState::UP) {
        linkChangeTlv->setHeaderType(LINKUP);
    } else if (linkState == LinkState::DOWN) {
        linkChangeTlv->setHeaderType(LINKDOWN);
    }
    else {
        throw cRuntimeError("Unknown LinkState in linkChangeRequest");
    }
    linkChangeTlv->setSa(localBridgeAddress);
    linkChangeTlv->setInterval(time.inUnit(SIMTIME_MS));
    linkChangeTlv->setBlocked(1);

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
    sendFrameReq(ringPort, MacAddress(MC_CONTROL), sourceAddress1, priority, MRP_LT, packet1);
    emit(linkChangeDetectedSignal, linkState == LinkState::DOWN ? 0 : 1);
}

void Mrp::testMgrNackReq(int ringPort, MrpPriority managerPrio, MacAddress sourceAddress)
{
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

    optionTlv->setValueLength(optionTlv->getValueLength() + testMgrTlv->getSubHeaderLength() + 2);

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
    //sendFrameReq(RingPort, MacAddress(MC_TEST), SourceAddress1, priority, MRP_LT , packet1);

    Packet *packet2 = new Packet("mrpTestMgrNackFrame");
    packet2->insertAtBack(version);
    packet2->insertAtBack(optionTlv);
    packet2->insertAtBack(testMgrTlv);
    packet2->insertAtBack(commonTlv);
    packet2->insertAtBack(endTlv);

    MacAddress sourceAddress1 = getPortNetworkInterface(primaryRingPortId)->getMacAddress();
    sendFrameReq(primaryRingPortId, MacAddress(MC_TEST), sourceAddress1, priority, MRP_LT, packet1);
    MacAddress sourceAddress2 = getPortNetworkInterface(secondaryRingPortId)->getMacAddress();
    sendFrameReq(secondaryRingPortId, MacAddress(MC_TEST), sourceAddress2, priority, MRP_LT, packet2);
}

void Mrp::testPropagateReq(int ringPort, MrpPriority managerPrio, MacAddress sourceAddress)
{
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

    optionTlv->setValueLength(optionTlv->getValueLength() + testMgrTlv->getSubHeaderLength() + 2);

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
    //sendFrameReq(RingPort, MacAddress(MC_TEST), SourceAddress1, priority, MRP_LT , packet1);

    Packet *packet2 = new Packet("mrpTestPropagateFrame");
    packet2->insertAtBack(version);
    packet2->insertAtBack(optionTlv);
    packet2->insertAtBack(testMgrTlv);
    packet2->insertAtBack(commonTlv);
    packet2->insertAtBack(endTlv);

    MacAddress sourceAddress1 = getPortNetworkInterface(primaryRingPortId)->getMacAddress();
    sendFrameReq(primaryRingPortId, MacAddress(MC_TEST), sourceAddress1, priority, MRP_LT, packet1);
    MacAddress sourceAddress2 = getPortNetworkInterface(secondaryRingPortId)->getMacAddress();
    sendFrameReq(secondaryRingPortId, MacAddress(MC_TEST), sourceAddress2, priority, MRP_LT, packet2);
}

void Mrp::testRingInd(int ringPort, MacAddress sourceAddress, MrpPriority managerPrio)
{
    switch (nodeState) {
    case POWER_ON:
    case AC_STAT1:
        break;
    case PRM_UP:
        if (sourceAddress == localBridgeAddress) {
            testMaxRetransmissionCount = testMonitoringCount - 1;
            testRetransmissionCount = 0;
            noTopologyChange = false;
            testRingReq(defaultTestInterval);
            nodeState = CHK_RC;
            EV_DETAIL << "Switching State from PRM_UP to CHK_RC" << EV_FIELD(nodeState) << EV_ENDL;
            ringState = CLOSED;
        } else if (role == MANAGER_AUTO
                && !isBetterThanOwnPrio(managerPrio, sourceAddress)) {
            testMgrNackReq(ringPort, managerPrio, sourceAddress);
        }
        //all other cases: ignore
        break;
    case CHK_RO:
        if (sourceAddress == localBridgeAddress) {
            setPortState(secondaryRingPortId, MrpInterfaceData::BLOCKED);
            testMaxRetransmissionCount = testMonitoringCount - 1;
            testRetransmissionCount = 0;
            noTopologyChange = false;
            testRingReq(defaultTestInterval);
            if (!reactOnLinkChange) {
                topologyChangeReq(topologyChangeInterval);
            }
            else {
                topologyChangeReq(SIMTIME_ZERO);
            }
            nodeState = CHK_RC;
            EV_DETAIL << "Switching State from CHK_RO to CHK_RC" << EV_FIELD(nodeState) << EV_ENDL;
            ringState = CLOSED;
        } else if (role == MANAGER_AUTO
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
        } else if (role == MANAGER_AUTO
                && !isBetterThanOwnPrio(managerPrio, sourceAddress)) {
            testMgrNackReq(ringPort, managerPrio, sourceAddress);
        }
        break;
    case DE:
    case DE_IDLE:
    case PT:
    case PT_IDLE:
        if (role == MANAGER_AUTO && sourceAddress != localBridgeAddress
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

void Mrp::topologyChangeInd(MacAddress sourceAddress, simtime_t time)
{
    switch (nodeState) {
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
        setPortState(secondaryRingPortId, MrpInterfaceData::FORWARDING);
        clearFDB(time);
        nodeState = PT_IDLE;
        EV_DETAIL << "Switching State from PT to PT_IDLE" << EV_FIELD(nodeState) << EV_ENDL;
        break;
    case DE:
        linkChangeCount = linkMaxChange;
        cancelEvent(linkDownTimer);
        clearFDB(time);
        nodeState = DE_IDLE;
        EV_DETAIL << "Switching State from DE to DE_IDLE" << EV_FIELD(nodeState) << EV_ENDL;
        break;
    case DE_IDLE:
        clearFDB(time);
        if (role == MANAGER_AUTO
                && linkUpHysteresisTimer->isScheduled()) {
            setPortState(secondaryRingPortId, MrpInterfaceData::FORWARDING);
            nodeState = PT_IDLE;
            EV_DETAIL << "Switching State from DE_IDLE to PT_IDLE" << EV_FIELD(nodeState) << EV_ENDL;
        }
    case PT_IDLE:
        clearFDB(time);
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void Mrp::linkChangeInd(LinkState linkState)
{
    switch (nodeState) {
    case POWER_ON:
    case AC_STAT1:
    case DE_IDLE:
    case PT:
    case DE:
    case PT_IDLE:
        break;
    case PRM_UP:
        if (!addTest) {
            if (nonblockingMrcSupported) { //15
                addTest = true;
                testRingReq(shortTestInterval);
                break;
            } else if (linkState == LinkState::UP) {
                addTest = true;
                testRingReq(shortTestInterval);
                topologyChangeReq(SIMTIME_ZERO);
                break;
            }
        }
        else {
            if (!nonblockingMrcSupported && linkState == LinkState::UP) { //18
                topologyChangeReq(SIMTIME_ZERO);
            }
            break;
        }
        //all other cases : ignore
        break;
    case CHK_RO:
        if (!addTest) {
            if (linkState == LinkState::DOWN) {
                addTest = true;
                testRingReq(shortTestInterval);
                break;
            } else if (linkState == LinkState::UP) {
                if (nonblockingMrcSupported) {
                    addTest = true;
                    testRingReq(shortTestInterval);
                }
                else {
                    setPortState(secondaryRingPortId, MrpInterfaceData::BLOCKED);
                    testMaxRetransmissionCount = testMonitoringExtendedCount - 1;
                    testRetransmissionCount = 0;
                    addTest = true;
                    testRingReq(shortTestInterval);
                    topologyChangeReq(SIMTIME_ZERO);
                    nodeState = CHK_RC;
                    ringState = CLOSED;
                    EV_DETAIL << "Switching State from CHK_RO to CHK_RC" << EV_FIELD(nodeState) << EV_ENDL;
                }
            }
        }
        else {
            if (!nonblockingMrcSupported && linkState == LinkState::UP) {
                setPortState(secondaryRingPortId, MrpInterfaceData::BLOCKED);
                testMaxRetransmissionCount = testMonitoringExtendedCount - 1;
                testRetransmissionCount = 0;
                testRingReq(defaultTestInterval);
                topologyChangeReq(SIMTIME_ZERO);
                nodeState = CHK_RC;
                ringState = CLOSED;
                EV_DETAIL << "Switching State from CHK_RO to CHK_RC" << EV_FIELD(nodeState) << EV_ENDL;
            }
        }
        //all other cases: ignore
        break;
    case CHK_RC:
        if (reactOnLinkChange) {
            if (linkState == LinkState::DOWN) {
                setPortState(secondaryRingPortId, MrpInterfaceData::FORWARDING);
                topologyChangeReq(SIMTIME_ZERO);
                nodeState = CHK_RO;
                ringState = OPEN;
                EV_DETAIL << "Switching State from CHK_RC to CHK_RO" << EV_FIELD(nodeState) << EV_ENDL;
                break;
            } else if (linkState == LinkState::UP) {
                if (nonblockingMrcSupported) {
                    testMaxRetransmissionCount = testMonitoringCount - 1;
                }
                else {
                    testMaxRetransmissionCount = testMonitoringExtendedCount - 1;
                }
                topologyChangeReq(SIMTIME_ZERO);
            }
        } else if (nonblockingMrcSupported) {
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

void Mrp::testMgrNackInd(int ringPort, MacAddress sourceAddress, MrpPriority managerPrio, MacAddress bestMRMSourceAddress)
{
    switch (nodeState) {
    case POWER_ON:
    case AC_STAT1:
    case DE:
    case DE_IDLE:
    case PT:
    case PT_IDLE:
        break;
    case PRM_UP:
        if (role == MANAGER_AUTO && sourceAddress != localBridgeAddress
                && bestMRMSourceAddress == localBridgeAddress) {
            if (isBetterThanBestPrio(managerPrio, sourceAddress)) {
                hostBestMRMSourceAddress = sourceAddress;
                hostBestMRMPriority = managerPrio;
            }
            cancelEvent(topologyChangeTimer);
            mrcInit();
            testPropagateReq(ringPort, hostBestMRMPriority, hostBestMRMSourceAddress);
            nodeState = DE_IDLE;
            EV_DETAIL << "Switching State from PRM_UP to DE_IDLE" << EV_FIELD(nodeState) << EV_ENDL;
        }
        break;
    case CHK_RO:
        if (role == MANAGER_AUTO && sourceAddress != localBridgeAddress
                && bestMRMSourceAddress == localBridgeAddress) {
            if (isBetterThanBestPrio(managerPrio, sourceAddress)) {
                hostBestMRMSourceAddress = sourceAddress;
                hostBestMRMPriority = managerPrio;
            }
            cancelEvent(topologyChangeTimer);
            mrcInit();
            testPropagateReq(ringPort, hostBestMRMPriority, hostBestMRMSourceAddress);
            nodeState = PT_IDLE;
            EV_DETAIL << "Switching State from CHK_RO to PT_IDLE" << EV_FIELD(nodeState) << EV_ENDL;
        }
        break;
    case CHK_RC:
        if (role == MANAGER_AUTO && sourceAddress != localBridgeAddress
                && bestMRMSourceAddress == localBridgeAddress) {
            if (isBetterThanBestPrio(managerPrio, sourceAddress)) {
                hostBestMRMSourceAddress = sourceAddress;
                hostBestMRMPriority = managerPrio;
            }
            cancelEvent(topologyChangeTimer);
            mrcInit();
            testPropagateReq(ringPort, hostBestMRMPriority, hostBestMRMSourceAddress);
            setPortState(secondaryRingPortId, MrpInterfaceData::FORWARDING);
            nodeState = PT_IDLE;
            EV_DETAIL << "Switching State from CHK_RC to PT_IDLE" << EV_FIELD(nodeState) << EV_ENDL;
        }
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void Mrp::testPropagateInd(int ringPort, MacAddress sourceAddress, MrpPriority managerPrio, MacAddress bestMRMSourceAddress, MrpPriority bestMRMPrio)
{
    switch (nodeState) {
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
        if (role == MANAGER_AUTO && sourceAddress != localBridgeAddress
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

void Mrp::mauTypeChangeInd(int ringPort, LinkState linkState)
{
    switch (nodeState) {
    case POWER_ON:
        //all cases: ignore
        break;
    case AC_STAT1:
        //Client
        if (role == CLIENT) {
            if (linkState == LinkState::UP) {
                if (ringPort == primaryRingPortId) {
                    setPortState(primaryRingPortId, MrpInterfaceData::FORWARDING);
                    EV_DETAIL << "Switching State from AC_STAT1 to DE_IDLE" << EV_ENDL;
                    nodeState = DE_IDLE;
                    break;
                } else if (ringPort == secondaryRingPortId) {
                    toggleRingPorts();
                    setPortState(primaryRingPortId, MrpInterfaceData::FORWARDING);
                    EV_DETAIL << "Switching State from AC_STAT1 to DE_IDLE" << EV_ENDL;
                    nodeState = DE_IDLE;
                    break;
                }
            }
            //all other cases: ignore
            break;
        }
        //Manager
        if (role == MANAGER || role == MANAGER_AUTO) {
            if (linkState == LinkState::UP) {
                if (ringPort == primaryRingPortId) {
                    setPortState(primaryRingPortId, MrpInterfaceData::FORWARDING);
                    testRingReq(defaultTestInterval);
                    EV_DETAIL << "Switching State from AC_STAT1 to PRM_UP" << EV_ENDL;
                    nodeState = PRM_UP;
                    ringState = OPEN;
                    break;
                } else if (ringPort == secondaryRingPortId) {
                    toggleRingPorts();
                    setPortState(primaryRingPortId, MrpInterfaceData::FORWARDING);
                    testRingReq(defaultTestInterval);
                    EV_DETAIL << "Switching State from AC_STAT1 to PRM_UP" << EV_ENDL;
                    nodeState = PRM_UP;
                    ringState = OPEN;
                    break;
                }
            }
            //all other cases: ignore
            break;
        }
        //all other roles: ignore
        break;
    case PRM_UP:
        if (ringPort == primaryRingPortId
                && linkState == LinkState::DOWN) {
            cancelEvent(testTimer);
            setPortState(primaryRingPortId, MrpInterfaceData::BLOCKED);
            nodeState = AC_STAT1;
            ringState = OPEN;
            EV_DETAIL << "Switching State from PRM_UP to AC_STAT1" << EV_FIELD(nodeState) << EV_ENDL;
            break;
        }
        if (ringPort == secondaryRingPortId
                && linkState == LinkState::UP) {
            testMaxRetransmissionCount = testMonitoringCount - 1;
            testRetransmissionCount = 0;
            noTopologyChange = true;
            testRingReq(defaultTestInterval);
            nodeState = CHK_RC;
            ringState = CLOSED;
            EV_DETAIL << "Switching State from PRM_UP to CHK_RC" << EV_FIELD(nodeState) << EV_ENDL;
            break;
        }
        //all other Cases: ignore
        break;
    case CHK_RO:
        if (linkState == LinkState::DOWN) {
            if (ringPort == primaryRingPortId) {
                toggleRingPorts();
                setPortState(secondaryRingPortId, MrpInterfaceData::BLOCKED);
                testRingReq(defaultTestInterval);
                topologyChangeReq(topologyChangeInterval);
                nodeState = PRM_UP;
                ringState = OPEN;
                EV_DETAIL << "Switching State from CHK_RO to PRM_UP" << EV_FIELD(nodeState) << EV_ENDL;
                break;
            }
            if (ringPort == secondaryRingPortId) {
                setPortState(secondaryRingPortId, MrpInterfaceData::BLOCKED);
                nodeState = PRM_UP;
                ringState = OPEN;
                EV_DETAIL << "Switching State from CHK_RO to PRM_UP" << EV_FIELD(nodeState) << EV_ENDL;
                break;
            }
        }
        //all other cases: ignore
        break;
    case CHK_RC:
        if (linkState == LinkState::DOWN) {
            if (ringPort == primaryRingPortId) {
                toggleRingPorts();
                testRingReq(defaultTestInterval);
                topologyChangeReq(topologyChangeInterval);
                nodeState = PRM_UP;
                ringState = OPEN;
                EV_DETAIL << "Switching State from CHK_RC to PRM_UP" << EV_FIELD(nodeState) << EV_ENDL;
                break;
            }
            if (ringPort == secondaryRingPortId) {
                nodeState = PRM_UP;
                ringState = OPEN;
                EV_DETAIL << "Switching State from CHK_RC to PRM_UP" << EV_FIELD(nodeState) << EV_ENDL;
            }
        }
        //all other Cases: ignore
        break;
    case DE_IDLE:
        if (ringPort == secondaryRingPortId
                && linkState == LinkState::UP) {
            linkChangeReq(primaryRingPortId, LinkState::UP);
            nodeState = PT;
            EV_DETAIL << "Switching State from DE_IDLE to PT" << EV_FIELD(nodeState) << EV_ENDL;
        }
        if (ringPort == primaryRingPortId
                && linkState == LinkState::DOWN) {
            setPortState(primaryRingPortId, MrpInterfaceData::BLOCKED);
            nodeState = AC_STAT1;
            EV_DETAIL << "Switching State from DE_IDLE to AC_STAT1" << EV_FIELD(nodeState) << EV_ENDL;
        }
        //all other cases: ignore
        break;
    case PT:
        if (linkState == LinkState::DOWN) {
            if (ringPort == secondaryRingPortId) {
                cancelEvent(linkUpTimer);
                setPortState(secondaryRingPortId, MrpInterfaceData::BLOCKED);
                linkChangeReq(primaryRingPortId, LinkState::DOWN);
                nodeState = DE;
                EV_DETAIL << "Switching State from PT to DE" << EV_FIELD(nodeState) << EV_ENDL;
                break;
            }
            if (ringPort == primaryRingPortId) {
                cancelEvent(linkUpTimer);
                toggleRingPorts();
                setPortState(primaryRingPortId, MrpInterfaceData::FORWARDING);
                setPortState(secondaryRingPortId, MrpInterfaceData::BLOCKED);
                linkChangeReq(primaryRingPortId, LinkState::DOWN);
                nodeState = DE;
                EV_DETAIL << "Switching State from PT to DE" << EV_FIELD(nodeState) << EV_ENDL;
                break;
            }
        }
        //all other cases: ignore
        break;
    case DE:
        if (ringPort == secondaryRingPortId
                && linkState == LinkState::UP) {
            cancelEvent(linkDownTimer);
            linkChangeReq(primaryRingPortId, LinkState::UP);
            nodeState = PT;
            EV_DETAIL << "Switching State from DE to PT" << EV_FIELD(nodeState) << EV_ENDL;
            break;
        }
        if (ringPort == primaryRingPortId
                && linkState == LinkState::DOWN) {
            linkChangeCount = linkMaxChange;
            setPortState(primaryRingPortId, MrpInterfaceData::BLOCKED);
            nodeState = AC_STAT1;
            EV_DETAIL << "Switching State from DE to AC_STAT1" << EV_FIELD(nodeState) << EV_ENDL;
        }
        //all other cases: ignore
        break;
    case PT_IDLE:
        if (linkState == LinkState::DOWN) {
            if (ringPort == secondaryRingPortId) {
                setPortState(secondaryRingPortId, MrpInterfaceData::BLOCKED);
                linkChangeReq(primaryRingPortId, LinkState::DOWN);
                nodeState = DE;
                EV_DETAIL << "Switching State from PT_IDLE to DE" << EV_FIELD(nodeState) << EV_ENDL;
                break;
            }
            if (ringPort == primaryRingPortId) {
                primaryRingPortId = secondaryRingPortId;
                secondaryRingPortId = ringPort;
                setPortState(secondaryRingPortId, MrpInterfaceData::BLOCKED);
                linkChangeReq(primaryRingPortId, LinkState::DOWN);
                nodeState = DE;
                EV_DETAIL << "Switching State from PT_IDLE to DE" << EV_FIELD(nodeState) << EV_ENDL;
                break;
            }
        }
        //all other cases: ignore
        break;
    default:
        throw cRuntimeError("Unknown Node-State");
    }
}

void Mrp::interconnTopologyChangeInd(MacAddress sourceAddress, simtime_t time, uint16_t inID, int ringPort, Packet *packet)
{
    switch (nodeState) {
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
        if (ringPort == primaryRingPortId) {
            interconnForwardReq(secondaryRingPortId, packet);
        } else if (ringPort == secondaryRingPortId) {
            interconnForwardReq(primaryRingPortId, packet);
        }
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void Mrp::interconnLinkChangeInd(uint16_t inID, LinkState linkstate, int ringPort, Packet *packet)
{
    switch (nodeState) {
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
        if (ringPort == primaryRingPortId) {
            interconnForwardReq(secondaryRingPortId, packet);
        } else if (ringPort == secondaryRingPortId) {
            interconnForwardReq(primaryRingPortId, packet);
        }
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void Mrp::interconnLinkStatusPollInd(uint16_t inID, int ringPort, Packet *packet)
{
    switch (nodeState) {
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
        if (ringPort == primaryRingPortId) {
            interconnForwardReq(secondaryRingPortId, packet);
        } else if (ringPort == secondaryRingPortId) {
            interconnForwardReq(primaryRingPortId, packet);
        }
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void Mrp::interconnTestInd(MacAddress sourceAddress, int ringPort, uint16_t inID, Packet *packet)
{
    switch (nodeState) {
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
        if (ringPort == primaryRingPortId) {
            interconnForwardReq(secondaryRingPortId, packet);
        } else if (ringPort == secondaryRingPortId) {
            interconnForwardReq(primaryRingPortId, packet);
        }
        break;
    default:
        throw cRuntimeError("Unknown NodeState");
    }
}

void Mrp::interconnForwardReq(int ringPort, Packet *packet)
{
    auto macAddressInd = packet->findTag<MacAddressInd>();
    auto sourceAddress = macAddressInd->getSrcAddress();
    auto destinationAddress = macAddressInd->getDestAddress();
    packet->trim();
    packet->clearTags();
    sendFrameReq(ringPort, destinationAddress, sourceAddress, priority, MRP_LT, packet);
}

void Mrp::sendFrameReq(int portId, const MacAddress &destinationAddress, const MacAddress &sourceAddress, int prio, uint16_t lt, Packet *mrpPDU)
{
    mrpPDU->addTag<InterfaceReq>()->setInterfaceId(portId);
    mrpPDU->addTag<PacketProtocolTag>()->setProtocol(&Protocol::mrp);
    mrpPDU->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ieee8022llc);
    auto macAddressReq = mrpPDU->addTag<MacAddressReq>();
    macAddressReq->setSrcAddress(sourceAddress);
    macAddressReq->setDestAddress(destinationAddress);
    EV_INFO << "Sending packet down" << EV_FIELD(mrpPDU) << EV_FIELD(destinationAddress) << EV_ENDL;
    send(mrpPDU, "relayOut");
}

void Mrp::sendCCM(int portId, Packet *ccm)
{
    ccm->addTag<InterfaceReq>()->setInterfaceId(portId);
    ccm->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ieee8021qCFM);
    ccm->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ieee8022llc);
    auto macAddressReq = ccm->addTag<MacAddressReq>();
    macAddressReq->setSrcAddress(getPortNetworkInterface(portId)->getMacAddress());
    macAddressReq->setDestAddress(MacAddress::CFM_CCM_MULTICAST_ADDRESS);
    EV_INFO << "Sending packet down" << EV_FIELD(ccm) << EV_ENDL;
    send(ccm, "relayOut");
}

void Mrp::handleStartOperation(LifecycleOperation *operation)
{
    //start();
}

void Mrp::handleStopOperation(LifecycleOperation *operation)
{
    stop();
}

void Mrp::handleCrashOperation(LifecycleOperation *operation)
{
    stop();
}

void Mrp::colorLink(NetworkInterface *ie, bool forwarding) const
{
    if (visualize) {
        cGate *inGate = switchModule->gate(ie->getNodeInputGateId());
        cGate *outGate = switchModule->gate(ie->getNodeOutputGateId());
        cGate *outGateNext = outGate ? outGate->getNextGate() : nullptr;
        cGate *outGatePrev = outGate ? outGate->getPreviousGate() : nullptr;
        cGate *inGatePrev = inGate ? inGate->getPreviousGate() : nullptr;
        cGate *inGatePrev2 = inGatePrev ? inGatePrev->getPreviousGate() : nullptr;

        if (outGate && inGate && inGatePrev && outGateNext && outGatePrev && inGatePrev2) {
            if (forwarding) {
                outGatePrev->getDisplayString().setTagArg("ls", 0, ENABLED_LINK_COLOR);
                inGate->getDisplayString().setTagArg("ls", 0, ENABLED_LINK_COLOR);
            }
            else {
                outGatePrev->getDisplayString().setTagArg("ls", 0, DISABLED_LINK_COLOR);
                inGate->getDisplayString().setTagArg("ls", 0, DISABLED_LINK_COLOR);
            }

            if ((!inGatePrev2->getDisplayString().containsTag("ls")
                    || strcmp(inGatePrev2->getDisplayString().getTagArg("ls", 0), ENABLED_LINK_COLOR) == 0)
                    && forwarding) {
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

void Mrp::refreshDisplay() const
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

void Mrp::updateDisplayString() const
{
    if (getEnvir()->isGUI()) {
        auto text = StringFormat::formatString(par("displayStringTextFormat"), this);
        getDisplayString().setTagArg("t", 0, text.c_str());
    }
}

std::string Mrp::resolveDirective(char directive) const
{
    switch (directive) {
        case 'r': return getMrpRoleName(role, true);
        case 'n': return getNodeStateName(nodeState);
        case 'g': return getRingStateName(ringState);
        default: throw cRuntimeError("Unknown directive: %c", directive);
    }
}

const char *Mrp::getMrpRoleName(MrpRole role, bool acronym)
{
    switch (role) {
        case DISABLED: return "DISABLED";
        case CLIENT: return acronym ? "MRC" : "CLIENT";
        case MANAGER: return acronym ? "MRM" : "MANAGER";
        case MANAGER_AUTO_COMP: return acronym ? "MRA-C" : "MANAGER_AUTO_COMP";
        case MANAGER_AUTO: return acronym ? "MRA" : "MANAGER_AUTO";
        default: return "???";
    }
}

const char *Mrp::getNodeStateName(NodeState state)
{
    switch (state) {
        case POWER_ON: return "POWER_ON";
        case AC_STAT1: return "AC_STAT1";
        case PRM_UP: return "PRM_UP";
        case CHK_RO: return "CHK_RO";
        case CHK_RC: return "CHK_RC";
        case DE_IDLE: return "DE_IDLE";
        case PT: return "PT";
        case DE: return "DE";
        case PT_IDLE: return "PT_IDLE";
        default: return "???";
    }
}

const char *Mrp::getRingStateName(RingState state)
{
    switch (state) {
        case OPEN: return "OPEN";
        case CLOSED: return "CLOSED";
        case UNDEFINED: return "UNDEFINED";
        default: return "???";
    }
}

} // namespace inet

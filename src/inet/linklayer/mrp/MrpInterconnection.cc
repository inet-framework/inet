//
// Copyright (C) 2024 Daniel Zeitler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/linklayer/mrp/MrpPdu_m.h"
#include "inet/linklayer/mrp/Timers_m.h"
#include "MrpInterconnection.h"
#include "Mrp.h"

namespace inet {

Register_Enum(MrpInterconnection::InterconnectionRole, (MrpInterconnection::INTERCONNECTION_CLIENT, MrpInterconnection::INTERCONNECTION_MANAGER));
Register_Enum(MrpInterconnection::InterconnectionNodeState, (MrpInterconnection::POWER_ON, MrpInterconnection::AC_STAT1, MrpInterconnection::CHK_IO, MrpInterconnection::CHK_IC, MrpInterconnection::PT, MrpInterconnection::IP_IDLE));
Register_Enum(MrpInterconnection::InterconnectionTopologyState, (MrpInterconnection::OPEN, MrpInterconnection::CLOSED));

Define_Module(MrpInterconnection);

MrpInterconnection::MrpInterconnection()
{
}

MrpInterconnection::~MrpInterconnection()
{
    cancelAndDelete(inLinkStatusPollTimer);
    cancelAndDelete(inLinkTestTimer);
    cancelAndDelete(inTopologyChangeTimer);
    cancelAndDelete(inLinkDownTimer);
    cancelAndDelete(inLinkUpTimer);
}

void MrpInterconnection::initialize(int stage)
{
    Mrp::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inRole = parseInterconnectionRole(par("interconnectionRole"));
        interconnectionID = par("interconnectionID");
        std::string interconnectionCheckMode = par("interconnectionCheckMode");
        inLinkCheckEnabled = interconnectionCheckMode.find("LC") != std::string::npos;
        inRingCheckEnabled = interconnectionCheckMode.find("RC") != std::string::npos;

        simsignal_t inRoleChangedSignal = registerSignal("inRoleChanged");
        simsignal_t inNodeStateChangedSignal = registerSignal("inNodeStateChanged");
        simsignal_t inTopologyStateChangedSignal = registerSignal("inTopologyStateChanged");
        inRole.addEmitCallback(this, inRoleChangedSignal);
        inNodeState.addEmitCallback(this, inNodeStateChangedSignal);
        inTopologyState.addEmitCallback(this, inTopologyStateChangedSignal);

        inPortStateChangedSignal = registerSignal("inPortStateChanged");
        inTopologyChangeAnnouncedSignal = registerSignal("inTopologyChangeAnnounced");
        inStatusPollSentSignal = registerSignal("inStatusPollSent");
        inLinkChangeDetectedSignal = registerSignal("inLinkChangeDetected");
        inTestFrameLatencySignal = registerSignal("inTestFrameLatency");
    }
    if (stage == INITSTAGE_LINK_LAYER) {
        EV_DETAIL << "Initialize Interconnection Stage link layer" << EV_ENDL;

        interconnectionPortId = resolveInterfaceIndex(par("interconnectionPort"));
        initRingPort(interconnectionPortId, MrpInterfaceData::INTERCONNECTION, inLinkCheckEnabled);
    }
}

MrpInterconnection::InterconnectionRole MrpInterconnection::parseInterconnectionRole(const char *role) const
{
    if (!strcmp(role, "MIC"))
        return INTERCONNECTION_CLIENT;
    else if (!strcmp(role, "MIM"))
        return INTERCONNECTION_MANAGER;
    else
        throw cRuntimeError("Unknown MRP Interconnection role '%s'", role);
}

void MrpInterconnection::start()
{
    Mrp::start();
    setTimingProfile(timingProfile);
    intopologyChangeRepeatCount = inTopologyChangeMaxRepeatCount - 1;
    inLinkStatusPollCount = inLinkStatusPollMaxCount - 1;
    inLinkStatusPollTimer = new cMessage("inLinkStatusPollTimer");
    inTopologyChangeTimer = new cMessage("inTopologyChangeTimer");
    inLinkUpTimer = new cMessage("inLinkUpTimer");
    inLinkDownTimer = new cMessage("inLinkDownTimer");
    if (inRole == INTERCONNECTION_MANAGER)
        mimInit();
    else if (inRole == INTERCONNECTION_CLIENT)
        micInit();
    else
        throw cRuntimeError("Unknown Interconnection Role");
}

void MrpInterconnection::stop()
{
    cancelAndDelete(inLinkStatusPollTimer);
    cancelAndDelete(inLinkTestTimer);
    cancelAndDelete(inTopologyChangeTimer);
    cancelAndDelete(inLinkDownTimer);
    cancelAndDelete(inLinkUpTimer);
    Mrp::stop();
}

void MrpInterconnection::mimInit()
{
    mrpMacForwardingTable->addMrpForwardingInterface(primaryRingPortId, MacAddress(MC_INCONTROL), vlanID);
    mrpMacForwardingTable->addMrpForwardingInterface(secondaryRingPortId, MacAddress(MC_INCONTROL), vlanID);
    //MIM may not forward mrp frames received on Interconnection Port to Ring, adding Filter
    mrpMacForwardingTable->addMrpIngressFilterInterface(interconnectionPortId, MacAddress(MC_INCONTROL), vlanID);
    mrpMacForwardingTable->addMrpIngressFilterInterface(interconnectionPortId, MacAddress(MC_INTEST), vlanID);

    relay->registerAddress(MacAddress(MC_INCONTROL));
    if (inLinkCheckEnabled) {
        relay->registerAddress(MacAddress(MC_INTRANSFER));
        handleInLinkStatusPollTimer();
        if (!enableLinkCheckOnRing)
            startContinuityCheck();
    }
    if (inRingCheckEnabled) {
        relay->registerAddress(MacAddress(MC_INTEST));
        inLinkTestTimer = new cMessage("inLinkTestTimer");
    }
    inNodeState = AC_STAT1;
    EV_DETAIL << "Interconnection Manager is started, Switching InState from POWER_ON to AC_STAT1"
                     << EV_FIELD(inNodeState) << EV_ENDL;
    mauTypeChangeInd(interconnectionPortId, getPortNetworkInterface(interconnectionPortId)->getState());
}

void MrpInterconnection::micInit()
{
    if (inRingCheckEnabled) {
        mrpMacForwardingTable->addMrpForwardingInterface(primaryRingPortId, MacAddress(MC_INTEST), vlanID);
        mrpMacForwardingTable->addMrpForwardingInterface(secondaryRingPortId, MacAddress(MC_INTEST), vlanID);
        mrpMacForwardingTable->addMrpForwardingInterface(interconnectionPortId, MacAddress(MC_INTEST), vlanID);
        /*
         *Ingress filter can now be set by MRP, therefore not need for forwarding on mrp level
         //frames received on Interconnection port are not forwarded on relay level (ingress) filter
         // and have to be forwarded by MIC
         relay->registerAddress(MacAddress(MC_INTEST));
         */
        mrpMacForwardingTable->addMrpForwardingInterface(primaryRingPortId, MacAddress(MC_INCONTROL), vlanID);
        mrpMacForwardingTable->addMrpForwardingInterface(secondaryRingPortId, MacAddress(MC_INCONTROL), vlanID);
    }
    if (inLinkCheckEnabled) {
        relay->registerAddress(MacAddress(MC_INTRANSFER));
        if (!enableLinkCheckOnRing)
            startContinuityCheck();
    }
    relay->registerAddress(MacAddress(MC_INCONTROL));
    inNodeState = AC_STAT1;
    EV_DETAIL << "Interconnection Client is started, Switching InState from POWER_ON to AC_STAT1" << EV_FIELD(inNodeState) << EV_ENDL;
    mauTypeChangeInd(interconnectionPortId, getPortNetworkInterface(interconnectionPortId)->getState());
}

void MrpInterconnection::setTimingProfile(int maxRecoveryTime)
{
    Mrp::setTimingProfile(maxRecoveryTime);
    switch (maxRecoveryTime) {
    case 500:
        inLinkChangeInterval = SimTime(20, SIMTIME_MS);
        inTopologyChangeInterval = SimTime(20, SIMTIME_MS);
        inLinkStatusPollInterval = SimTime(20, SIMTIME_MS);
        inTestDefaultInterval = SimTime(50, SIMTIME_MS);
        break;
    case 200:
        inLinkChangeInterval = SimTime(20, SIMTIME_MS);
        inTopologyChangeInterval = SimTime(10, SIMTIME_MS);
        inLinkStatusPollInterval = SimTime(20, SIMTIME_MS);
        inTestDefaultInterval = SimTime(20, SIMTIME_MS);
        break;
    case 30:
        inLinkChangeInterval = SimTime(3, SIMTIME_MS);
        inTopologyChangeInterval = SimTime(1, SIMTIME_MS);
        inLinkStatusPollInterval = SimTime(3, SIMTIME_MS);
        inTestDefaultInterval = SimTime(3, SIMTIME_MS);
        break;
    case 10:
        inLinkChangeInterval = SimTime(3, SIMTIME_MS);
        inTopologyChangeInterval = SimTime(1, SIMTIME_MS);
        inLinkStatusPollInterval = SimTime(3, SIMTIME_MS);
        inTestDefaultInterval = SimTime(3, SIMTIME_MS);
        break;
    default:
        throw cRuntimeError("Only RecoveryTimes 500, 200, 30 and 10 ms are defined!");
    }
}

void MrpInterconnection::handleStartOperation(LifecycleOperation *operation)
{
    //start();
}

void MrpInterconnection::handleStopOperation(LifecycleOperation *operation)
{
    stop();
}

void MrpInterconnection::handleCrashOperation(LifecycleOperation *operation)
{
    stop();
}

void MrpInterconnection::handleMessageWhenUp(cMessage *msg)
{
    if (!msg->isSelfMessage()) {
        msg->setKind(2);
        EV_INFO << "Received Message on InterConnectionNode, Rescheduling:"
                       << EV_FIELD(msg) << EV_ENDL;
        simtime_t processingDelay = SimTime((int64_t)(processingDelayPar->doubleValue() * 1e6), SIMTIME_US);
        scheduleAfter(processingDelay, msg);
    }
    else {
        EV_INFO << "Received Self-Message:" << EV_FIELD(msg) << EV_ENDL;
        EV_DEBUG << "State:" << EV_FIELD(inNodeState) << EV_ENDL;
        if (msg == inLinkStatusPollTimer)
            handleInLinkStatusPollTimer();
        else if (msg == inLinkTestTimer)
            handleInTestTimer();
        else if (msg == inTopologyChangeTimer)
            handleInTopologyChangeTimer();
        else if (msg == inLinkUpTimer)
            handleInLinkUpTimer();
        else if (msg == inLinkDownTimer)
            handleInLinkDownTimer();
        else if (msg == testTimer)
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
            ProcessDelayTimer *timer = dynamic_cast<ProcessDelayTimer*>(msg);
            if (timer != nullptr) {
                handleDelayTimer(timer->getPort(), timer->getField());
                delete timer;
            }
        } else if (msg->getKind() == 1) {
            ContinuityCheckTimer *timer = dynamic_cast<ContinuityCheckTimer*>(msg);
            if (timer != nullptr) {
                handleContinuityCheckTimer(timer->getPort());
                delete timer;
            }
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

void MrpInterconnection::handleInTestTimer()
{
    switch (inNodeState) {
    case CHK_IO:
        interconnTestReq(inTestDefaultInterval);
        break;
    case CHK_IC:
        interconnTestReq(inTestDefaultInterval);
        if (inTestRetransmissionCount >= inTestMaxRetransmissionCount) {
            setPortState(interconnectionPortId, MrpInterfaceData::FORWARDING);
            inTestMaxRetransmissionCount = inTestMonitoringCount - 1;
            inTestRetransmissionCount = 0;
            interconnTopologyChangeReq(inTopologyChangeInterval);
            inNodeState = CHK_IO;
        }
        else {
            inTestRetransmissionCount = inTestRetransmissionCount + 1;
        }
        break;
    case PT:
    case IP_IDLE:
    case AC_STAT1:
    case POWER_ON:
        break;
    default:
        throw cRuntimeError("Unknown Node State");
    }
}

void MrpInterconnection::handleInLinkStatusPollTimer()
{
    if (inLinkStatusPollCount > 0) {
        inLinkStatusPollCount--;
        scheduleAfter(inLinkStatusPollInterval, inLinkStatusPollTimer);
    } else if (inLinkStatusPollCount == 0) {
        inLinkStatusPollCount = inLinkStatusPollMaxCount - 1;
    }
    setupInterconnLinkStatusPollReq();
}

void MrpInterconnection::handleInTopologyChangeTimer()
{
    if (intopologyChangeRepeatCount > 0) {
        setupInterconnTopologyChangeReq(intopologyChangeRepeatCount * inTopologyChangeInterval);
        intopologyChangeRepeatCount--;
        scheduleAfter(inTopologyChangeInterval, inTopologyChangeTimer);
    } else if (intopologyChangeRepeatCount == 0) {
        intopologyChangeRepeatCount = inTopologyChangeMaxRepeatCount - 1;
        clearFDB(0);
        setupInterconnTopologyChangeReq(0);
    }
}

void MrpInterconnection::handleInLinkUpTimer()
{
    inLinkChangeCount--;
    switch (inNodeState) {
    case PT:
        if (inLinkChangeCount == 0) {
            inLinkChangeCount = inLinkMaxChange;
            setPortState(interconnectionPortId, MrpInterfaceData::FORWARDING);
            inNodeState = IP_IDLE;
        }
        else {
            scheduleAfter(inLinkChangeInterval, inLinkUpTimer);
            interconnLinkChangeReq(LinkState::UP, inLinkChangeCount * inLinkChangeInterval);
        }
        break;
    case IP_IDLE:
    case AC_STAT1:
    case POWER_ON:
    case CHK_IO:
    case CHK_IC:
        break;
    default:
        throw cRuntimeError("Unknown Node State");
    }
}

void MrpInterconnection::handleInLinkDownTimer()
{
    inLinkChangeCount--;
    switch (inNodeState) {
    case AC_STAT1:
        if (inRole == INTERCONNECTION_CLIENT) {
            if (inLinkChangeCount == 0) {
                inLinkChangeCount = inLinkMaxChange;
            }
            else {
                scheduleAfter(inLinkChangeInterval, inLinkDownTimer);
                interconnLinkChangeReq(LinkState::DOWN, inLinkChangeCount * inLinkChangeInterval);
            }
        }
        break;
    case POWER_ON:
    case CHK_IO:
    case CHK_IC:
    case PT:
    case IP_IDLE:
        break;
    default:
        throw cRuntimeError("Unknown Node State");
    }
}

void MrpInterconnection::mauTypeChangeInd(int ringPort, LinkState linkState)
{
    if (ringPort == interconnectionPortId) {
        switch (inNodeState) {
        case AC_STAT1:
            if (linkState == LinkState::UP) {
                if (inRole == INTERCONNECTION_MANAGER) {
                    setPortState(ringPort, MrpInterfaceData::BLOCKED);
                    if (inLinkCheckEnabled) {
                        interconnLinkStatusPollReq(inLinkStatusPollInterval);
                    }
                    if (inRingCheckEnabled) {
                        inTestMaxRetransmissionCount = inTestMonitoringCount - 1;
                        inTestRetransmissionCount = 0;
                        interconnTestReq(inTestDefaultInterval);
                    }
                    inNodeState = CHK_IC;
                    EV_DETAIL << "Switching InState from AC_STAT1 to CHK_IC"
                                     << EV_FIELD(inNodeState) << EV_ENDL;
                    inTopologyState = CLOSED;
                } else if (inRole == INTERCONNECTION_CLIENT) {
                    inLinkChangeCount = inLinkMaxChange;
                    cancelEvent(inLinkDownTimer);
                    scheduleAfter(inLinkChangeInterval, inLinkUpTimer);
                    interconnLinkChangeReq(LinkState::UP, inLinkChangeCount * inLinkChangeInterval);
                    inNodeState = PT;
                    EV_DETAIL << "Switching InState from AC_STAT1 to PT"
                                     << EV_FIELD(inNodeState) << EV_ENDL;
                }
            }
            break;
        case CHK_IO:
            if (linkState == LinkState::DOWN) {
                setPortState(ringPort, MrpInterfaceData::BLOCKED);
                if (inLinkCheckEnabled) {
                    interconnLinkStatusPollReq(inLinkStatusPollInterval);
                }
                if (inRingCheckEnabled) {
                    interconnTopologyChangeReq(inTopologyChangeInterval);
                    interconnTestReq(inTestDefaultInterval);
                }
                inNodeState = AC_STAT1;
                EV_DETAIL << "Switching InState from CHK_IO to AC_STAT1"
                                 << EV_FIELD(inNodeState) << EV_ENDL;
            }
            break;
        case CHK_IC:
            if (linkState == LinkState::DOWN) {
                setPortState(ringPort, MrpInterfaceData::BLOCKED);
                if (inRingCheckEnabled) {
                    interconnTopologyChangeReq(inTopologyChangeInterval);
                    interconnTestReq(inTestDefaultInterval);
                }
                inNodeState = AC_STAT1;
                EV_DETAIL << "Switching InState from CHK_IC to AC_STAT1"
                                 << EV_FIELD(inNodeState) << EV_ENDL;
            }
            break;
        case PT:
            if (linkState == LinkState::DOWN) {
                inLinkChangeCount = inLinkMaxChange;
                cancelEvent(inLinkUpTimer);
                setPortState(ringPort, MrpInterfaceData::BLOCKED);
                scheduleAfter(inLinkChangeInterval, inLinkDownTimer);
                interconnLinkChangeReq(LinkState::DOWN, inLinkChangeCount * inLinkChangeInterval);
                inNodeState = AC_STAT1;
                EV_DETAIL << "Switching InState from PT to AC_STAT1"
                                 << EV_FIELD(inNodeState) << EV_ENDL;
            }
            break;
        case IP_IDLE:
            if (linkState == LinkState::DOWN) {
                inLinkChangeCount = inLinkMaxChange;
                setPortState(ringPort, MrpInterfaceData::BLOCKED);
                scheduleAfter(inLinkChangeInterval, inLinkDownTimer);
                interconnLinkChangeReq(LinkState::DOWN, inLinkChangeCount * inLinkChangeInterval);
                inNodeState = AC_STAT1;
                EV_DETAIL << "Switching InState from IP_IDLE to AC_STAT1"
                                 << EV_FIELD(inNodeState) << EV_ENDL;
            }
            break;
        case POWER_ON:
            break;
        default:
            throw cRuntimeError("Unknown Node State");
        }
    }
    else {
        Mrp::mauTypeChangeInd(ringPort, linkState);
    }
}

void MrpInterconnection::interconnTopologyChangeInd(MacAddress sourceAddress, simtime_t time, uint16_t inID, int ringPort, Packet *packet)
{
    if (inID == interconnectionID) {
        auto offset = B(2);
        const auto &firstTLV = packet->peekDataAt<MrpInTopologyChange>(offset);
        offset = offset + firstTLV->getChunkLength();
        const auto &secondTLV = packet->peekDataAt<MrpCommon>(offset);
        uint16_t sequence = secondTLV->getSequenceID();
        //Poll is reaching Node from Primary, Secondary and Interconnection Port
        //it is not necessary to react after the first frame
        if (sequence > lastInTopologyId) {
            lastInTopologyId = sequence;
            switch (inNodeState) {
            case AC_STAT1:
                if (inRole == INTERCONNECTION_CLIENT) {
                    cancelEvent(inLinkDownTimer);
                } else if (inRole == INTERCONNECTION_MANAGER
                        && sourceAddress == localBridgeAddress) {
                    clearFDB(time);
                }
                delete packet;
                break;
            case CHK_IO:
            case CHK_IC:
                if (sourceAddress == localBridgeAddress) {
                    clearFDB(time);
                }
                delete packet;
                break;
            case PT:
                inLinkChangeCount = inLinkMaxChange;
                cancelEvent(inLinkUpTimer);
                setPortState(interconnectionPortId, MrpInterfaceData::FORWARDING);
                inNodeState = IP_IDLE;
                EV_DETAIL << "Switching InState from PT to IP_IDLE"
                                 << EV_FIELD(inNodeState) << EV_ENDL;
                if (ringPort != interconnectionPortId) {
                    if (inLinkCheckEnabled)
                        inTransferReq(INTOPOLOGYCHANGE, interconnectionPortId, MC_INTRANSFER, packet);
                    else
                        inTransferReq(INTOPOLOGYCHANGE, interconnectionPortId, MC_INCONTROL, packet);
                } else
                    mrpForwardReq(INTOPOLOGYCHANGE, ringPort, MC_INCONTROL, packet);
                break;
            case IP_IDLE:
                if (ringPort != interconnectionPortId) {
                    if (inLinkCheckEnabled)
                        inTransferReq(INTOPOLOGYCHANGE, interconnectionPortId, MC_INTRANSFER, packet);
                    else
                        inTransferReq(INTOPOLOGYCHANGE, interconnectionPortId, MC_INCONTROL, packet);
                } else
                    mrpForwardReq(INTOPOLOGYCHANGE, ringPort, MC_INCONTROL, packet);
                break;
            case POWER_ON:
                delete packet;
                break;
            default:
                throw cRuntimeError("Unknown Node State");
            }
        }
        else {
            EV_DETAIL << "Received same Frame already" << EV_ENDL;
            delete packet;
        }
    }
    else {
        EV_INFO << "Received Frame from other InterConnectionID"
                       << EV_FIELD(inID) << EV_ENDL;
        delete packet;
    }
}

void MrpInterconnection::interconnLinkChangeInd(uint16_t InID, LinkState linkState, int ringPort, Packet *packet)
{
    if (InID == interconnectionID) {
        auto firstTLV = packet->peekDataAt<MrpInLinkChange>(B(2));
        switch (inNodeState) {
        case CHK_IO:
            if (inLinkCheckEnabled) {
                if (linkState == LinkState::UP) {
                    setPortState(interconnectionPortId, MrpInterfaceData::BLOCKED);
                    cancelEvent(inLinkStatusPollTimer);
                    interconnTopologyChangeReq(inTopologyChangeInterval);
                    EV_INFO << "Interconnection Ring closed" << EV_ENDL;
                    inNodeState = CHK_IC;
                    EV_DETAIL << "Switching InState from CHK_IO to CHK_IC"
                                     << EV_FIELD(inNodeState) << EV_ENDL;
                    inTopologyState = CLOSED;
                } else if (linkState == LinkState::DOWN) {
                    setPortState(interconnectionPortId, MrpInterfaceData::FORWARDING);
                }
            }
            if (inRingCheckEnabled) {
                if (linkState == LinkState::UP) {
                    interconnTestReq(inTestDefaultInterval);
                }
            }
            delete packet;
            break;
        case CHK_IC:
            if (linkState == LinkState::DOWN) {
                setPortState(interconnectionPortId, MrpInterfaceData::FORWARDING);
                interconnTopologyChangeReq(inTopologyChangeInterval);
                if (inLinkCheckEnabled) {
                    cancelEvent(inLinkStatusPollTimer);
                    EV_INFO << "Interconnection Ring open" << EV_ENDL;
                    inNodeState = CHK_IO;
                    EV_DETAIL << "Switching InState from CHK_IC to CHK_IO"
                                     << EV_FIELD(inNodeState) << EV_ENDL;
                    inTopologyState = OPEN;
                }
            }
            if (inRingCheckEnabled && linkState == LinkState::UP) {
                inTestMaxRetransmissionCount = inTestMonitoringCount - 1;
                interconnTopologyChangeReq(inTopologyChangeInterval);
            }
            delete packet;
            break;
        case PT:
        case IP_IDLE:
            if (ringPort == interconnectionPortId && inLinkCheckEnabled) {
                if (linkState == LinkState::UP) {
                    mrpForwardReq(INLINKUP, ringPort, MC_INCONTROL, packet);
                } else if (linkState == LinkState::DOWN) {
                    mrpForwardReq(INLINKDOWN, ringPort, MC_INCONTROL, packet);
                } else
                    delete packet;
            } else if (ringPort != interconnectionPortId) {
                if (inRingCheckEnabled) {
                    interconnForwardReq(interconnectionPortId, packet);
                } else if (linkState == LinkState::UP) {
                    inTransferReq(INLINKUP, interconnectionPortId, MC_INTRANSFER, packet);
                } else if (linkState == LinkState::DOWN) {
                    inTransferReq(INLINKDOWN, interconnectionPortId, MC_INTRANSFER, packet);
                } else
                    delete packet;
            } else
                delete packet;
            break;
        case AC_STAT1:
        case POWER_ON:
            delete packet;
            break;
        default:
            throw cRuntimeError("Unknown Node State");
        }
    }
    else {
        EV_INFO << "Received Frame from other InterConnectionID"
                       << EV_FIELD(ringPort) << EV_FIELD(InID) << EV_ENDL;
        delete packet;
    }
}

void MrpInterconnection::interconnLinkStatusPollInd(uint16_t inID, int ringPort, Packet *packet)
{
    if (inID == interconnectionID) {
        b offset = B(2);
        auto firstTLV = packet->peekDataAt<MrpInLinkStatusPoll>(offset);
        offset = offset + firstTLV->getChunkLength();
        auto secondTLV = packet->peekDataAt<MrpCommon>(offset);
        uint16_t sequence = secondTLV->getSequenceID();
        //Poll is reaching Node from Primary, Secondary and Interconnection Port
        //it is not necessary to react after the first frame
        if (sequence > lastPollId) {
            lastPollId = sequence;
            switch (inNodeState) {
            case AC_STAT1:
                if (inRole == INTERCONNECTION_CLIENT) {
                    interconnLinkChangeReq(LinkState::DOWN, 0);
                }
                delete packet;
                break;
            case PT:
            case IP_IDLE:
                interconnLinkChangeReq(LinkState::UP, 0);
                if (ringPort != interconnectionPortId) {
                    if (inLinkCheckEnabled)
                        inTransferReq(INLINKSTATUSPOLL, interconnectionPortId, MC_INTRANSFER, packet);
                    else
                        inTransferReq(INLINKSTATUSPOLL, interconnectionPortId, MC_INCONTROL, packet);
                } else
                    mrpForwardReq(INLINKSTATUSPOLL, ringPort, MC_INCONTROL, packet);
                break;
            case POWER_ON:
            case CHK_IO:
            case CHK_IC:
                delete packet;
                break;
            default:
                throw cRuntimeError("Unknown Node State");
            }
        }
        else {
            EV_DETAIL << "Received same Frame already" << EV_ENDL;
            delete packet;
        }
    }
    else {
        EV_INFO << "Received Frame from other InterConnectionID"
                       << EV_FIELD(ringPort) << EV_FIELD(inID) << EV_ENDL;
        delete packet;
    }
}

void MrpInterconnection::interconnTestInd(MacAddress sourceAddress, int ringPort, uint16_t inID, Packet *packet)
{
    if (inID == interconnectionID) {
        b offset = B(2);
        auto firstTLV = packet->peekDataAt<MrpInTest>(offset);
        offset = offset + firstTLV->getChunkLength();
        auto secondTLV = packet->peekDataAt<MrpCommon>(offset);
        uint16_t sequence = secondTLV->getSequenceID();
        simtime_t ringTime = simTime() - SimTime(firstTLV->getTimeStamp(), SIMTIME_MS);
        auto it = inTestFrameSent.find(sequence);
        if (it != inTestFrameSent.end()) {
            simtime_t ringTimePrecise = simTime() - it->second;
            emit(inTestFrameLatencySignal, ringTimePrecise);
            EV_DETAIL << "InterconnectionRingTime" << EV_FIELD(ringTime)
                             << EV_FIELD(ringTimePrecise) << EV_ENDL;
        }
        else {
            emit(inTestFrameLatencySignal, ringTime);
            EV_DETAIL << "InterconnectionRingTime" << EV_FIELD(ringTime) << EV_ENDL;
        }
        switch (inNodeState) {
        case AC_STAT1:
            if (inRole == INTERCONNECTION_MANAGER
                    && sourceAddress == localBridgeAddress) {
                setPortState(interconnectionPortId, MrpInterfaceData::BLOCKED);
                inTestMaxRetransmissionCount = inTestMonitoringCount - 1;
                inTestRetransmissionCount = 0;
                interconnTestReq(inTestDefaultInterval);
                inNodeState = CHK_IC;
                EV_DETAIL << "Switching InState from AC_STAT1 to CHK_IC"
                                 << EV_FIELD(inNodeState) << EV_ENDL;
            }
            delete packet;
            break;
        case CHK_IO:
            if (sourceAddress == localBridgeAddress) {
                setPortState(interconnectionPortId, MrpInterfaceData::BLOCKED);
                interconnTopologyChangeReq(inTopologyChangeInterval);
                inTestMaxRetransmissionCount = inTestMonitoringCount - 1;
                inTestRetransmissionCount = 0;
                interconnTestReq(inTestDefaultInterval);
                inNodeState = CHK_IC;
            }
            delete packet;
            break;
        case CHK_IC:
            if (sourceAddress == localBridgeAddress) {
                inTestMaxRetransmissionCount = inTestMonitoringCount - 1;
                inTestRetransmissionCount = 0;
            }
            delete packet;
            break;
        case POWER_ON:
            delete packet;
            break;
        case PT:
        case IP_IDLE:
            //Forwarding is done on relay level, not necessary with implemented ingress filter needed for MIM
            /*
             if (RingPort == interconnectionPort)
             mrpForwardReq(INTEST, RingPort, MC_INTEST,packet);
             else
             //other case are covered by automatic forwarding from ring to interconnection on relay level
             //setting "addMactoFDB" in client initialization
             delete packet;
             */
            break;
        default:
            throw cRuntimeError("Unknown Node State");
        }
    }
    else {
        EV_INFO << "Received Frame from other InterConnectionID"
                       << EV_FIELD(ringPort) << EV_FIELD(inID) << EV_ENDL;
        delete packet;
    }
}

void MrpInterconnection::interconnTestReq(simtime_t time)
{
    if (!inLinkTestTimer->isScheduled()) {
        scheduleAfter(time, inLinkTestTimer);
        setupInterconnTestReq();
    } else
        EV_DETAIL << "inTest already scheduled" << EV_ENDL;
}

void MrpInterconnection::setupInterconnTestReq()
{
    //Create MRP-PDU according MRP_InTest
    auto version = makeShared<MrpVersion>();
    auto inTestTLV1 = makeShared<MrpInTest>();
    auto inTestTLV2 = makeShared<MrpInTest>();
    auto inTestTLV3 = makeShared<MrpInTest>();
    auto commonTLV = makeShared<MrpCommon>();
    auto endTLV = makeShared<MrpEnd>();

    uint32_t timestamp = simTime().inUnit(SIMTIME_MS);
    inTestFrameSent.insert( { sequenceID, simTime() });

    inTestTLV1->setInID(interconnectionID);
    inTestTLV1->setSa(localBridgeAddress);
    inTestTLV1->setInState(inTopologyState);
    inTestTLV1->setTransition(transition);
    inTestTLV1->setTimeStamp(timestamp);
    inTestTLV1->setPortRole(MrpInterfaceData::INTERCONNECTION);

    inTestTLV2->setInID(interconnectionID);
    inTestTLV2->setSa(localBridgeAddress);
    inTestTLV2->setInState(inTopologyState);
    inTestTLV2->setTransition(transition);
    inTestTLV2->setTimeStamp(timestamp);
    inTestTLV2->setPortRole(MrpInterfaceData::PRIMARY);

    inTestTLV3->setInID(interconnectionID);
    inTestTLV3->setSa(localBridgeAddress);
    inTestTLV3->setInState(inTopologyState);
    inTestTLV3->setTransition(transition);
    inTestTLV3->setTimeStamp(timestamp);
    inTestTLV3->setPortRole(MrpInterfaceData::SECONDARY);

    commonTLV->setSequenceID(sequenceID);
    sequenceID++;
    commonTLV->setUuid0(domainID.uuid0);
    commonTLV->setUuid1(domainID.uuid1);

    auto packet1 = new Packet("InterconnTest");
    packet1->insertAtBack(version);
    packet1->insertAtBack(inTestTLV1);
    packet1->insertAtBack(commonTLV);
    packet1->insertAtBack(endTLV);
    MacAddress sourceAddress1 = getPortNetworkInterface(interconnectionPortId)->getMacAddress();
    sendFrameReq(interconnectionPortId, MacAddress(MC_INTEST), sourceAddress1, priority, MRP_LT, packet1);

    auto packet2 = new Packet("InterconnTest");
    packet2->insertAtBack(version);
    packet2->insertAtBack(inTestTLV2);
    packet2->insertAtBack(commonTLV);
    packet2->insertAtBack(endTLV);
    MacAddress sourceAddress2 = getPortNetworkInterface(primaryRingPortId)->getMacAddress();
    sendFrameReq(primaryRingPortId, MacAddress(MC_INTEST), sourceAddress2, priority, MRP_LT, packet2);

    auto packet3 = new Packet("InterconnTest");
    packet3->insertAtBack(version);
    packet3->insertAtBack(inTestTLV3);
    packet3->insertAtBack(commonTLV);
    packet3->insertAtBack(endTLV);
    MacAddress sourceAddress3 = getPortNetworkInterface(primaryRingPortId)->getMacAddress();
    sendFrameReq(secondaryRingPortId, MacAddress(MC_INTEST), sourceAddress3, priority, MRP_LT, packet3);
}

void MrpInterconnection::interconnTopologyChangeReq(simtime_t time)
{
    if (time == 0) {
        clearLocalFDB();
        setupInterconnTopologyChangeReq(inTopologyChangeMaxRepeatCount * time);
    } else if (!inTopologyChangeTimer->isScheduled()) {
        scheduleAfter(inTopologyChangeInterval, inTopologyChangeTimer);
        setupInterconnTopologyChangeReq(inTopologyChangeMaxRepeatCount * time);
    } else
        EV_DETAIL << "inTopologyChangeTimer already scheduled" << EV_ENDL;
}

void MrpInterconnection::setupInterconnTopologyChangeReq(simtime_t time)
{
    //Create MRP-PDU according MRP_InTopologyChange
    auto version = makeShared<MrpVersion>();
    auto inTopologyChangeTLV = makeShared<MrpInTopologyChange>();
    auto commonTLV = makeShared<MrpCommon>();
    auto endTLV = makeShared<MrpEnd>();

    inTopologyChangeTLV->setInID(interconnectionID);
    inTopologyChangeTLV->setSa(localBridgeAddress);
    inTopologyChangeTLV->setInterval(time.inUnit(SIMTIME_MS));

    commonTLV->setSequenceID(sequenceID);
    sequenceID++;
    commonTLV->setUuid0(domainID.uuid0);
    commonTLV->setUuid1(domainID.uuid1);

    auto packet1 = new Packet("InterconnTopologyChange");
    packet1->insertAtBack(version);
    packet1->insertAtBack(inTopologyChangeTLV);
    packet1->insertAtBack(commonTLV);
    packet1->insertAtBack(endTLV);
    MacAddress sourceAddress1 = getPortNetworkInterface(primaryRingPortId)->getMacAddress();
    sendFrameReq(primaryRingPortId, MacAddress(MC_INCONTROL), sourceAddress1, priority, MRP_LT, packet1);

    auto packet2 = new Packet("InterconnTopologyChange");
    packet2->insertAtBack(version);
    packet2->insertAtBack(inTopologyChangeTLV);
    packet2->insertAtBack(commonTLV);
    packet2->insertAtBack(endTLV);
    MacAddress sourceAddress2 = getPortNetworkInterface(secondaryRingPortId)->getMacAddress();
    sendFrameReq(secondaryRingPortId, MacAddress(MC_INCONTROL), sourceAddress2, priority, MRP_LT, packet2);

    auto packet3 = new Packet("InterconnTopologyChange");
    packet3->insertAtBack(version);
    packet3->insertAtBack(inTopologyChangeTLV);
    packet3->insertAtBack(commonTLV);
    packet3->insertAtBack(endTLV);
    MacAddress sourceAddress3 = getPortNetworkInterface(interconnectionPortId)->getMacAddress();
    if (inLinkCheckEnabled) {
        sendFrameReq(interconnectionPortId, MacAddress(MC_INTRANSFER), sourceAddress3, priority, MRP_LT, packet3);
    }
    else {
        sendFrameReq(interconnectionPortId, MacAddress(MC_INCONTROL), sourceAddress3, priority, MRP_LT, packet3);
    }
    emit(inTopologyChangeAnnouncedSignal, 1);
}

void MrpInterconnection::interconnLinkChangeReq(LinkState linkState, simtime_t time)
{
    //Create MRP-PDU according MRP_InLinkUp or MRP_InLinkDown
    auto version = makeShared<MrpVersion>();
    auto inLinkChangeTLV1 = makeShared<MrpInLinkChange>();
    auto inLinkChangeTLV2 = makeShared<MrpInLinkChange>();
    auto inLinkChangeTLV3 = makeShared<MrpInLinkChange>();
    auto commonTLV = makeShared<MrpCommon>();
    auto endTLV = makeShared<MrpEnd>();

    TlvHeaderType type = INLINKDOWN;
    if (linkState == LinkState::UP) {
        type = INLINKUP;
    } else if (linkState == LinkState::DOWN) {
        type = INLINKDOWN;
    }

    inLinkChangeTLV1->setHeaderType(type);
    inLinkChangeTLV1->setInID(interconnectionID);
    inLinkChangeTLV1->setSa(localBridgeAddress);
    inLinkChangeTLV1->setPortRole(MrpInterfaceData::PRIMARY);
    inLinkChangeTLV1->setInterval(time.inUnit(SIMTIME_MS));
    inLinkChangeTLV1->setLinkInfo(0x00); // "sent from the primary link"

    inLinkChangeTLV2->setHeaderType(type);
    inLinkChangeTLV2->setInID(interconnectionID);
    inLinkChangeTLV2->setSa(localBridgeAddress);
    inLinkChangeTLV2->setPortRole(MrpInterfaceData::SECONDARY);
    inLinkChangeTLV2->setInterval(time.inUnit(SIMTIME_MS));
    inLinkChangeTLV2->setLinkInfo(0x01); // "sent from the secondary link"

    inLinkChangeTLV3->setHeaderType(type);
    inLinkChangeTLV3->setInID(interconnectionID);
    inLinkChangeTLV3->setSa(localBridgeAddress);
    inLinkChangeTLV3->setPortRole(MrpInterfaceData::INTERCONNECTION);
    inLinkChangeTLV3->setInterval(time.inUnit(SIMTIME_MS));
    inLinkChangeTLV3->setLinkInfo(0x00);

    commonTLV->setSequenceID(sequenceID);
    sequenceID++;
    commonTLV->setUuid0(domainID.uuid0);
    commonTLV->setUuid1(domainID.uuid1);

    auto packet1 = new Packet("interconnLinkChange");
    packet1->insertAtBack(version);
    packet1->insertAtBack(inLinkChangeTLV1);
    packet1->insertAtBack(commonTLV);
    packet1->insertAtBack(endTLV);
    MacAddress sourceAddress1 = getPortNetworkInterface(primaryRingPortId)->getMacAddress();
    sendFrameReq(primaryRingPortId, MacAddress(MC_INCONTROL), sourceAddress1, priority, MRP_LT, packet1);

    auto packet2 = new Packet("interconnLinkChange");
    packet2->insertAtBack(version);
    packet2->insertAtBack(inLinkChangeTLV2);
    packet2->insertAtBack(commonTLV);
    packet2->insertAtBack(endTLV);
    MacAddress sourceAddress2 = getPortNetworkInterface(secondaryRingPortId)->getMacAddress();
    sendFrameReq(secondaryRingPortId, MacAddress(MC_INCONTROL), sourceAddress2, priority, MRP_LT, packet2);

    //sending out change notification to Interface which just changed.....
    //in text declaration of standard not requested
    //in function description explicitly requested
    auto packet3 = new Packet("interconnLinkChange");
    packet3->insertAtBack(version);
    packet3->insertAtBack(inLinkChangeTLV3);
    packet3->insertAtBack(commonTLV);
    packet3->insertAtBack(endTLV);
    MacAddress sourceAddress3 = getPortNetworkInterface(interconnectionPortId)->getMacAddress();
    if (inLinkCheckEnabled) {
        sendFrameReq(interconnectionPortId, MacAddress(MC_INTRANSFER), sourceAddress3, priority, MRP_LT, packet3);
    }
    else {
        sendFrameReq(interconnectionPortId, MacAddress(MC_INCONTROL), sourceAddress3, priority, MRP_LT, packet3);
    }
    emit(inLinkChangeDetectedSignal, linkState == LinkState::DOWN ? 0 : 1);
}

void MrpInterconnection::interconnLinkStatusPollReq(simtime_t time)
{
    if (!inLinkStatusPollTimer->isScheduled() && time > 0) {
        scheduleAfter(time, inLinkStatusPollTimer);
    } else
        EV_DETAIL << "inLinkStatusPoll already scheduled" << EV_ENDL;
    setupInterconnLinkStatusPollReq();
}

void MrpInterconnection::setupInterconnLinkStatusPollReq()
{
    //Create MRP-PDU according MRP_In_LinkStatusPollRequest
    auto version = makeShared<MrpVersion>();
    auto inLinkStatusPollTLV1 = makeShared<MrpInLinkStatusPoll>();
    auto inLinkStatusPollTLV2 = makeShared<MrpInLinkStatusPoll>();
    auto inLinkStatusPollTLV3 = makeShared<MrpInLinkStatusPoll>();
    auto commonTLV = makeShared<MrpCommon>();
    auto endTLV = makeShared<MrpEnd>();

    inLinkStatusPollTLV1->setInID(interconnectionID);
    inLinkStatusPollTLV1->setSa(localBridgeAddress);
    inLinkStatusPollTLV1->setPortRole(MrpInterfaceData::INTERCONNECTION);

    inLinkStatusPollTLV2->setInID(interconnectionID);
    inLinkStatusPollTLV2->setSa(localBridgeAddress);
    inLinkStatusPollTLV2->setPortRole(MrpInterfaceData::PRIMARY);

    inLinkStatusPollTLV3->setInID(interconnectionID);
    inLinkStatusPollTLV3->setSa(localBridgeAddress);
    inLinkStatusPollTLV3->setPortRole(MrpInterfaceData::SECONDARY);

    commonTLV->setSequenceID(sequenceID);
    sequenceID++;
    commonTLV->setUuid0(domainID.uuid0);
    commonTLV->setUuid1(domainID.uuid1);

    auto packet1 = new Packet("InterconnLinkStatusPoll");
    packet1->insertAtBack(version);
    packet1->insertAtBack(inLinkStatusPollTLV1);
    packet1->insertAtBack(commonTLV);
    packet1->insertAtBack(endTLV);
    MacAddress sourceAddress1 = getPortNetworkInterface(interconnectionPortId)->getMacAddress();
    sendFrameReq(interconnectionPortId, MacAddress(MC_INTRANSFER), sourceAddress1, priority, MRP_LT, packet1);

    auto packet2 = new Packet("InterconnLinkStatusPoll");
    packet2->insertAtBack(version);
    packet2->insertAtBack(inLinkStatusPollTLV2);
    packet2->insertAtBack(commonTLV);
    packet2->insertAtBack(endTLV);
    MacAddress sourceAddress2 = getPortNetworkInterface(primaryRingPortId)->getMacAddress();
    sendFrameReq(primaryRingPortId, MacAddress(MC_INCONTROL), sourceAddress2, priority, MRP_LT, packet2);

    auto packet3 = new Packet("InterconnLinkStatusPoll");
    packet3->insertAtBack(version);
    packet3->insertAtBack(inLinkStatusPollTLV3);
    packet3->insertAtBack(commonTLV);
    packet3->insertAtBack(endTLV);
    MacAddress sourceAddress3 = getPortNetworkInterface(primaryRingPortId)->getMacAddress();
    sendFrameReq(secondaryRingPortId, MacAddress(MC_INCONTROL), sourceAddress3, priority, MRP_LT, packet3);
    emit(inStatusPollSentSignal, 1);
}

void MrpInterconnection::inTransferReq(TlvHeaderType headerType, int ringPort, FrameType frameType, Packet *packet)
{
    auto macAddressInd = packet->getTag<MacAddressInd>();
    auto sourceAddress = macAddressInd->getSrcAddress();
    packet->trim();
    packet->clearTags();
    EV_INFO << "Received Frame from Ring. forwarding on InterConnection" << EV_FIELD(packet) << EV_ENDL;
    sendFrameReq(interconnectionPortId, MacAddress(frameType), sourceAddress, priority, MRP_LT, packet);
}

void MrpInterconnection::mrpForwardReq(TlvHeaderType headerType, int ringport, FrameType frameType, Packet *packet)
{
    auto macAddressInd = packet->getTag<MacAddressInd>();
    auto sourceAddress = macAddressInd->getSrcAddress();
    packet->trim();
    packet->clearTags();
    EV_INFO << "Received Frame from InterConnection. forwarding on Ring" << EV_FIELD(packet) << EV_ENDL;
    sendFrameReq(primaryRingPortId, MacAddress(frameType), sourceAddress, priority, MRP_LT, packet->dup());
    sendFrameReq(secondaryRingPortId, MacAddress(frameType), sourceAddress, priority, MRP_LT, packet);
}

std::string MrpInterconnection::resolveDirective(char directive) const
{
    switch (directive) {
        case 'R': return getInterconnectionRoleName(inRole, true);
        case 'N': return getInterconnectionNodeStateName(inNodeState);
        case 'I': return getInterconnectionStateName(inTopologyState);
        default: return Mrp::resolveDirective(directive);
    }
}

const char *MrpInterconnection::getInterconnectionRoleName(InterconnectionRole role, bool acronym)
{
    switch (role) {
        case INTERCONNECTION_CLIENT: return acronym ? "MIC" : "INTERCONNECTION_CLIENT";
        case INTERCONNECTION_MANAGER: return acronym ? "MIM" : "INTERCONNECTION_MANAGER";
        default: return "???";
    }
}

const char *MrpInterconnection::getInterconnectionNodeStateName(InterconnectionNodeState state)
{
    switch (state) {
        case POWER_ON: return "POWER_ON";
        case AC_STAT1: return "AC_STAT1";
        case CHK_IO: return "CHK_IO";
        case CHK_IC: return "CHK_IC";
        case PT: return "PT";
        case IP_IDLE: return "IP_IDLE";
        default: return "???";
    }
}

const char *MrpInterconnection::getInterconnectionStateName(InterconnectionTopologyState state)
{
    switch (state) {
        case OPEN: return "OPEN";
        case CLOSED: return "CLOSED";
        default: return "???";
    }
}

} // namespace inet

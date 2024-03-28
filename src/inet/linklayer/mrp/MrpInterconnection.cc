// Copyright (C) 2024 Daniel Zeitler
// SPDX-License-Identifier: LGPL-3.0-or-later

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

Define_Module(MrpInterconnection);

MrpInterconnection::MrpInterconnection() {
}

MrpInterconnection::~MrpInterconnection() {
    cancelAndDelete(inLinkStatusPollTimer);
    cancelAndDelete(inLinkTestTimer);
    cancelAndDelete(inTopologyChangeTimer);
    cancelAndDelete(inLinkDownTimer);
    cancelAndDelete(inLinkUpTimer);
    //Mrp::~Mrp();
}

void MrpInterconnection::initialize(int stage) {
    Mrp::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        inRole = static_cast<InRoleState>(par("interconnectionRole").intValue());
        interConnectionID = par("interconnectionID");
        interconnectionPort = par("interconnectionPort");
        linkCheckEnabled = par("linkCheckEnabled");
        ringCheckEnabled = par("ringCheckEnabled");

        inLinkChangeSignal = registerSignal("inLinkChange");
        inTopologyChangeSignal = registerSignal("inTopologyChange");
        inTestSignal = registerSignal("inTest");
        receivedInChangeSignal = registerSignal("receivedInChange");
        receivedInTestSignal = registerSignal("receivedInTest");
        interconnectionStateChangedSignal = registerSignal("interconnectionStateChanged");
        inStatusPollSignal = registerSignal("inStatusPoll");
        receivedInStatusPollSignal = registerSignal("receivedInStatusPoll");
    }
    if (stage == INITSTAGE_LINK_LAYER) {
        EV_DETAIL << "Initialize Interconnection Stage link layer" << EV_ENDL;
        setInterconnectionInterface(interconnectionPort);
        initInterconnectionPort();
    }
}

void MrpInterconnection::initInterconnectionPort() {
    if (interconnectionInterface == nullptr)
        setInterconnectionInterface(interconnectionPort);
    auto ifd = getPortInterfaceDataForUpdate(interconnectionInterface->getInterfaceId());
    ifd->setRole(MrpInterfaceData::INTERCONNECTION);
    ifd->setState(MrpInterfaceData::BLOCKED);
    ifd->setContinuityCheck(linkCheckEnabled);
    EV_DETAIL << "Initialize InterconnectionPort:"
              << EV_FIELD(ifd->getContinuityCheck())
              << EV_FIELD(ifd->getRole())
              << EV_FIELD(ifd->getState())
              << EV_ENDL;
}

void MrpInterconnection::start() {
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

void MrpInterconnection::stop() {
    cancelAndDelete(inLinkStatusPollTimer);
    cancelAndDelete(inLinkTestTimer);
    cancelAndDelete(inTopologyChangeTimer);
    cancelAndDelete(inLinkDownTimer);
    cancelAndDelete(inLinkUpTimer);
    Mrp::stop();
}

void MrpInterconnection::mimInit() {
    mrpMacForwardingTable->addMrpForwardingInterface(primaryRingPort, MacAddress(MC_INCONTROL), vlanID);
    mrpMacForwardingTable->addMrpForwardingInterface(secondaryRingPort, MacAddress(MC_INCONTROL), vlanID);
    //MIM may not forward mrp frames received on Interconnection Port to Ring, adding Filter
    mrpMacForwardingTable->addMrpIngressFilterInterface(interconnectionPort, MacAddress(MC_INCONTROL), vlanID);
    mrpMacForwardingTable->addMrpIngressFilterInterface(interconnectionPort, MacAddress(MC_INTEST), vlanID);

    relay->registerAddress(MacAddress(MC_INCONTROL));
    if (linkCheckEnabled) {
        relay->registerAddress(MacAddress(MC_INTRANSFER));
        handleInLinkStatusPollTimer();
        if (!enableLinkCheckOnRing)
            startContinuityCheck();
    }
    if (ringCheckEnabled) {
        relay->registerAddress(MacAddress(MC_INTEST));
        inLinkTestTimer = new cMessage("inLinkTestTimer");
    }
    inState = AC_STAT1;
    EV_DETAIL << "Interconnection Manager is started, Switching InState from POWER_ON to AC_STAT1"
                     << EV_FIELD(inState) << EV_ENDL;
    mauTypeChangeInd(interconnectionPort, getPortNetworkInterface(interconnectionPort)->getState());
}

void MrpInterconnection::micInit() {
    if (ringCheckEnabled) {
        mrpMacForwardingTable->addMrpForwardingInterface(primaryRingPort, MacAddress(MC_INTEST), vlanID);
        mrpMacForwardingTable->addMrpForwardingInterface(secondaryRingPort, MacAddress(MC_INTEST), vlanID);
        mrpMacForwardingTable->addMrpForwardingInterface(interconnectionPort, MacAddress(MC_INTEST), vlanID);
        /*
         *Ingress filter can now be set by MRP, therefore not need for forwarding on mrp level
         //frames received on Interconnection port are not forwarded on relay level (ingress) filter
         // and have to be forwarded by MIC
         relay->registerAddress(MacAddress(MC_INTEST));
         */
        mrpMacForwardingTable->addMrpForwardingInterface(primaryRingPort, MacAddress(MC_INCONTROL), vlanID);
        mrpMacForwardingTable->addMrpForwardingInterface(secondaryRingPort, MacAddress(MC_INCONTROL), vlanID);
    }
    if (linkCheckEnabled) {
        relay->registerAddress(MacAddress(MC_INTRANSFER));
        if (!enableLinkCheckOnRing)
            startContinuityCheck();
    }
    relay->registerAddress(MacAddress(MC_INCONTROL));
    inState = AC_STAT1;
    EV_DETAIL << "Interconnection Client is started, Switching InState from POWER_ON to AC_STAT1" << EV_FIELD(inState) << EV_ENDL;
    mauTypeChangeInd(interconnectionPort, getPortNetworkInterface(interconnectionPort)->getState());
}

void MrpInterconnection::setInterconnectionInterface(int interfaceIndex) {
    interconnectionInterface = interfaceTable->getInterface(interfaceIndex);
    if (interconnectionInterface->isLoopback()) {
        interconnectionInterface = nullptr;
        EV_DEBUG << "Chosen Interface is Loopback-Interface" << EV_ENDL;
    } else {
        interconnectionPort = interconnectionInterface->getInterfaceId();
    }
}

void MrpInterconnection::setTimingProfile(int maxRecoveryTime) {
    Mrp::setTimingProfile(maxRecoveryTime);
    switch (maxRecoveryTime) {
    case 500:
        inLinkChangeInterval_ms = 20;
        inTopologyChangeInterval_ms = 20;
        inLinkStatusPollInterval_ms = 20;
        inTestDefaultInterval_ms = 50;
        break;
    case 200:
        inLinkChangeInterval_ms = 20;
        inTopologyChangeInterval_ms = 10;
        inLinkStatusPollInterval_ms = 20;
        inTestDefaultInterval_ms = 20;
        break;
    case 30:
        inLinkChangeInterval_ms = 3;
        inTopologyChangeInterval_ms = 1;
        inLinkStatusPollInterval_ms = 3;
        inTestDefaultInterval_ms = 3;
        break;
    case 10:
        inLinkChangeInterval_ms = 3;
        inTopologyChangeInterval_ms = 1;
        inLinkStatusPollInterval_ms = 3;
        inTestDefaultInterval_ms = 3;
        break;
    default:
        throw cRuntimeError("Only RecoveryTimes 500, 200, 30 and 10 ms are defined!");
    }
}

void MrpInterconnection::handleStartOperation(LifecycleOperation *operation) {
    //start();
}

void MrpInterconnection::handleStopOperation(LifecycleOperation *operation) {
    stop();
}

void MrpInterconnection::handleCrashOperation(LifecycleOperation *operation) {
    stop();
}

void MrpInterconnection::handleMessageWhenUp(cMessage *msg) {
    if (!msg->isSelfMessage()) {
        msg->setKind(2);
        EV_INFO << "Received Message on InterConnectionNode, Rescheduling:"
                       << EV_FIELD(msg) << EV_ENDL;
        processingDelay = SimTime(par("processingDelay").doubleValue(), SIMTIME_US);
        scheduleAt(simTime() + processingDelay, msg);
    } else {
        EV_INFO << "Received Self-Message:" << EV_FIELD(msg) << EV_ENDL;
        EV_DEBUG << "State:" << EV_FIELD(inState) << EV_ENDL;
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
                handleContinuityCheckMessage(packet);
            }
            if (protocol == &Protocol::mrp) {
                handleMrpPDU(packet);
            }
        } else
            throw cRuntimeError("Unknown self-message received");
    }
}

void MrpInterconnection::handleInTestTimer() {
    switch (inState) {
    case CHK_IO:
        interconnTestReq(inTestDefaultInterval_ms);
        break;
    case CHK_IC:
        interconnTestReq(inTestDefaultInterval_ms);
        if (inTestRetransmissionCount >= inTestMaxRetransmissionCount) {
            setPortState(interconnectionPort, MrpInterfaceData::FORWARDING);
            inTestMaxRetransmissionCount = inTestMonitoringCount - 1;
            inTestRetransmissionCount = 0;
            interconnTopologyChangeReq(inTopologyChangeInterval_ms);
            inState = CHK_IO;
        } else {
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

void MrpInterconnection::handleInLinkStatusPollTimer() {
    if (inLinkStatusPollCount > 0) {
        inLinkStatusPollCount--;
        scheduleAt(simTime() + SimTime(inLinkStatusPollInterval_ms, SIMTIME_MS), inLinkStatusPollTimer);
    } else if (inLinkStatusPollCount == 0) {
        inLinkStatusPollCount = inLinkStatusPollMaxCount - 1;
    }
    setupInterconnLinkStatusPollReq();
}

void MrpInterconnection::handleInTopologyChangeTimer() {
    if (intopologyChangeRepeatCount > 0) {
        setupInterconnTopologyChangeReq(intopologyChangeRepeatCount * inTopologyChangeInterval_ms);
        intopologyChangeRepeatCount--;
        scheduleAt(simTime() + SimTime(inTopologyChangeInterval_ms, SIMTIME_MS), inTopologyChangeTimer);
    } else if (intopologyChangeRepeatCount == 0) {
        intopologyChangeRepeatCount = inTopologyChangeMaxRepeatCount - 1;
        clearFDB(0);
        setupInterconnTopologyChangeReq(0);
    }
}

void MrpInterconnection::handleInLinkUpTimer() {
    inLinkChangeCount--;
    switch (inState) {
    case PT:
        if (inLinkChangeCount == 0) {
            inLinkChangeCount = inLinkMaxChange;
            setPortState(interconnectionPort, MrpInterfaceData::FORWARDING);
            inState = IP_IDLE;
        } else {
            scheduleAt(simTime() + SimTime(inLinkChangeInterval_ms, SIMTIME_MS), inLinkUpTimer);
            interconnLinkChangeReq(LinkState::UP, inLinkChangeCount * inLinkChangeInterval_ms);
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

void MrpInterconnection::handleInLinkDownTimer() {
    inLinkChangeCount--;
    switch (inState) {
    case AC_STAT1:
        if (inRole == INTERCONNECTION_CLIENT) {
            if (inLinkChangeCount == 0) {
                inLinkChangeCount = inLinkMaxChange;
            } else {
                scheduleAt(simTime() + SimTime(inLinkChangeInterval_ms, SIMTIME_MS), inLinkDownTimer);
                interconnLinkChangeReq(LinkState::DOWN, inLinkChangeCount * inLinkChangeInterval_ms);
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

void MrpInterconnection::mauTypeChangeInd(int ringPort, LinkState linkState) {
    if (ringPort == interconnectionPort) {
        switch (inState) {
        case AC_STAT1:
            if (linkState == LinkState::UP) {
                if (inRole == INTERCONNECTION_MANAGER) {
                    setPortState(ringPort, MrpInterfaceData::BLOCKED);
                    if (linkCheckEnabled) {
                        interconnLinkStatusPollReq(inLinkStatusPollInterval_ms);
                    }
                    if (ringCheckEnabled) {
                        inTestMaxRetransmissionCount = inTestMonitoringCount - 1;
                        inTestRetransmissionCount = 0;
                        interconnTestReq(inTestDefaultInterval_ms);
                    }
                    inState = CHK_IC;
                    EV_DETAIL << "Switching InState from AC_STAT1 to CHK_IC"
                                     << EV_FIELD(inState) << EV_ENDL;
                    currentInterconnectionState = CLOSED;
                    emit(interconnectionStateChangedSignal, simTime().inUnit(SIMTIME_US));
                } else if (inRole == INTERCONNECTION_CLIENT) {
                    inLinkChangeCount = inLinkMaxChange;
                    cancelEvent(inLinkDownTimer);
                    scheduleAt(simTime() + SimTime(inLinkChangeInterval_ms, SIMTIME_MS), inLinkUpTimer);
                    interconnLinkChangeReq(LinkState::UP, inLinkChangeCount * inLinkChangeInterval_ms);
                    inState = PT;
                    EV_DETAIL << "Switching InState from AC_STAT1 to PT"
                                     << EV_FIELD(inState) << EV_ENDL;
                }
            }
            break;
        case CHK_IO:
            if (linkState == LinkState::DOWN) {
                setPortState(ringPort, MrpInterfaceData::BLOCKED);
                if (linkCheckEnabled) {
                    interconnLinkStatusPollReq(inLinkStatusPollInterval_ms);
                }
                if (ringCheckEnabled) {
                    interconnTopologyChangeReq(inTopologyChangeInterval_ms);
                    interconnTestReq(inTestDefaultInterval_ms);
                }
                inState = AC_STAT1;
                EV_DETAIL << "Switching InState from CHK_IO to AC_STAT1"
                                 << EV_FIELD(inState) << EV_ENDL;
            }
            break;
        case CHK_IC:
            if (linkState == LinkState::DOWN) {
                setPortState(ringPort, MrpInterfaceData::BLOCKED);
                if (ringCheckEnabled) {
                    interconnTopologyChangeReq(inTopologyChangeInterval_ms);
                    interconnTestReq(inTestDefaultInterval_ms);
                }
                inState = AC_STAT1;
                EV_DETAIL << "Switching InState from CHK_IC to AC_STAT1"
                                 << EV_FIELD(inState) << EV_ENDL;
            }
            break;
        case PT:
            if (linkState == LinkState::DOWN) {
                inLinkChangeCount = inLinkMaxChange;
                cancelEvent(inLinkUpTimer);
                setPortState(ringPort, MrpInterfaceData::BLOCKED);
                scheduleAt(simTime() + SimTime(inLinkChangeInterval_ms, SIMTIME_MS), inLinkDownTimer);
                interconnLinkChangeReq(LinkState::DOWN, inLinkChangeCount * inLinkChangeInterval_ms);
                inState = AC_STAT1;
                EV_DETAIL << "Switching InState from PT to AC_STAT1"
                                 << EV_FIELD(inState) << EV_ENDL;
            }
            break;
        case IP_IDLE:
            if (linkState == LinkState::DOWN) {
                inLinkChangeCount = inLinkMaxChange;
                setPortState(ringPort, MrpInterfaceData::BLOCKED);
                scheduleAt(simTime() + SimTime(inLinkChangeInterval_ms, SIMTIME_MS), inLinkDownTimer);
                interconnLinkChangeReq(LinkState::DOWN, inLinkChangeCount * inLinkChangeInterval_ms);
                inState = AC_STAT1;
                EV_DETAIL << "Switching InState from IP_IDLE to AC_STAT1"
                                 << EV_FIELD(inState) << EV_ENDL;
            }
            break;
        case POWER_ON:
            break;
        default:
            throw cRuntimeError("Unknown Node State");
        }
    } else {
        Mrp::mauTypeChangeInd(ringPort, linkState);
    }
}

void MrpInterconnection::interconnTopologyChangeInd(MacAddress sourceAddress, double time_ms, uint16_t inID, int ringPort, Packet *packet) {
    if (inID == interConnectionID) {
        auto offset = B(2);
        const auto &firstTLV = packet->peekDataAt<MrpInTopologyChange>(offset);
        offset = offset + firstTLV->getChunkLength();
        const auto &secondTLV = packet->peekDataAt<MrpCommon>(offset);
        uint16_t sequence = secondTLV->getSequenceID();
        //Poll is reaching Node from Primary, Secondary and Interconnection Port
        //it is not necessary to react after the first frame
        if (sequence > lastInTopologyId) {
            lastInTopologyId = sequence;
            emit(receivedInChangeSignal, firstTLV->getInterval());
            switch (inState) {
            case AC_STAT1:
                if (inRole == INTERCONNECTION_CLIENT) {
                    cancelEvent(inLinkDownTimer);
                } else if (inRole == INTERCONNECTION_MANAGER
                        && sourceAddress == localBridgeAddress) {
                    clearFDB(time_ms);
                }
                delete packet;
                break;
            case CHK_IO:
            case CHK_IC:
                if (sourceAddress == localBridgeAddress) {
                    clearFDB(time_ms);
                }
                delete packet;
                break;
            case PT:
                inLinkChangeCount = inLinkMaxChange;
                cancelEvent(inLinkUpTimer);
                setPortState(interconnectionPort, MrpInterfaceData::FORWARDING);
                inState = IP_IDLE;
                EV_DETAIL << "Switching InState from PT to IP_IDLE"
                                 << EV_FIELD(inState) << EV_ENDL;
                if (ringPort != interconnectionPort) {
                    if (linkCheckEnabled)
                        inTransferReq(INTOPOLOGYCHANGE, interconnectionPort, MC_INTRANSFER, packet);
                    else
                        inTransferReq(INTOPOLOGYCHANGE, interconnectionPort, MC_INCONTROL, packet);
                } else
                    mrpForwardReq(INTOPOLOGYCHANGE, ringPort, MC_INCONTROL, packet);
                break;
            case IP_IDLE:
                if (ringPort != interconnectionPort) {
                    if (linkCheckEnabled)
                        inTransferReq(INTOPOLOGYCHANGE, interconnectionPort, MC_INTRANSFER, packet);
                    else
                        inTransferReq(INTOPOLOGYCHANGE, interconnectionPort, MC_INCONTROL, packet);
                } else
                    mrpForwardReq(INTOPOLOGYCHANGE, ringPort, MC_INCONTROL, packet);
                break;
            case POWER_ON:
                delete packet;
                break;
            default:
                throw cRuntimeError("Unknown Node State");
            }
        } else {
            EV_DETAIL << "Received same Frame already" << EV_ENDL;
            delete packet;
        }
    } else {
        EV_INFO << "Received Frame from other InterConnectionID"
                       << EV_FIELD(inID) << EV_ENDL;
        delete packet;
    }
}

void MrpInterconnection::interconnLinkChangeInd(uint16_t InID, LinkState linkState, int ringPort, Packet *packet) {
    if (InID == interConnectionID) {
        auto firstTLV = packet->peekDataAt<MrpInLinkChange>(B(2));
        emit(receivedInChangeSignal, firstTLV->getInterval());
        switch (inState) {
        case CHK_IO:
            if (linkCheckEnabled) {
                if (linkState == LinkState::UP) {
                    setPortState(interconnectionPort, MrpInterfaceData::BLOCKED);
                    cancelEvent(inLinkStatusPollTimer);
                    interconnTopologyChangeReq(inTopologyChangeInterval_ms);
                    EV_INFO << "Interconnection Ring closed" << EV_ENDL;
                    inState = CHK_IC;
                    EV_DETAIL << "Switching InState from CHK_IO to CHK_IC"
                                     << EV_FIELD(inState) << EV_ENDL;
                    currentInterconnectionState = CLOSED;
                    emit(interconnectionStateChangedSignal, simTime().inUnit(SIMTIME_US));
                } else if (linkState == LinkState::DOWN) {
                    setPortState(interconnectionPort, MrpInterfaceData::FORWARDING);
                }
            }
            if (ringCheckEnabled) {
                if (linkState == LinkState::UP) {
                    interconnTestReq(inTestDefaultInterval_ms);
                }
            }
            delete packet;
            break;
        case CHK_IC:
            if (linkState == LinkState::DOWN) {
                setPortState(interconnectionPort, MrpInterfaceData::FORWARDING);
                interconnTopologyChangeReq(inTopologyChangeInterval_ms);
                if (linkCheckEnabled) {
                    cancelEvent(inLinkStatusPollTimer);
                    EV_INFO << "Interconnection Ring open" << EV_ENDL;
                    inState = CHK_IO;
                    EV_DETAIL << "Switching InState from CHK_IC to CHK_IO"
                                     << EV_FIELD(inState) << EV_ENDL;
                    currentInterconnectionState = OPEN;
                    emit(interconnectionStateChangedSignal, simTime().inUnit(SIMTIME_US));
                }
            }
            if (ringCheckEnabled && linkState == LinkState::UP) {
                inTestMaxRetransmissionCount = inTestMonitoringCount - 1;
                interconnTopologyChangeReq(inTopologyChangeInterval_ms);
            }
            delete packet;
            break;
        case PT:
        case IP_IDLE:
            if (ringPort == interconnectionPort && linkCheckEnabled) {
                if (linkState == LinkState::UP) {
                    mrpForwardReq(INLINKUP, ringPort, MC_INCONTROL, packet);
                } else if (linkState == LinkState::DOWN) {
                    mrpForwardReq(INLINKDOWN, ringPort, MC_INCONTROL, packet);
                } else
                    delete packet;
            } else if (ringPort != interconnectionPort) {
                if (ringCheckEnabled) {
                    interconnForwardReq(interconnectionPort, packet);
                } else if (linkState == LinkState::UP) {
                    inTransferReq(INLINKUP, interconnectionPort, MC_INTRANSFER, packet);
                } else if (linkState == LinkState::DOWN) {
                    inTransferReq(INLINKDOWN, interconnectionPort, MC_INTRANSFER, packet);
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
    } else {
        EV_INFO << "Received Frame from other InterConnectionID"
                       << EV_FIELD(ringPort) << EV_FIELD(InID) << EV_ENDL;
        delete packet;
    }
}

void MrpInterconnection::interconnLinkStatusPollInd(uint16_t inID, int ringPort, Packet *packet) {
    if (inID == interConnectionID) {
        b offset = B(2);
        auto firstTLV = packet->peekDataAt<MrpInLinkStatusPoll>(offset);
        offset = offset + firstTLV->getChunkLength();
        auto secondTLV = packet->peekDataAt<MrpCommon>(offset);
        uint16_t sequence = secondTLV->getSequenceID();
        //Poll is reaching Node from Primary, Secondary and Interconnection Port
        //it is not necessary to react after the first frame
        if (sequence > lastPollId) {
            lastPollId = sequence;
            emit(receivedInStatusPollSignal, simTime().inUnit(SIMTIME_US));
            switch (inState) {
            case AC_STAT1:
                if (inRole == INTERCONNECTION_CLIENT) {
                    interconnLinkChangeReq(LinkState::DOWN, 0);
                }
                delete packet;
                break;
            case PT:
            case IP_IDLE:
                interconnLinkChangeReq(LinkState::UP, 0);
                if (ringPort != interconnectionPort) {
                    if (linkCheckEnabled)
                        inTransferReq(INLINKSTATUSPOLL, interconnectionPort, MC_INTRANSFER, packet);
                    else
                        inTransferReq(INLINKSTATUSPOLL, interconnectionPort, MC_INCONTROL, packet);
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
        } else {
            EV_DETAIL << "Received same Frame already" << EV_ENDL;
            delete packet;
        }
    } else {
        EV_INFO << "Received Frame from other InterConnectionID"
                       << EV_FIELD(ringPort) << EV_FIELD(inID) << EV_ENDL;
        delete packet;
    }
}

void MrpInterconnection::interconnTestInd(MacAddress sourceAddress, int ringPort, uint16_t inID, Packet *packet) {
    if (inID == interConnectionID) {
        b offset = B(2);
        auto firstTLV = packet->peekDataAt<MrpInTest>(offset);
        offset = offset + firstTLV->getChunkLength();
        auto secondTLV = packet->peekDataAt<MrpCommon>(offset);
        uint16_t sequence = secondTLV->getSequenceID();
        int ringTime = simTime().inUnit(SIMTIME_MS) - firstTLV->getTimeStamp();
        auto lastInTestFrameSent = inTestFrameSent.find(sequence);
        if (lastInTestFrameSent != inTestFrameSent.end()) {
            int64_t ringTimePrecise = simTime().inUnit(SIMTIME_US) - lastInTestFrameSent->second;
            emit(receivedInTestSignal, ringTimePrecise);
            EV_DETAIL << "InterconnectionRingTime" << EV_FIELD(ringTime)
                             << EV_FIELD(ringTimePrecise) << EV_ENDL;
        } else {
            emit(receivedInTestSignal, ringTime * 1000);
            EV_DETAIL << "InterconnectionRingTime" << EV_FIELD(ringTime) << EV_ENDL;
        }
        switch (inState) {
        case AC_STAT1:
            if (inRole == INTERCONNECTION_MANAGER
                    && sourceAddress == localBridgeAddress) {
                setPortState(interconnectionPort, MrpInterfaceData::BLOCKED);
                inTestMaxRetransmissionCount = inTestMonitoringCount - 1;
                inTestRetransmissionCount = 0;
                interconnTestReq(inTestDefaultInterval_ms);
                inState = CHK_IC;
                EV_DETAIL << "Switching InState from AC_STAT1 to CHK_IC"
                                 << EV_FIELD(inState) << EV_ENDL;
            }
            delete packet;
            break;
        case CHK_IO:
            if (sourceAddress == localBridgeAddress) {
                setPortState(interconnectionPort, MrpInterfaceData::BLOCKED);
                interconnTopologyChangeReq(inTopologyChangeInterval_ms);
                inTestMaxRetransmissionCount = inTestMonitoringCount - 1;
                inTestRetransmissionCount = 0;
                interconnTestReq(inTestDefaultInterval_ms);
                inState = CHK_IC;
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
    } else {
        EV_INFO << "Received Frame from other InterConnectionID"
                       << EV_FIELD(ringPort) << EV_FIELD(inID) << EV_ENDL;
        delete packet;
    }
}

void MrpInterconnection::interconnTestReq(double time_ms) {
    if (!inLinkTestTimer->isScheduled()) {
        scheduleAt(simTime() + SimTime(time_ms, SIMTIME_MS), inLinkTestTimer);
        setupInterconnTestReq();
    } else
        EV_DETAIL << "inTest already scheduled" << EV_ENDL;
}

void MrpInterconnection::setupInterconnTestReq() {
    //Create MRP-PDU according MRP_InTest
    auto version = makeShared<MrpVersion>();
    auto inTestTLV1 = makeShared<MrpInTest>();
    auto inTestTLV2 = makeShared<MrpInTest>();
    auto inTestTLV3 = makeShared<MrpInTest>();
    auto commonTLV = makeShared<MrpCommon>();
    auto endTLV = makeShared<MrpEnd>();

    uint32_t timestamp = simTime().inUnit(SIMTIME_MS);
    int64_t lastInTestFrameSent = simTime().inUnit(SIMTIME_US);
    inTestFrameSent.insert( { sequenceID, lastInTestFrameSent });

    inTestTLV1->setInID(interConnectionID);
    inTestTLV1->setSa(localBridgeAddress);
    inTestTLV1->setInState(currentInterconnectionState);
    inTestTLV1->setTransition(transition);
    inTestTLV1->setTimeStamp(timestamp);
    inTestTLV1->setPortRole(MrpInterfaceData::INTERCONNECTION);

    inTestTLV2->setInID(interConnectionID);
    inTestTLV2->setSa(localBridgeAddress);
    inTestTLV2->setInState(currentInterconnectionState);
    inTestTLV2->setTransition(transition);
    inTestTLV2->setTimeStamp(timestamp);
    inTestTLV2->setPortRole(MrpInterfaceData::PRIMARY);

    inTestTLV3->setInID(interConnectionID);
    inTestTLV3->setSa(localBridgeAddress);
    inTestTLV3->setInState(currentInterconnectionState);
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
    MacAddress sourceAddress1 = getPortNetworkInterface(interconnectionPort)->getMacAddress();
    sendFrameReq(interconnectionPort, MacAddress(MC_INTEST), sourceAddress1, priority, MRP_LT, packet1);

    auto packet2 = new Packet("InterconnTest");
    packet2->insertAtBack(version);
    packet2->insertAtBack(inTestTLV2);
    packet2->insertAtBack(commonTLV);
    packet2->insertAtBack(endTLV);
    MacAddress sourceAddress2 = getPortNetworkInterface(primaryRingPort)->getMacAddress();
    sendFrameReq(primaryRingPort, MacAddress(MC_INTEST), sourceAddress2, priority, MRP_LT, packet2);

    auto packet3 = new Packet("InterconnTest");
    packet3->insertAtBack(version);
    packet3->insertAtBack(inTestTLV3);
    packet3->insertAtBack(commonTLV);
    packet3->insertAtBack(endTLV);
    MacAddress sourceAddress3 = getPortNetworkInterface(primaryRingPort)->getMacAddress();
    sendFrameReq(secondaryRingPort, MacAddress(MC_INTEST), sourceAddress3, priority, MRP_LT, packet3);
    emit(inTestSignal, lastInTestFrameSent);
}

void MrpInterconnection::interconnTopologyChangeReq(double time_ms) {  //TODO see comments for the similar code in Mrp.cc
    if (time_ms == 0) {
        clearLocalFDB();
        setupInterconnTopologyChangeReq(inTopologyChangeMaxRepeatCount * time_ms);
    } else if (!inTopologyChangeTimer->isScheduled()) {
        scheduleAt(simTime() + SimTime(inTopologyChangeInterval_ms, SIMTIME_MS), inTopologyChangeTimer);
        setupInterconnTopologyChangeReq(inTopologyChangeMaxRepeatCount * time_ms);
    } else
        EV_DETAIL << "inTopologyChangeTimer already scheduled" << EV_ENDL;
}

void MrpInterconnection::setupInterconnTopologyChangeReq(double time_ms) {
    //Create MRP-PDU according MRP_InTopologyChange
    auto version = makeShared<MrpVersion>();
    auto inTopologyChangeTLV = makeShared<MrpInTopologyChange>();
    auto commonTLV = makeShared<MrpCommon>();
    auto endTLV = makeShared<MrpEnd>();

    inTopologyChangeTLV->setInID(interConnectionID);
    inTopologyChangeTLV->setSa(localBridgeAddress);
    inTopologyChangeTLV->setInterval(time_ms);

    commonTLV->setSequenceID(sequenceID);
    sequenceID++;
    commonTLV->setUuid0(domainID.uuid0);
    commonTLV->setUuid1(domainID.uuid1);

    auto packet1 = new Packet("InterconnTopologyChange");
    packet1->insertAtBack(version);
    packet1->insertAtBack(inTopologyChangeTLV);
    packet1->insertAtBack(commonTLV);
    packet1->insertAtBack(endTLV);
    MacAddress sourceAddress1 = getPortNetworkInterface(primaryRingPort)->getMacAddress();
    sendFrameReq(primaryRingPort, MacAddress(MC_INCONTROL), sourceAddress1, priority, MRP_LT, packet1);

    auto packet2 = new Packet("InterconnTopologyChange");
    packet2->insertAtBack(version);
    packet2->insertAtBack(inTopologyChangeTLV);
    packet2->insertAtBack(commonTLV);
    packet2->insertAtBack(endTLV);
    MacAddress sourceAddress2 = getPortNetworkInterface(secondaryRingPort)->getMacAddress();
    sendFrameReq(secondaryRingPort, MacAddress(MC_INCONTROL), sourceAddress2, priority, MRP_LT, packet2);

    auto packet3 = new Packet("InterconnTopologyChange");
    packet3->insertAtBack(version);
    packet3->insertAtBack(inTopologyChangeTLV);
    packet3->insertAtBack(commonTLV);
    packet3->insertAtBack(endTLV);
    MacAddress sourceAddress3 = getPortNetworkInterface(interconnectionPort)->getMacAddress();
    if (linkCheckEnabled) {
        sendFrameReq(interconnectionPort, MacAddress(MC_INTRANSFER), sourceAddress3, priority, MRP_LT, packet3);
    } else {
        sendFrameReq(interconnectionPort, MacAddress(MC_INCONTROL), sourceAddress3, priority, MRP_LT, packet3);
    }
    emit(inTopologyChangeSignal, simTime().inUnit(SIMTIME_US));
}

void MrpInterconnection::interconnLinkChangeReq(LinkState linkState, double time_ms) {
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
    inLinkChangeTLV1->setInID(interConnectionID);
    inLinkChangeTLV1->setSa(localBridgeAddress);
    inLinkChangeTLV1->setPortRole(MrpInterfaceData::PRIMARY);
    inLinkChangeTLV1->setInterval(time_ms);
    inLinkChangeTLV1->setLinkInfo(0x00); // "sent from the primary link"

    inLinkChangeTLV2->setHeaderType(type);
    inLinkChangeTLV2->setInID(interConnectionID);
    inLinkChangeTLV2->setSa(localBridgeAddress);
    inLinkChangeTLV2->setPortRole(MrpInterfaceData::SECONDARY);
    inLinkChangeTLV2->setInterval(time_ms);
    inLinkChangeTLV2->setLinkInfo(0x01); // "sent from the secondary link"

    inLinkChangeTLV3->setHeaderType(type);
    inLinkChangeTLV3->setInID(interConnectionID);
    inLinkChangeTLV3->setSa(localBridgeAddress);
    inLinkChangeTLV3->setPortRole(MrpInterfaceData::INTERCONNECTION);
    inLinkChangeTLV3->setInterval(time_ms);
    inLinkChangeTLV3->setLinkInfo(0x00); //TODO or maybe 0x01? -- see "Table 39 – MRP_LinkInfo"

    commonTLV->setSequenceID(sequenceID);
    sequenceID++;
    commonTLV->setUuid0(domainID.uuid0);
    commonTLV->setUuid1(domainID.uuid1);

    auto packet1 = new Packet("interconnLinkChange");
    packet1->insertAtBack(version);
    packet1->insertAtBack(inLinkChangeTLV1);
    packet1->insertAtBack(commonTLV);
    packet1->insertAtBack(endTLV);
    MacAddress sourceAddress1 = getPortNetworkInterface(primaryRingPort)->getMacAddress();
    sendFrameReq(primaryRingPort, MacAddress(MC_INCONTROL), sourceAddress1, priority, MRP_LT, packet1);

    auto packet2 = new Packet("interconnLinkChange");
    packet2->insertAtBack(version);
    packet2->insertAtBack(inLinkChangeTLV2);
    packet2->insertAtBack(commonTLV);
    packet2->insertAtBack(endTLV);
    MacAddress sourceAddress2 = getPortNetworkInterface(secondaryRingPort)->getMacAddress();
    sendFrameReq(secondaryRingPort, MacAddress(MC_INCONTROL), sourceAddress2, priority, MRP_LT, packet2);

    //sending out change notification to Interface which just changed.....
    //in text declaration of standard not requested
    //in function description explicitly requested
    auto packet3 = new Packet("interconnLinkChange");
    packet3->insertAtBack(version);
    packet3->insertAtBack(inLinkChangeTLV3);
    packet3->insertAtBack(commonTLV);
    packet3->insertAtBack(endTLV);
    MacAddress sourceAddress3 = getPortNetworkInterface(interconnectionPort)->getMacAddress();
    if (linkCheckEnabled) {
        sendFrameReq(interconnectionPort, MacAddress(MC_INTRANSFER), sourceAddress3, priority, MRP_LT, packet3);
    } else {
        sendFrameReq(interconnectionPort, MacAddress(MC_INCONTROL), sourceAddress3, priority, MRP_LT, packet3);
    }
    emit(inLinkChangeSignal, simTime().inUnit(SIMTIME_US));
}

void MrpInterconnection::interconnLinkStatusPollReq(double time_ms) {
    if (!inLinkStatusPollTimer->isScheduled() && time_ms > 0) {
        scheduleAt(simTime() + SimTime(time_ms, SIMTIME_MS), inLinkStatusPollTimer);
    } else
        EV_DETAIL << "inLinkStatusPoll already scheduled" << EV_ENDL;
    setupInterconnLinkStatusPollReq();
}

void MrpInterconnection::setupInterconnLinkStatusPollReq() {
    //Create MRP-PDU according MRP_In_LinkStatusPollRequest
    auto version = makeShared<MrpVersion>();
    auto inLinkStatusPollTLV1 = makeShared<MrpInLinkStatusPoll>();
    auto inLinkStatusPollTLV2 = makeShared<MrpInLinkStatusPoll>();
    auto inLinkStatusPollTLV3 = makeShared<MrpInLinkStatusPoll>();
    auto commonTLV = makeShared<MrpCommon>();
    auto endTLV = makeShared<MrpEnd>();

    inLinkStatusPollTLV1->setInID(interConnectionID);
    inLinkStatusPollTLV1->setSa(localBridgeAddress);
    inLinkStatusPollTLV1->setPortRole(MrpInterfaceData::INTERCONNECTION);

    inLinkStatusPollTLV2->setInID(interConnectionID);
    inLinkStatusPollTLV2->setSa(localBridgeAddress);
    inLinkStatusPollTLV2->setPortRole(MrpInterfaceData::PRIMARY);

    inLinkStatusPollTLV3->setInID(interConnectionID);
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
    MacAddress sourceAddress1 = getPortNetworkInterface(interconnectionPort)->getMacAddress();
    sendFrameReq(interconnectionPort, MacAddress(MC_INTRANSFER), sourceAddress1, priority, MRP_LT, packet1);

    auto packet2 = new Packet("InterconnLinkStatusPoll");
    packet2->insertAtBack(version);
    packet2->insertAtBack(inLinkStatusPollTLV2);
    packet2->insertAtBack(commonTLV);
    packet2->insertAtBack(endTLV);
    MacAddress sourceAddress2 = getPortNetworkInterface(primaryRingPort)->getMacAddress();
    sendFrameReq(primaryRingPort, MacAddress(MC_INCONTROL), sourceAddress2, priority, MRP_LT, packet2);

    auto packet3 = new Packet("InterconnLinkStatusPoll");
    packet3->insertAtBack(version);
    packet3->insertAtBack(inLinkStatusPollTLV3);
    packet3->insertAtBack(commonTLV);
    packet3->insertAtBack(endTLV);
    MacAddress sourceAddress3 = getPortNetworkInterface(primaryRingPort)->getMacAddress();
    sendFrameReq(secondaryRingPort, MacAddress(MC_INCONTROL), sourceAddress3, priority, MRP_LT, packet3);
    emit(inStatusPollSignal, simTime().inUnit(SIMTIME_US));
}

void MrpInterconnection::inTransferReq(TlvHeaderType headerType, int ringPort, FrameType frameType, Packet *packet) {
    auto macAddressInd = packet->getTag<MacAddressInd>();
    auto sourceAddress = macAddressInd->getSrcAddress();
    packet->trim();
    packet->clearTags();
    EV_INFO << "Received Frame from Ring. forwarding on InterConnection" << EV_FIELD(packet) << EV_ENDL;
    sendFrameReq(interconnectionPort, MacAddress(frameType), sourceAddress, priority, MRP_LT, packet);
}

void MrpInterconnection::mrpForwardReq(TlvHeaderType headerType, int ringport, FrameType frameType, Packet *packet) {
    auto macAddressInd = packet->getTag<MacAddressInd>();
    auto sourceAddress = macAddressInd->getSrcAddress();
    packet->trim();
    packet->clearTags();
    EV_INFO << "Received Frame from InterConnection. forwarding on Ring" << EV_FIELD(packet) << EV_ENDL;
    sendFrameReq(primaryRingPort, MacAddress(frameType), sourceAddress, priority, MRP_LT, packet->dup());
    sendFrameReq(secondaryRingPort, MacAddress(frameType), sourceAddress, priority, MRP_LT, packet);
}

} // namespace inet

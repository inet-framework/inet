// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "InterconnectionNode.h"

#include "../mediaredundancynode/MediaRedundancyNode.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/linklayer/mrp/common/MrpPdu_m.h"

namespace inet {

Define_Module(InterconnectionNode);

InterconnectionNode::InterconnectionNode()
{
}

InterconnectionNode::~InterconnectionNode()
{
    cancelAndDelete(inLinkStatusPollTimer);
    cancelAndDelete(inLinkTestTimer);
    cancelAndDelete(inTopologyChangeTimer);
    cancelAndDelete(inLinkDownTimer);
    cancelAndDelete(inLinkUpTimer);
    //MediaRedundancyNode::~MediaRedundancyNode();
}

void InterconnectionNode::initialize(int stage)
{

    MediaRedundancyNode::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        expectedRole = CLIENT;
        int inRoleById = par("inRole");
        inRole= static_cast<inRoleState>(inRoleById);
        interConnectionID = par("interConnectionID");
        interconnectionPort = par("InterconnectionPort");
        linkCheckEnabled = par("linkCheckEnabled");
        ringCheckEnabled = par("ringCheckEnabled");

        InLinkChangeSignal = registerSignal("InLinkChangeSignal");
        InTopologyChangeSignal = registerSignal("InTopologyChangeSignal");
        InTestSignal = registerSignal("InTestSignal");
        ReceivedInChangeSignal = registerSignal("ReceivedInChangeSignal");
        ReceivedInTestSignal = registerSignal("ReceivedInTestSignal");
        InterconnectionStateChangedSignal = registerSignal("InterconnectionStateChangedSignal");
        InStatusPollSignal = registerSignal("InStatusPollSignal");
        ReceivedInStatusPollSignal = registerSignal("ReceivedInStatusPollSignal");
    }
    if(stage ==INITSTAGE_LAST){
        EV_DETAIL << "Number of Interfaces:" <<EV_FIELD(interfaceTable->getNumInterfaces()) << EV_ENDL;
        setInterconnectionInterface(interconnectionPort);
        interconnectionPort = interconnectionInterface->getInterfaceId();
        initInterconnectionPort();
    }
}

void InterconnectionNode::initInterconnectionPort()
{
    if (interconnectionInterface == nullptr)
        setInterconnectionInterface(interconnectionPort);
    auto ifd = getPortInterfaceDataForUpdate(interconnectionInterface->getInterfaceId());
    ifd->setRole(MrpInterfaceData::INTERCONNECTION);
    ifd->setState(MrpInterfaceData::BLOCKED);
    ifd->setContinuityCheck(linkCheckEnabled);
    EV_DETAIL << "Initialize InterPort:" <<EV_FIELD(ifd->getContinuityCheck()) <<EV_FIELD(ifd->getRole()) <<EV_FIELD(ifd->getState())<<EV_ENDL;
}

void InterconnectionNode::start()
{
    MediaRedundancyNode::start();
    setTimingProfile(timingProfile);
    intopologyChangeRepeatCount=inTopologyChangeMaxRepeatCount -1;
    inLinkStatusPollCount = inLinkStatusPollMaxCount -1;
    inLinkStatusPollTimer = new cMessage("inLinkStatusPollTimer");
    inTopologyChangeTimer = new cMessage("inTopologyChangeTimer");
    inLinkUpTimer = new cMessage("inLinkUpTimer");
    inLinkDownTimer = new cMessage("inLinkDownTimer");

    //clean start
    //removing everything regarding Interconnection set by MRC/MRM to avoid packets circling mrp ring
    //direct relay (as requested by standard) is not possible
    mrpMacForwardingTable->removeMrpForwardingInterface(primaryRingPort, static_cast<MacAddress>(MC_INTEST), vlanID);
    mrpMacForwardingTable->removeMrpForwardingInterface(secondaryRingPort, static_cast<MacAddress>(MC_INTEST), vlanID);
    mrpMacForwardingTable->removeMrpForwardingInterface(primaryRingPort, static_cast<MacAddress>(MC_INCONTROL),vlanID);
    mrpMacForwardingTable->removeMrpForwardingInterface(secondaryRingPort, static_cast<MacAddress>(MC_INCONTROL),vlanID);

    if (inRole == INTERCONNECTION_MANAGER)
        mimInit();
    else if (inRole == INTERCONNECTION_CLIENT)
        micInit();
    else
        throw cRuntimeError("Unknown Interconnection Role");
}

void InterconnectionNode::stop()
{
    cancelAndDelete(inLinkStatusPollTimer);
    cancelAndDelete(inLinkTestTimer);
    cancelAndDelete(inTopologyChangeTimer);
    cancelAndDelete(inLinkDownTimer);
    cancelAndDelete(inLinkUpTimer);
    MediaRedundancyNode::stop();
}

void InterconnectionNode::read()
{
    //todo
}

void InterconnectionNode::mimInit()
{
    relay->registerAddress(static_cast<MacAddress>(MC_INCONTROL));
    if (linkCheckEnabled){
        relay->registerAddress(static_cast<MacAddress>(MC_INTRANSFER));
        handleInLinkStatusPollTimer();
    }
    if (ringCheckEnabled){
        relay->registerAddress(static_cast<MacAddress>(MC_INTEST));
        inLinkTestTimer = new cMessage("inLinkTestTimer");
    }
    inState = AC_STAT1;
    EV_DETAIL << "Interconnection Manager is started, Switching InState from POWER_ON to AC_STAT1" <<EV_FIELD(inState) <<EV_ENDL;
    mauTypeChangeInd(interconnectionPort, getPortNetworkInterface(interconnectionPort)->getState());
}

void InterconnectionNode::micInit()
{
    relay->registerAddress(static_cast<MacAddress>(MC_INCONTROL));
    relay->registerAddress(static_cast<MacAddress>(MC_INTEST));
    if (ringCheckEnabled){
        mrpMacForwardingTable->addMrpForwardingInterface(interconnectionPort, static_cast<MacAddress>(MC_INTEST), vlanID);
        mrpMacForwardingTable->addMrpForwardingInterface(interconnectionPort, static_cast<MacAddress>(MC_INCONTROL), vlanID);
    }
    if (linkCheckEnabled){
        relay->registerAddress(static_cast<MacAddress>(MC_INTRANSFER));
    }
    setPortState(interconnectionPort, MrpInterfaceData::BLOCKED);
    inState =AC_STAT1;
    EV_DETAIL << "Interconnection Client is started, Switching InState from POWER_ON to AC_STAT1" <<EV_FIELD(inState) <<EV_ENDL;
    mauTypeChangeInd(interconnectionPort, getPortNetworkInterface(interconnectionPort)->getState());
}

void InterconnectionNode::setInterconnectionInterface(int InterfaceIndex)
{
    interconnectionInterface = interfaceTable->getInterface(InterfaceIndex);
    if (interconnectionInterface->isLoopback()){
        interconnectionInterface=nullptr;
    }
    else{
        auto ifd = getPortInterfaceDataForUpdate(interconnectionInterface->getInterfaceId());
        ifd->setRole(MrpInterfaceData::INTERCONNECTION);
        ifd->setState(MrpInterfaceData::BLOCKED);
        ifd->setContinuityCheck(linkCheckEnabled);
    }
}

void InterconnectionNode::setTimingProfile(int maxRecoveryTime)
{
    MediaRedundancyNode::setTimingProfile(maxRecoveryTime);
    switch (maxRecoveryTime){
    case 500:
        inLinkChangeInterval = 20;
        inTopologyChangeInterval = 20;
        inLinkStatusPollInterval = 20;
        inTestDefaultInterval = 50;
        break;
    case 200:
        inLinkChangeInterval = 20;
        inTopologyChangeInterval = 10;
        inLinkStatusPollInterval = 20;
        inTestDefaultInterval = 20;
        break;
    case 30:
        inLinkChangeInterval = 3;
        inTopologyChangeInterval = 1;
        inLinkStatusPollInterval = 3;
        inTestDefaultInterval = 3;
        break;
    case 10:
        inLinkChangeInterval = 3;
        inTopologyChangeInterval = 1;
        inLinkStatusPollInterval = 3;
        inTestDefaultInterval = 3;
        break;
    default:
        throw cRuntimeError("Only RecoveryTimes 500, 200, 30 and 10 ms are defined!");
    }
}

bool InterconnectionNode::hasPassedInterConnRing(uint16_t PortRole,int RingPort)
{
    if (PortRole == MrpInterfaceData::INTERCONNECTION && RingPort != interconnectionPort)
        return true;
    if (PortRole != MrpInterfaceData::INTERCONNECTION && RingPort == interconnectionPort)
            return true;
    return false;
}
void InterconnectionNode::handleStartOperation(LifecycleOperation *operation)
{
}

void InterconnectionNode::handleStopOperation(LifecycleOperation *operation)
{
    stop();
}

void InterconnectionNode::handleCrashOperation(LifecycleOperation *operation)
{
    stop();
}

void InterconnectionNode::handleMessageWhenUp(cMessage *msg)
{
    if (!msg->isSelfMessage()) {
        msg->setKind(2);
        EV_INFO << "Received Message on InterConnectionNode, Rescheduling:" << EV_FIELD(msg) << EV_ENDL;
        scheduleAt(simTime()+SimTime(processingDelay,SIMTIME_US), msg);
    }
    else {
        EV_INFO << "Received Self-Message:" << EV_FIELD(msg) << EV_ENDL;
        if(msg == inLinkStatusPollTimer)
            handleInLinkStatusPollTimer();
        else if(msg == inLinkTestTimer)
            handleInTestTimer();
        else if(msg == inTopologyChangeTimer)
            handleInTopologyChangeTimer();
        else if(msg == inLinkUpTimer)
            handleInLinkUpTimer();
        else if(msg == inLinkDownTimer)
            handleInLinkDownTimer();
        else if(msg == testTimer)
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
            processDelayTimer* timer = dynamic_cast<processDelayTimer *>(msg);
            if (timer != nullptr){
                handleDelayTimer(timer->getPort(), timer->getField());
                delete timer;
            }
        }
        else if (msg->getKind()== 1){
            continuityCheckTimer* timer = dynamic_cast<continuityCheckTimer *>(msg);
            if (timer != nullptr){
                handleContinuityCheckTimer(timer->getPort());
                delete timer;
            }
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

void InterconnectionNode::handleInTestTimer()
{
    switch(inState){
    case CHK_IO:
        interconnTestReq(inTestDefaultInterval);
        break;
    case CHK_IC:
        interconnTestReq(inTestDefaultInterval);
        if (inTestRetransmissionCount >= inTestMaxRetransmissionCount){
            setPortState(interconnectionPort, MrpInterfaceData::FORWARDING);
            inTestMaxRetransmissionCount = inTestMonitoringCount- 1;
            inTestRetransmissionCount = 0;
            interconnTopologyChangeReq(inTopologyChangeInterval);
            inState= CHK_IO;
        }
        else{
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

void InterconnectionNode::handleInLinkStatusPollTimer()
{
    if (inLinkStatusPollCount > 0){
        inLinkStatusPollCount--;
        scheduleAt(simTime()+SimTime(inLinkStatusPollInterval, SIMTIME_MS), inLinkStatusPollTimer);
    }
    else if (inLinkStatusPollCount == 0){
        inLinkStatusPollCount = inLinkStatusPollMaxCount - 1;
    }
    setupInterconnLinkStatusPollReq();
}

void InterconnectionNode::handleInTopologyChangeTimer()
{
 if(intopologyChangeRepeatCount > 0){
     setupInterconnTopologyChangeReq(intopologyChangeRepeatCount*inTopologyChangeInterval);
     intopologyChangeRepeatCount--;
     scheduleAt(simTime()+SimTime(inTopologyChangeInterval, SIMTIME_MS), inTopologyChangeTimer);
 }
 else if(intopologyChangeRepeatCount == 0){
     intopologyChangeRepeatCount = inTopologyChangeMaxRepeatCount - 1;
     clearFDB(0);
     setupInterconnTopologyChangeReq(0);
 }
}

void InterconnectionNode::handleInLinkUpTimer()
{
    inLinkChangeCount--;
    switch(inState){
    case PT:
        if (inLinkChangeCount == 0){
            inLinkChangeCount = inLinkMaxChange;
            setPortState(interconnectionPort, MrpInterfaceData::FORWARDING);
            inState= IP_IDLE;
        }
        else{
            scheduleAt(simTime()+SimTime(inLinkChangeInterval,SIMTIME_MS),inLinkUpTimer);
            interconnLinkChangeReq(NetworkInterface::UP, inLinkChangeCount * inLinkChangeInterval);
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

void InterconnectionNode::handleInLinkDownTimer()
{
    inLinkChangeCount--;
    switch(inState){
    case AC_STAT1:
        if (inRole == INTERCONNECTION_CLIENT){
            if (inLinkChangeCount == 0){
                inLinkChangeCount = inLinkMaxChange;
            }
            else{
                scheduleAt(simTime()+SimTime(inLinkChangeInterval,SIMTIME_MS),inLinkDownTimer);
                interconnLinkChangeReq(NetworkInterface::DOWN, inLinkChangeCount * inLinkChangeInterval);
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

void InterconnectionNode::mauTypeChangeInd(int RingPort, uint16_t LinkState)
{
    if (RingPort == interconnectionPort){
        switch(inState){
        case AC_STAT1:
            if (LinkState == NetworkInterface::UP){
                if (inRole == INTERCONNECTION_MANAGER){
                    setPortState(RingPort,MrpInterfaceData::BLOCKED);
                    if (linkCheckEnabled){
                        interconnLinkStatusPollReq(inLinkStatusPollInterval);
                    }
                    if (ringCheckEnabled){
                        inTestMaxRetransmissionCount = inTestMonitoringCount - 1;
                        inTestRetransmissionCount =0;
                        interconnTestReq(inTestDefaultInterval);
                    }
                    inState = CHK_IC;
                    EV_DETAIL << "Switching InState from AC_STAT1 to CHK_IC" << EV_FIELD(inState) << EV_ENDL;
                    currentInterconnectionState = CLOSED;
                    emit(InterconnectionStateChangedSignal,simTime().inUnit(SIMTIME_US));
                }
                else if(inRole == INTERCONNECTION_CLIENT){
                    inLinkChangeCount = inLinkMaxChange;
                    cancelEvent(inLinkDownTimer);
                    scheduleAt(simTime()+SimTime(inLinkChangeInterval, SIMTIME_MS),inLinkUpTimer);
                    interconnLinkChangeReq(NetworkInterface::UP, inLinkChangeCount * inLinkChangeInterval);
                    inState = PT;
                    EV_DETAIL << "Switching InState from AC_STAT1 to PT" << EV_FIELD(inState) << EV_ENDL;
                }
            }
            break;
        case CHK_IO:
            if (LinkState == NetworkInterface::DOWN){
                setPortState(RingPort,MrpInterfaceData::BLOCKED);
                if (linkCheckEnabled){
                    interconnLinkStatusPollReq(inLinkStatusPollInterval);
                }
                if (ringCheckEnabled){
                    interconnTopologyChangeReq(inTopologyChangeInterval);
                    interconnTestReq(inTestDefaultInterval);
                }
                inState = AC_STAT1;
                EV_DETAIL << "Switching InState from CHK_IO to AC_STAT1" << EV_FIELD(inState) << EV_ENDL;
            }
            break;
        case CHK_IC:
            if (LinkState == NetworkInterface::DOWN){
                setPortState(RingPort,MrpInterfaceData::BLOCKED);
                if (ringCheckEnabled){
                    interconnTopologyChangeReq(inTopologyChangeInterval);
                    interconnTestReq(inTestDefaultInterval);
                }
                inState = AC_STAT1;
                EV_DETAIL << "Switching InState from CHK_IC to AC_STAT1" << EV_FIELD(inState) << EV_ENDL;
            }
            break;
        case PT:
            if (LinkState == NetworkInterface::DOWN){
                inLinkChangeCount = inLinkMaxChange;
                cancelEvent(inLinkUpTimer);
                setPortState(RingPort,MrpInterfaceData::BLOCKED);
                scheduleAt(simTime()+SimTime(inLinkChangeInterval, SIMTIME_MS),inLinkDownTimer);
                interconnLinkChangeReq(NetworkInterface::DOWN, inLinkChangeCount * inLinkChangeInterval);
                inState = AC_STAT1;
                EV_DETAIL << "Switching InState from PT to AC_STAT1" << EV_FIELD(inState) << EV_ENDL;
            }
            break;
        case IP_IDLE:
            if (LinkState == NetworkInterface::DOWN){
                inLinkChangeCount = inLinkMaxChange;
                setPortState(RingPort,MrpInterfaceData::BLOCKED);
                scheduleAt(simTime()+SimTime(inLinkChangeInterval, SIMTIME_MS),inLinkDownTimer);
                interconnLinkChangeReq(NetworkInterface::DOWN, inLinkChangeCount * inLinkChangeInterval);
                inState = AC_STAT1;
                EV_DETAIL << "Switching InState from IP_IDLE to AC_STAT1" << EV_FIELD(inState) << EV_ENDL;
            }
            break;
        case POWER_ON:
            break;
        default:
            throw cRuntimeError("Unknown Node State");
        }
    }
    else{
        MediaRedundancyNode::mauTypeChangeInd(RingPort,LinkState);
    }
}

void InterconnectionNode::interconnTopologyChangeInd(MacAddress SourceAddress, double Time, uint16_t InID, int RingPort, Packet* packet)
{
    if (InID == interConnectionID){
        auto offset = B(2);
        const auto& firstTLV = packet->peekDataAt<inTopologyChangeFrame>(offset);
        offset = offset+firstTLV->getChunkLength();
        const auto& secondTLV = packet->peekDataAt<commonHeader>(offset);
        uint16_t sequence = secondTLV->getSequenceID();
        //Poll is reaching Node from Primary, Secondary and Interconnection Port
        //it is not necessary to react after the first frame
        if (sequence > lastInTopologyId){
            lastInTopologyId = sequence;
        emit(ReceivedInChangeSignal,firstTLV->getInterval());
        switch(inState){
        case AC_STAT1:
            if (inRole == INTERCONNECTION_CLIENT){
                cancelEvent(inLinkDownTimer);
            }else if (inRole == INTERCONNECTION_MANAGER && SourceAddress == sourceAddress){
                clearFDB(Time);
            }
            delete packet;
            break;
        case CHK_IO:
        case CHK_IC:
            if (SourceAddress == sourceAddress){
                clearFDB(Time);
            }
            delete packet;
            break;
        case PT:
            inLinkChangeCount = inLinkMaxChange;
            cancelEvent(inLinkUpTimer);
            setPortState(interconnectionPort,MrpInterfaceData::FORWARDING);
            inState = IP_IDLE;
            EV_DETAIL << "Switching InState from PT to IP_IDLE" << EV_FIELD(inState) << EV_ENDL;
            if (RingPort != interconnectionPort){
                if (linkCheckEnabled)
                    inTransferReq(INTOPOLOGYCHANGE, interconnectionPort, MC_INTRANSFER, packet);
                else
                    inTransferReq(INTOPOLOGYCHANGE, interconnectionPort, MC_INCONTROL, packet);
            }
            else
                mrpForwardReq(INTOPOLOGYCHANGE,RingPort, MC_INCONTROL, packet);
            break;
        case IP_IDLE:
            if (RingPort != interconnectionPort){
                if (linkCheckEnabled)
                    inTransferReq(INTOPOLOGYCHANGE, interconnectionPort, MC_INTRANSFER, packet);
                else
                    inTransferReq(INTOPOLOGYCHANGE, interconnectionPort, MC_INCONTROL, packet);
            }
            else
                mrpForwardReq(INTOPOLOGYCHANGE, RingPort, MC_INCONTROL, packet);
            break;
        case POWER_ON:
            delete packet;
            break;
        default:
            throw cRuntimeError("Unknown Node State");
        }
        }
        else{
            EV_INFO << "Received same Frame already" << EV_ENDL;
            delete packet;
        }
    }
    else{
        EV_INFO << "Received Frame from other InterConnectionID" << EV_FIELD(InID) << EV_ENDL;
        if (RingPort == primaryRingPort)
            interconnForwardReq(secondaryRingPort,packet);
        else if (RingPort == secondaryRingPort)
            interconnForwardReq(primaryRingPort,packet);
        else
            delete packet;
    }
}

void InterconnectionNode::interconnLinkChangeInd(uint16_t InID, uint16_t LinkState,int RingPort,Packet* packet)
{
    if (InID == interConnectionID){
        auto firstTLV = packet->peekDataAt<inLinkChangeFrame>(B(2));
        emit(ReceivedInChangeSignal,firstTLV->getInterval());
        switch(inState){
        case CHK_IO:
            if(linkCheckEnabled){
                if (LinkState == NetworkInterface::UP){
                    setPortState(interconnectionPort,MrpInterfaceData::BLOCKED);
                    cancelEvent(inLinkStatusPollTimer);
                    interconnTopologyChangeReq(inTopologyChangeInterval);
                    EV_INFO << "Interconnection Ring closed" << EV_ENDL;
                    inState = CHK_IC;
                    EV_DETAIL << "Switching InState from CHK_IO to CHK_IC" << EV_FIELD(inState) << EV_ENDL;
                    currentInterconnectionState = CLOSED;
                    emit(InterconnectionStateChangedSignal,simTime().inUnit(SIMTIME_US));
                }
                else if (LinkState == NetworkInterface::DOWN){
                    setPortState(interconnectionPort,MrpInterfaceData::FORWARDING);
                }
            }
            if(ringCheckEnabled){
                if (LinkState == NetworkInterface::UP){
                    interconnTestReq(inTestDefaultInterval);
                }
            }
            delete packet;
            break;
        case CHK_IC:
            if (LinkState == NetworkInterface::DOWN){
                setPortState(interconnectionPort,MrpInterfaceData::FORWARDING);
                interconnTopologyChangeReq(inTopologyChangeInterval);
                if (linkCheckEnabled){
                    cancelEvent(inLinkStatusPollTimer);
                    EV_INFO << "Interconnection Ring open" << EV_ENDL;
                    inState = CHK_IO;
                    EV_DETAIL << "Switching InState from CHK_IC to CHK_IO" << EV_FIELD(inState) << EV_ENDL;
                    currentInterconnectionState = OPEN;
                    emit(InterconnectionStateChangedSignal,simTime().inUnit(SIMTIME_US));
                }
            }
            if (ringCheckEnabled && LinkState == NetworkInterface::UP){
                inTestMaxRetransmissionCount = inTestMonitoringCount - 1;
                interconnTopologyChangeReq(inTopologyChangeInterval);
            }
            delete packet;
            break;
        case PT:
        case IP_IDLE:
            if (RingPort == interconnectionPort){
                if (LinkState == NetworkInterface::UP){
                    mrpForwardReq(INLINKUP,RingPort,MC_INCONTROL, packet);
                }
                else if (LinkState == NetworkInterface::DOWN){
                    mrpForwardReq(INLINKDOWN,RingPort,MC_INCONTROL, packet);
                }
                else
                    delete packet;
            }
            else if (RingPort != interconnectionPort){
                if (ringCheckEnabled){
                    interconnForwardReq(interconnectionPort, packet);
                }
                else if (LinkState == NetworkInterface::UP){
                    inTransferReq(INLINKUP,interconnectionPort,MC_INTRANSFER, packet);
                }
                else if (LinkState == NetworkInterface::DOWN){
                    inTransferReq(INLINKDOWN,interconnectionPort,MC_INTRANSFER, packet);
                }
                else
                    delete packet;
            }
            else
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
    else{
        EV_INFO << "Received Frame from other InterConnectionID" << EV_FIELD(RingPort) << EV_FIELD(InID) << EV_ENDL;
        if (RingPort == primaryRingPort)
            interconnForwardReq(secondaryRingPort,packet);
        else if (RingPort == secondaryRingPort)
            interconnForwardReq(primaryRingPort,packet);
        else
            delete packet;
    }
}

void InterconnectionNode::interconnLinkStatusPollInd(uint16_t InID, int RingPort, Packet* packet)
{
    if (InID == interConnectionID){
        b offset = B(2);
        auto firstTLV = packet->peekDataAt<inLinkStatusPollFrame>(offset);
        offset = offset+firstTLV->getChunkLength();
        auto secondTLV = packet->peekDataAt<commonHeader>(offset);
        uint16_t sequence = secondTLV->getSequenceID();
        //Poll is reaching Node from Primary, Secondary and Interconnection Port
        //it is not necessary to react after the first frame
        if (sequence > lastPollId){
            lastPollId = sequence;
            emit(ReceivedInStatusPollSignal,simTime().inUnit(SIMTIME_US));
            switch(inState){
            case AC_STAT1:
                if (inRole == INTERCONNECTION_CLIENT){
                    interconnLinkChangeReq(NetworkInterface::DOWN, 0);
                }
                delete packet;
                break;
            case PT:
            case IP_IDLE:
                interconnLinkChangeReq(NetworkInterface::UP, 0);
                if (RingPort != interconnectionPort){
                    if (linkCheckEnabled)
                        inTransferReq(INLINKSTATUSPOLL, interconnectionPort, MC_INTRANSFER,packet);
                    else
                        inTransferReq(INLINKSTATUSPOLL, interconnectionPort, MC_INCONTROL,packet);
                }
                else
                    mrpForwardReq(INLINKSTATUSPOLL, RingPort, MC_INCONTROL,packet);
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
        else{
            EV_INFO << "Received same Frame already" << EV_ENDL;
            delete packet;
        }
    }else{
        EV_INFO << "Received Frame from other InterConnectionID" << EV_FIELD(RingPort) << EV_FIELD(InID) << EV_ENDL;
        if (RingPort == primaryRingPort)
            interconnForwardReq(secondaryRingPort,packet);
        else if (RingPort == secondaryRingPort)
            interconnForwardReq(primaryRingPort,packet);
        else
            delete packet;
    }
}

void InterconnectionNode::interconnTestInd(MacAddress SourceAddress, int RingPort, uint16_t InID, Packet* packet)
{
    if (InID == interConnectionID){
        b offset = B(2);
        auto firstTLV = packet->peekDataAt<inTestFrame>(offset);
        offset = offset+firstTLV->getChunkLength();
        auto secondTLV = packet->peekDataAt<commonHeader>(offset);
        uint16_t sequence = secondTLV->getSequenceID();
        //Test is reaching Node from Primary, Secondary and Interconnection Port
        //Test is only valid if packet travel over ring
        //only the first occurrence/direction has to be handled
        if(hasPassedInterConnRing(firstTLV->getPortRole(),RingPort) && sequence > lastInTestId){
            lastInTestId = sequence;
            int ringTime = simTime().inUnit(SIMTIME_MS) - firstTLV->getTimeStamp();
            auto lastInTestFrameSent = inTestFrameSent.find(sequence);
            if (lastInTestFrameSent != inTestFrameSent.end()){
                int64_t ringTimePrecise = simTime().inUnit(SIMTIME_US) - lastInTestFrameSent->second;
                emit(ReceivedInTestSignal,ringTimePrecise);
                EV_DETAIL << "InterconnectionRingTime" <<EV_FIELD(ringTime) <<EV_FIELD(ringTimePrecise) << EV_ENDL;
            }
            else{
                emit(ReceivedInTestSignal,ringTime*1000);
                EV_DETAIL << "InterconnectionRingTime" <<EV_FIELD(ringTime) << EV_ENDL;
            }

            switch(inState){
            case AC_STAT1:
                if (inRole == INTERCONNECTION_MANAGER && SourceAddress == sourceAddress){
                    setPortState(interconnectionPort, MrpInterfaceData::BLOCKED);
                    inTestMaxRetransmissionCount = inTestMonitoringCount -1;
                    inTestRetransmissionCount =0;
                    interconnTestReq(inTestDefaultInterval);
                    inState = CHK_IC;
                    EV_DETAIL << "Switching InState from AC_STAT1 to CHK_IC" << EV_FIELD(inState) << EV_ENDL;
                }
                delete packet;
                break;
            case CHK_IO:
                if (SourceAddress == sourceAddress){
                    setPortState(interconnectionPort, MrpInterfaceData::BLOCKED);
                    interconnTopologyChangeReq(inTopologyChangeInterval);
                    inTestMaxRetransmissionCount = inTestMonitoringCount -1;
                    inTestRetransmissionCount =0;
                    interconnTestReq(inTestDefaultInterval);
                    inState=CHK_IC;
                }
                delete packet;
                break;
            case CHK_IC:
                if (SourceAddress == sourceAddress){
                    inTestMaxRetransmissionCount =inTestMonitoringCount -1;
                    inTestRetransmissionCount =0;
                }
                delete packet;
                break;
            case POWER_ON:
                delete packet;
                break;
            case PT:
            case IP_IDLE:
                if (RingPort == interconnectionPort)
                    mrpForwardReq(INTEST, RingPort, MC_INTEST,packet);
                else
                    //other case are covered by automatic forwarding from ring to interconnection on relay level
                    //setting "addMactoFDB" in client initialisation
                    delete packet;
                break;
            default:
                throw cRuntimeError("Unknown Node State");
            }
        }
        else{
            EV_INFO << "Received InTest again. Already reacted on Frame" << EV_FIELD(secondTLV->getSequenceID()) << EV_FIELD(lastInTestId) << EV_ENDL;
            delete packet;
        }
    }
    else if (RingPort == primaryRingPort){
        interconnForwardReq(secondaryRingPort, packet);
    }
    else if (RingPort == secondaryRingPort){
        interconnForwardReq(primaryRingPort, packet);
    }
}

void InterconnectionNode::interconnTestReq(double Time)
{
    setupInterconnTestReq();
    if (!inLinkTestTimer->isScheduled())
        scheduleAt(simTime()+SimTime(Time,SIMTIME_MS),inLinkTestTimer);
}

void InterconnectionNode::setupInterconnTestReq()
{
    //Create MRP-PDU according MRP_InTest
    auto Version = makeShared<mrpVersionField>();
    auto InTestTLV1 = makeShared<inTestFrame>();
    auto InTestTLV2 = makeShared<inTestFrame>();
    auto InTestTLV3 = makeShared<inTestFrame>();
    auto CommonTLV = makeShared<commonHeader>();
    auto EndTLV = makeShared<tlvHeader>();

    uint32_t timestamp= simTime().inUnit(SIMTIME_MS);
    int64_t lastInTestFrameSent = simTime().inUnit(SIMTIME_US);
    inTestFrameSent.insert({sequenceID, lastInTestFrameSent});

    InTestTLV1->setInID(interConnectionID);
    InTestTLV1->setSa(sourceAddress);
    InTestTLV1->setInState(currentInterconnectionState);
    InTestTLV1->setTransition(transition);
    InTestTLV1->setTimeStamp(timestamp);
    InTestTLV1->setPortRole(MrpInterfaceData::INTERCONNECTION);

    InTestTLV2->setInID(interConnectionID);
    InTestTLV2->setSa(sourceAddress);
    InTestTLV2->setInState(currentInterconnectionState);
    InTestTLV2->setTransition(transition);
    InTestTLV2->setTimeStamp(timestamp);
    InTestTLV2->setPortRole(MrpInterfaceData::PRIMARY);

    InTestTLV3->setInID(interConnectionID);
    InTestTLV3->setSa(sourceAddress);
    InTestTLV3->setInState(currentInterconnectionState);
    InTestTLV3->setTransition(transition);
    InTestTLV3->setTimeStamp(timestamp);
    InTestTLV3->setPortRole(MrpInterfaceData::SECONDARY);

    CommonTLV->setSequenceID(sequenceID);
    sequenceID++;
    CommonTLV->setUuid0(domainID.uuid0);
    CommonTLV->setUuid1(domainID.uuid1);

    auto packet1 = new Packet("InterconnTest");
    packet1->insertAtBack(Version);
    packet1->insertAtBack(InTestTLV1);
    packet1->insertAtBack(CommonTLV);
    packet1->insertAtBack(EndTLV);
    MacAddress SourceAddress1 = getPortNetworkInterface(interconnectionPort)->getMacAddress();
    sendFrameReq(interconnectionPort, static_cast<MacAddress>(MC_INTEST), SourceAddress1, priority, MRP_LT, packet1);

    auto packet2 = new Packet("InterconnTest");
    packet2->insertAtBack(Version);
    packet2->insertAtBack(InTestTLV2);
    packet2->insertAtBack(CommonTLV);
    packet2->insertAtBack(EndTLV);
    MacAddress SourceAddress2 = getPortNetworkInterface(primaryRingPort)->getMacAddress();
    sendFrameReq(primaryRingPort, static_cast<MacAddress>(MC_INTEST), SourceAddress2, priority, MRP_LT, packet2);

    auto packet3 = new Packet("InterconnTest");
    packet3->insertAtBack(Version);
    packet3->insertAtBack(InTestTLV3);
    packet3->insertAtBack(CommonTLV);
    packet3->insertAtBack(EndTLV);
    MacAddress SourceAddress3 = getPortNetworkInterface(primaryRingPort)->getMacAddress();
    sendFrameReq(secondaryRingPort, static_cast<MacAddress>(MC_INTEST), SourceAddress3, priority, MRP_LT, packet3);
    emit(InTestSignal,lastInTestFrameSent);
}

void InterconnectionNode::interconnTopologyChangeReq(double Time)
{
    setupInterconnTopologyChangeReq(inTopologyChangeMaxRepeatCount * Time);
    if (Time == 0){
        clearLocalFDB();
    }
    else if (!inTopologyChangeTimer->isScheduled()){
            scheduleAt(simTime() + SimTime(inTopologyChangeInterval, SIMTIME_MS),inTopologyChangeTimer);
    }
    else
        EV_INFO << "inTopologyChangeTimer already scheduled" << EV_ENDL;
}

void InterconnectionNode::setupInterconnTopologyChangeReq(double Time)
{
    //Create MRP-PDU according MRP_InTopologyChange
    auto Version = makeShared<mrpVersionField>();
    auto InTopologyChangeTLV = makeShared<inTopologyChangeFrame>();
    auto CommonTLV = makeShared<commonHeader>();
    auto EndTLV = makeShared<tlvHeader>();

    InTopologyChangeTLV->setInID(interConnectionID);
    InTopologyChangeTLV->setSa(sourceAddress);
    InTopologyChangeTLV->setInterval(Time);

    CommonTLV->setSequenceID(sequenceID);
    sequenceID++;
    CommonTLV->setUuid0(domainID.uuid0);
    CommonTLV->setUuid1(domainID.uuid1);

    auto packet1 = new Packet("InterconnTopologyChange");
    packet1->insertAtBack(Version);
    packet1->insertAtBack(InTopologyChangeTLV);
    packet1->insertAtBack(CommonTLV);
    packet1->insertAtBack(EndTLV);
    MacAddress SourceAddress1 = getPortNetworkInterface(primaryRingPort)->getMacAddress();
    sendFrameReq(primaryRingPort, static_cast<MacAddress>(MC_INCONTROL), SourceAddress1, priority, MRP_LT, packet1);

    auto packet2 = new Packet("InterconnTopologyChange");
    packet2->insertAtBack(Version);
    packet2->insertAtBack(InTopologyChangeTLV);
    packet2->insertAtBack(CommonTLV);
    packet2->insertAtBack(EndTLV);
    MacAddress SourceAddress2 = getPortNetworkInterface(secondaryRingPort)->getMacAddress();
    sendFrameReq(secondaryRingPort, static_cast<MacAddress>(MC_INCONTROL), SourceAddress2, priority, MRP_LT, packet2);

    auto packet3 = new Packet("InterconnTopologyChange");
    packet3->insertAtBack(Version);
    packet3->insertAtBack(InTopologyChangeTLV);
    packet3->insertAtBack(CommonTLV);
    packet3->insertAtBack(EndTLV);
    MacAddress SourceAddress3 = getPortNetworkInterface(interconnectionPort)->getMacAddress();
    if (linkCheckEnabled){
        sendFrameReq(interconnectionPort, static_cast<MacAddress>(MC_INTRANSFER), SourceAddress3, priority, MRP_LT, packet3);
    }else{
        sendFrameReq(interconnectionPort, static_cast<MacAddress>(MC_INCONTROL), SourceAddress3, priority, MRP_LT, packet3);
    }
    emit(InTopologyChangeSignal,simTime().inUnit(SIMTIME_US));
}

void InterconnectionNode::interconnLinkChangeReq(uint16_t LinkState, double Time)
{
    //Create MRP-PDU according MRP_InLinkUp or MRP_InLinkDown
    auto Version = makeShared<mrpVersionField>();
    auto InLinkChangeTLV1 = makeShared<inLinkChangeFrame>();
    auto InLinkChangeTLV2 = makeShared<inLinkChangeFrame>();
    auto InLinkChangeTLV3 = makeShared<inLinkChangeFrame>();
    auto CommonTLV = makeShared<commonHeader>();
    auto EndTLV = makeShared<tlvHeader>();

    tlvHeaderType type = INLINKDOWN;
    if (LinkState == NetworkInterface::UP){
        type = INLINKUP;
    }
    else if (LinkState == NetworkInterface::DOWN){
        type = INLINKDOWN;
    }

    InLinkChangeTLV1->setHeaderType(type);
    InLinkChangeTLV1->setInID(interConnectionID);
    InLinkChangeTLV1->setSa(sourceAddress);
    InLinkChangeTLV1->setPortRole(MrpInterfaceData::PRIMARY);
    InLinkChangeTLV1->setInterval(Time);
    InLinkChangeTLV1->setLinkInfo(LinkState);

    InLinkChangeTLV2->setHeaderType(type);
    InLinkChangeTLV2->setInID(interConnectionID);
    InLinkChangeTLV2->setSa(sourceAddress);
    InLinkChangeTLV2->setPortRole(MrpInterfaceData::SECONDARY);
    InLinkChangeTLV2->setInterval(Time);
    InLinkChangeTLV2->setLinkInfo(LinkState);

    InLinkChangeTLV3->setHeaderType(type);
    InLinkChangeTLV3->setInID(interConnectionID);
    InLinkChangeTLV3->setSa(sourceAddress);
    InLinkChangeTLV3->setPortRole(MrpInterfaceData::INTERCONNECTION);
    InLinkChangeTLV3->setInterval(Time);
    InLinkChangeTLV3->setLinkInfo(LinkState);

    CommonTLV->setSequenceID(sequenceID);
    sequenceID++;
    CommonTLV->setUuid0(domainID.uuid0);
    CommonTLV->setUuid1(domainID.uuid1);

    auto packet1 = new Packet("interconnLinkChange");
    packet1->insertAtBack(Version);
    packet1->insertAtBack(InLinkChangeTLV1);
    packet1->insertAtBack(CommonTLV);
    packet1->insertAtBack(EndTLV);
    MacAddress SourceAddress1 = getPortNetworkInterface(primaryRingPort)->getMacAddress();
    sendFrameReq(primaryRingPort, static_cast<MacAddress>(MC_INCONTROL), SourceAddress1, priority, MRP_LT, packet1);

    auto packet2 = new Packet("interconnLinkChange");
    packet2->insertAtBack(Version);
    packet2->insertAtBack(InLinkChangeTLV2);
    packet2->insertAtBack(CommonTLV);
    packet2->insertAtBack(EndTLV);
    MacAddress SourceAddress2 = getPortNetworkInterface(secondaryRingPort)->getMacAddress();
    sendFrameReq(secondaryRingPort, static_cast<MacAddress>(MC_INCONTROL), SourceAddress2, priority, MRP_LT, packet2);

    auto packet3 = new Packet("interconnLinkChange");
    packet3->insertAtBack(Version);
    packet3->insertAtBack(InLinkChangeTLV3);
    packet3->insertAtBack(CommonTLV);
    packet3->insertAtBack(EndTLV);
    MacAddress SourceAddress3 = getPortNetworkInterface(interconnectionPort)->getMacAddress();
    if (linkCheckEnabled){
        sendFrameReq(interconnectionPort, static_cast<MacAddress>(MC_INTRANSFER), SourceAddress3, priority, MRP_LT, packet3);
    }else{
        sendFrameReq(interconnectionPort, static_cast<MacAddress>(MC_INCONTROL), SourceAddress3, priority, MRP_LT, packet3);
    }
    emit(InLinkChangeSignal, simTime().inUnit(SIMTIME_US));
}

void InterconnectionNode::interconnLinkStatusPollReq(double Time)
{
    setupInterconnLinkStatusPollReq();
    if (Time>0 && !inLinkStatusPollTimer->isScheduled())
        scheduleAt(simTime() + SimTime(Time,SIMTIME_MS),inLinkStatusPollTimer);
    else
        EV_INFO << "inTopologyChangeTimer already scheduled" << EV_ENDL;
}

void InterconnectionNode::setupInterconnLinkStatusPollReq()
{
    //Create MRP-PDU according MRP_In_LinkStatusPollRequest
    auto Version = makeShared<mrpVersionField>();
    auto InLinkStatusPollTLV1 = makeShared<inLinkStatusPollFrame>();
    auto InLinkStatusPollTLV2 = makeShared<inLinkStatusPollFrame>();
    auto InLinkStatusPollTLV3 = makeShared<inLinkStatusPollFrame>();
    auto CommonTLV = makeShared<commonHeader>();
    auto EndTLV = makeShared<tlvHeader>();

    InLinkStatusPollTLV1->setInID(interConnectionID);
    InLinkStatusPollTLV1->setSa(sourceAddress);
    InLinkStatusPollTLV1->setPortRole(MrpInterfaceData::INTERCONNECTION);

    InLinkStatusPollTLV2->setInID(interConnectionID);
    InLinkStatusPollTLV2->setSa(sourceAddress);
    InLinkStatusPollTLV2->setPortRole(MrpInterfaceData::PRIMARY);

    InLinkStatusPollTLV3->setInID(interConnectionID);
    InLinkStatusPollTLV3->setSa(sourceAddress);
    InLinkStatusPollTLV3->setPortRole(MrpInterfaceData::SECONDARY);

    CommonTLV->setSequenceID(sequenceID);
    sequenceID++;
    CommonTLV->setUuid0(domainID.uuid0);
    CommonTLV->setUuid1(domainID.uuid1);

    auto packet1 = new Packet("InterconnLinkStatusPoll");
    packet1->insertAtBack(Version);
    packet1->insertAtBack(InLinkStatusPollTLV1);
    packet1->insertAtBack(CommonTLV);
    packet1->insertAtBack(EndTLV);
    MacAddress SourceAddress1 = getPortNetworkInterface(interconnectionPort)->getMacAddress();
    sendFrameReq(interconnectionPort, static_cast<MacAddress>(MC_INTRANSFER), SourceAddress1, priority, MRP_LT, packet1);

    auto packet2 = new Packet("InterconnLinkStatusPoll");
    packet2->insertAtBack(Version);
    packet2->insertAtBack(InLinkStatusPollTLV2);
    packet2->insertAtBack(CommonTLV);
    packet2->insertAtBack(EndTLV);
    MacAddress SourceAddress2 = getPortNetworkInterface(primaryRingPort)->getMacAddress();
    sendFrameReq(primaryRingPort, static_cast<MacAddress>(MC_INCONTROL), SourceAddress2, priority, MRP_LT, packet2);

    auto packet3 = new Packet("InterconnLinkStatusPoll");
    packet3->insertAtBack(Version);
    packet3->insertAtBack(InLinkStatusPollTLV3);
    packet3->insertAtBack(CommonTLV);
    packet3->insertAtBack(EndTLV);
    MacAddress SourceAddress3 = getPortNetworkInterface(primaryRingPort)->getMacAddress();
    sendFrameReq(secondaryRingPort, static_cast<MacAddress>(MC_INCONTROL), SourceAddress3, priority, MRP_LT, packet3);
    EV_INFO << "Emiting LinkStatusPoll" << EV_FIELD(simTime().inUnit(SIMTIME_US))<< EV_ENDL;
    emit(InStatusPollSignal, simTime().inUnit(SIMTIME_US));
}

void InterconnectionNode::inTransferReq(tlvHeaderType HeaderType,int RingPort, frameType FrameType, Packet* packet)
{
    auto macAddressInd = packet->getTag<MacAddressInd>();
    auto sourceAddress = macAddressInd->getSrcAddress();
    packet->trim();
    packet->clearTags();
    EV_INFO << "Received Frame from Ring. forwarding on InterConnection" << EV_FIELD(packet)<< EV_ENDL;
    sendFrameReq(interconnectionPort, static_cast<MacAddress>(FrameType), sourceAddress, priority, MRP_LT, packet);
}

void InterconnectionNode::mrpForwardReq(tlvHeaderType HeaderType,int Ringport, frameType FrameType, Packet* packet)
{
    auto macAddressInd = packet->getTag<MacAddressInd>();
    auto sourceAddress = macAddressInd->getSrcAddress();
    packet->trim();
    packet->clearTags();
    EV_INFO << "Received Frame from InterConnection. forwarding on Ring" << EV_FIELD(packet)<< EV_ENDL;
    sendFrameReq(primaryRingPort, static_cast<MacAddress>(FrameType), sourceAddress, priority, MRP_LT, packet->dup());
    sendFrameReq(secondaryRingPort, static_cast<MacAddress>(FrameType), sourceAddress, priority, MRP_LT, packet->dup());
    delete packet;
}

} // namespace inet


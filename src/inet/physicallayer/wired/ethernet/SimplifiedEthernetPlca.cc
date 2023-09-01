//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wired/ethernet/SimplifiedEthernetPlca.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/physicallayer/wired/ethernet/EthernetPhyHeader_m.h"

namespace inet {

namespace physicallayer {

simsignal_t SimplifiedEthernetPlca::curIDSignal = cComponent::registerSignal("curID");
simsignal_t SimplifiedEthernetPlca::stateChangedSignal = cComponent::registerSignal("stateChanged");

Register_Enum(SimplifiedEthernetPlca::State,
        (SimplifiedEthernetPlca::DISABLED,
         SimplifiedEthernetPlca::SENDING_BEACON,
         SimplifiedEthernetPlca::SENDING_COMMIT,
         SimplifiedEthernetPlca::SENDING_DATA,
         SimplifiedEthernetPlca::WAITING_BEACON,
         SimplifiedEthernetPlca::WAITING_COMMIT,
         SimplifiedEthernetPlca::WAITING_DATA,
         SimplifiedEthernetPlca::WAITING_CRS_OFF,
         SimplifiedEthernetPlca::WAITING_TO,
         SimplifiedEthernetPlca::RECEIVING_BEACON,
         SimplifiedEthernetPlca::RECEIVING_COMMIT,
         SimplifiedEthernetPlca::RECEIVING_DATA,
         SimplifiedEthernetPlca::END_TO));

Define_Module(SimplifiedEthernetPlca);

SimplifiedEthernetPlca::~SimplifiedEthernetPlca()
{
    cancelAndDelete(beaconTimer);
    cancelAndDelete(commitTimer);
    cancelAndDelete(dataTimer);
    cancelAndDelete(toTimer);
    cancelAndDelete(burstTimer);
}

void SimplifiedEthernetPlca::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        plca_node_count = par("plca_node_count");
        local_nodeID = par("local_nodeID");
        max_bc = par("max_bc");
        to_interval = par("to_interval");
        burst_interval = par("burst_interval");
        beacon_duration = par("beacon_duration");
        phy = getConnectedModule<IEthernetCsmaPhy>(gate("lowerLayerOut"));
        mac = getConnectedModule<IEthernetCsmaMac>(gate("upperLayerOut"));
        beaconTimer = new cMessage("beaconTimer", END_BEACON_TIMER);
        commitTimer = new cMessage("commitTimer", END_COMMIT_TIMER);
        dataTimer = new cMessage("dataTimer", END_DATA_TIMER);
        toTimer = new cMessage("toTimer", END_TO_TIMER);
        burstTimer = new cMessage("burstTimer", END_BURST_TIMER);
        fsm.setStateChangedSignal(stateChangedSignal);
        fsm.setState(DISABLED, "DISABLED");
        WATCH(packetPending);
        WATCH(phyCOL);
        WATCH(phyCRS);
        WATCH(curID);
        WATCH(bc);
        WATCH(receivedSignalType);
    }
    else if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION) {
        networkInterface = getContainingNicModule(this);
        // TODO register to networkInterface parameter change signal and process changes
        mode = &EthernetModes::getEthernetMode(networkInterface->getDatarate());
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER)
        handleWithFSM(ENABLE_PLCA, nullptr);
}

void SimplifiedEthernetPlca::refreshDisplay() const
{
    auto& displayString = getDisplayString();
    std::stringstream stream;
    stream << curID << "/" << plca_node_count << " (" << local_nodeID << ")" << std::endl << fsm.getStateName();
    displayString.setTagArg("t", 0, stream.str().c_str());
}

void SimplifiedEthernetPlca::handleMessage(cMessage *message)
{
    if (message->isSelfMessage())
        handleSelfMessage(message);
    else
        throw cRuntimeError("Unknown message");
}

void SimplifiedEthernetPlca::handleSelfMessage(cMessage *message)
{
    EV_DEBUG << "Handling self message" << EV_FIELD(message) << EV_ENDL;
    handleWithFSM(message->getKind(), message);
}

void SimplifiedEthernetPlca::handleCarrierSenseStart()
{
    Enter_Method("handleCarrierSenseStart");
    ASSERT(!phyCRS);
    phyCRS = true;
    EV_DEBUG << "Handling carrier sense start" << EV_FIELD(phyCRS) << EV_ENDL;
    if (fsm.getState() == DISABLED)
        mac->handleCarrierSenseStart();
//    handleWithFSM(CARRIER_SENSE_START, nullptr);
}

void SimplifiedEthernetPlca::handleCarrierSenseEnd()
{
    Enter_Method("handleCarrierSenseEnd");
    ASSERT(phyCRS);
    phyCRS = false;
    EV_DEBUG << "Handling carrier sense end" << EV_FIELD(phyCRS) << EV_ENDL;
    if (fsm.getState() == DISABLED)
        mac->handleCarrierSenseEnd();
    handleWithFSM(CARRIER_SENSE_END, nullptr);
}

void SimplifiedEthernetPlca::handleCollisionStart()
{
    Enter_Method("handleCollisionStart");
    ASSERT(!phyCOL);
    phyCOL = true;
    EV_DEBUG << "Handling collision start" << EV_FIELD(phyCOL) << EV_ENDL;
    if (fsm.getState() == DISABLED)
        mac->handleCollisionStart();
    else
        throw cRuntimeError("Illegal operation");
}

void SimplifiedEthernetPlca::handleCollisionEnd()
{
    Enter_Method("handleCollisionEnd");
    ASSERT(phyCOL);
    phyCOL = false;
    EV_DEBUG << "Handling collision end" << EV_FIELD(phyCOL) << EV_ENDL;
    if (fsm.getState() == DISABLED)
        mac->handleCollisionEnd();
    else
        throw cRuntimeError("Illegal operation");
}

void SimplifiedEthernetPlca::handleReceptionStart(EthernetSignalType signalType, Packet *packet)
{
    Enter_Method("handleReceptionStart");
    EV_DEBUG << "Handling reception start" << EV_FIELD(signalType) << EV_FIELD(packet) << EV_ENDL;
    if (fsm.getState() == DISABLED)
        mac->handleReceptionStart(signalType, packet);
    else {
        receivedSignalType = signalType;
        handleWithFSM(RECEPTION_START, packet);
    }
}

void SimplifiedEthernetPlca::handleReceptionError(EthernetSignalType signalType, Packet *packet)
{
    Enter_Method("handleReceptionError");
    EV_DEBUG << "Handling reception error" << EV_FIELD(signalType) << EV_FIELD(packet) << EV_ENDL;
    if (fsm.getState() == DISABLED)
        mac->handleReceptionError(signalType, packet);
    else {
//        receivedSignalType = signalType;
//        handleWithFSM(RECEPTION_ERROR, packet);
    }
}

void SimplifiedEthernetPlca::handleReceptionEnd(EthernetSignalType signalType, Packet *packet, EthernetEsdType esd1)
{
    Enter_Method("handleReceptionEnd");
    EV_DEBUG << "Handling reception end" << EV_FIELD(signalType) << EV_FIELD(packet) << EV_FIELD(esd1) << EV_ENDL;
    if (packet != nullptr)
        take(packet);
    if (fsm.getState() == DISABLED)
        mac->handleReceptionEnd(signalType, packet, esd1);
    else {
        receivedSignalType = signalType;
        receivedEsd1 = esd1;
        handleWithFSM(RECEPTION_END, packet);
    }
}

void SimplifiedEthernetPlca::startSignalTransmission(EthernetSignalType signalType)
{
    Enter_Method("startSignalTransmission");
    EV_DEBUG << "Starting signal transmission" << EV_ENDL;
    if (fsm.getState() == DISABLED)
        phy->startSignalTransmission(signalType);
    else {
        if (signalType != JAM)
            throw cRuntimeError("Invalid operation");
//        handleWithFSM(START_SIGNAL_TRANSMISSION, nullptr);
    }
}

void SimplifiedEthernetPlca::endSignalTransmission(EthernetEsdType esd1)
{
    Enter_Method("endSignalTransmission");
    EV_DEBUG << "Ending signal transmission" << EV_ENDL;
    if (fsm.getState() == DISABLED)
        phy->endSignalTransmission(esd1);
//    else
//        handleWithFSM(END_SIGNAL_TRANSMISSION, nullptr);
}

void SimplifiedEthernetPlca::startFrameTransmission(Packet *packet, EthernetEsdType esd1)
{
    Enter_Method("startFrameTransmission");
    EV_DEBUG << "Starting frame transmission" << EV_FIELD(packet) << EV_ENDL;
    take(packet);
    if (fsm.getState() == DISABLED)
        phy->startFrameTransmission(packet, esd1);
    else
        handleWithFSM(START_FRAME_TRANSMISSION, packet);
}

void SimplifiedEthernetPlca::endFrameTransmission()
{
    Enter_Method("endFrameTransmission");
    EV_DEBUG << "Ending frame transmission" << EV_ENDL;
    if (fsm.getState() == DISABLED)
        phy->endFrameTransmission();
//    else
//        handleWithFSM(END_FRAME_TRANSMISSION, nullptr);
}

void SimplifiedEthernetPlca::handleWithFSM(int event, cMessage *message)
{
    { FSMA_Switch(fsm) {
        FSMA_State(DISABLED) {
            FSMA_Event_Transition(ENABLE_PLCA_AND_CONTROLLER,
                                  event == ENABLE_PLCA && local_nodeID == 0 && !phyCRS,
                                  SENDING_BEACON,
                FSMA_Delay_Action(sendBeacon());
            );
            FSMA_Event_Transition(ENABLE_PLCA_AND_NODE,
                                  event == ENABLE_PLCA && local_nodeID != 0 && !phyCRS,
                                  WAITING_BEACON,
                waitBeacon();
            );
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(SENDING_BEACON) {
            FSMA_Enter(ASSERT(local_nodeID == 0));
            FSMA_Event_Transition(BEACON_END_AND_NO_PACKET_PENDING,
                                  event == END_BEACON_TIMER && !packetPending,
                                  WAITING_TO,
                setCurID(0);
                scheduleAfter(to_interval, toTimer);
                FSMA_Delay_Action(phy->endSignalTransmission(ESDNONE))
            );
            FSMA_Event_Transition(BEACON_END_AND_PACKET_PENDING,
                                  event == END_BEACON_TIMER && packetPending,
                                  SENDING_COMMIT,
                setCurID(0);
                FSMA_Delay_Action(phy->endSignalTransmission(ESDNONE))
            );
            FSMA_Event_Transition(DELAY_FRAME_TRANSMISSION,
                                  event == START_FRAME_TRANSMISSION,
                                  SENDING_BEACON,
                delayTransmission(check_and_cast<Packet *>(message));
            );
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(SENDING_COMMIT) {
            FSMA_Enter(ASSERT(curID == local_nodeID));
            FSMA_Enter(
                scheduleAfter((512 + 32 + 96) / mode->bitrate, commitTimer); // backoff + jam + ifg
                FSMA_Delay_Action(phy->startSignalTransmission(COMMIT))
                if (packetPending)
                    FSMA_Delay_Action(mac->handleCarrierSenseEnd());
            );
            // TODO delme, this cannot happen
            FSMA_Event_Transition(END_COMMIT_TIMER,
                                  event == END_COMMIT_TIMER,
                                  WAITING_CRS_OFF,
                cancelEvent(burstTimer);
                FSMA_Delay_Action(phy->endSignalTransmission(ESD))
            );
            FSMA_Event_Transition(END_BURST_TIMER,
                                  event == END_BURST_TIMER,
                                  WAITING_CRS_OFF,
                cancelEvent(commitTimer);
                FSMA_Delay_Action(phy->endSignalTransmission(ESD))
            );
            FSMA_Event_Transition(START_FRAME_TRANSMISSION,
                                  event == START_FRAME_TRANSMISSION,
                                  SENDING_DATA,
                cancelEvent(commitTimer);
                cancelEvent(burstTimer);
                FSMA_Delay_Action(phy->endSignalTransmission(ESDNONE))
                auto packet = check_and_cast<Packet *>(message);
                FSMA_Delay_Action(phy->startFrameTransmission(packet, bc < max_bc - 1 ? ESDBRS : ESD));
                scheduleAfter(b(packet->getDataLength() + ETHERNET_PHY_HEADER_LEN + ETHERNET_PHY_ESD_LEN).get() / mode->bitrate, dataTimer);
                packetPending = false;
            );
            FSMA_Ignore_Event(event == CARRIER_SENSE_END);
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(SENDING_DATA) {
            FSMA_Enter(ASSERT(curID == local_nodeID));
            FSMA_Event_Transition(END_DATA_TIMER_AND_PACKET_BURST,
                                  event == END_DATA_TIMER && bc < max_bc - 1,
                                  SENDING_COMMIT,
                bc++;
                scheduleAfter(burst_interval, burstTimer);
                FSMA_Delay_Action(phy->endFrameTransmission());
            );
            FSMA_Event_Transition(END_DATA_TIMER_AND_NOT_PACKET_BURST,
                                  event == END_DATA_TIMER && bc == max_bc - 1,
                                  WAITING_CRS_OFF,
                FSMA_Delay_Action(phy->endFrameTransmission());
            );
            FSMA_Ignore_Event(event == CARRIER_SENSE_END);
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(WAITING_BEACON) {
            FSMA_Enter(ASSERT(local_nodeID != 0));
            FSMA_Event_Transition(RECEPTION_START_OF_BEACON,
                                  event == RECEPTION_START && receivedSignalType == BEACON,
                                  RECEIVING_BEACON,
            );
            FSMA_Event_Transition(DELAY_FRAME_TRANSMISSION,
                                  event == START_FRAME_TRANSMISSION,
                                  WAITING_BEACON,
                delayTransmission(check_and_cast<Packet *>(message));
            );
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(WAITING_COMMIT) {
            FSMA_Enter(ASSERT(local_nodeID != curID));
            FSMA_Event_Transition(RECEPTION_START_OF_COMMIT,
                                  event == RECEPTION_START && receivedSignalType == COMMIT,
                                  RECEIVING_COMMIT,
            );
            FSMA_Event_Transition(DELAY_FRAME_TRANSMISSION,
                                  event == START_FRAME_TRANSMISSION,
                                  WAITING_COMMIT,
                delayTransmission(check_and_cast<Packet *>(message));
            );
            FSMA_Ignore_Event(event == CARRIER_SENSE_END);
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(WAITING_DATA) {
            FSMA_Enter(ASSERT(local_nodeID != curID));
            FSMA_Event_Transition(RECEPTION_START_OF_DATA,
                                  event == RECEPTION_START && receivedSignalType == DATA,
                                  RECEIVING_DATA,
            );
            FSMA_Event_Transition(DELAY_FRAME_TRANSMISSION,
                                  event == START_FRAME_TRANSMISSION,
                                  WAITING_DATA,
                delayTransmission(check_and_cast<Packet *>(message));
            );
            FSMA_Ignore_Event(event == CARRIER_SENSE_END);
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(WAITING_CRS_OFF) {
            FSMA_Enter(ASSERT(phyCRS));
            FSMA_Event_Transition(END_TO,
                                  event == CARRIER_SENSE_END,
                                  END_TO,
            );
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(WAITING_TO) {
            FSMA_Event_Transition(NEXT_TO,
                                  event == END_TO_TIMER,
                                  END_TO,
            );
            FSMA_Event_Transition(START_FRAME_TRANSMISSION,
                                  event == START_FRAME_TRANSMISSION && local_nodeID == curID && toTimer->getArrivalTime() - simTime() > 1 / mode->bitrate,
                                  SENDING_DATA,
                cancelEvent(toTimer);
                auto packet = check_and_cast<Packet *>(message);
                FSMA_Delay_Action(phy->startFrameTransmission(packet, bc < max_bc - 1 ? ESDBRS : ESD));
                scheduleAfter(b(packet->getDataLength() + ETHERNET_PHY_HEADER_LEN + ETHERNET_PHY_ESD_LEN).get() / mode->bitrate, dataTimer);
                packetPending = false;
            );
            FSMA_Event_Transition(DELAY_FRAME_TRANSMISSION,
                                  event == START_FRAME_TRANSMISSION && (local_nodeID != curID || local_nodeID == curID && toTimer->getArrivalTime() - simTime() <= 1 / mode->bitrate),
                                  WAITING_TO,
                delayTransmission(check_and_cast<Packet *>(message));
            );
            FSMA_Event_Transition(RECEPTION_START_OF_COMMIT,
                                  event == RECEPTION_START && receivedSignalType == COMMIT,
                                  RECEIVING_COMMIT,
                cancelEvent(toTimer);
            );
            FSMA_Event_Transition(RECEPTION_START_OF_DATA,
                                  event == RECEPTION_START && receivedSignalType == DATA,
                                  RECEIVING_DATA,
                cancelEvent(toTimer);
                FSMA_Delay_Action(mac->handleReceptionStart(DATA, check_and_cast<Packet *>(message)));
            );
            FSMA_Ignore_Event(event == CARRIER_SENSE_END);
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(RECEIVING_BEACON) {
            FSMA_Enter(ASSERT(local_nodeID != 0));
            FSMA_Event_Transition(RECEPTION_END,
                                  event == RECEPTION_END && receivedSignalType == BEACON,
                                  WAITING_TO,
                delete message;
                setCurID(0);
                scheduleAfter(to_interval, toTimer);
            );
            FSMA_Event_Transition(DELAY_FRAME_TRANSMISSION,
                                  event == START_FRAME_TRANSMISSION,
                                  RECEIVING_BEACON,
                delayTransmission(check_and_cast<Packet *>(message));
            );
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(RECEIVING_COMMIT) {
            FSMA_Enter(ASSERT(curID != local_nodeID));
            FSMA_Event_Transition(RECEPTION_END_OF_COMMIT_WITH_ESD,
                                  event == RECEPTION_END && receivedSignalType == COMMIT && receivedEsd1 == ESD,
                                  WAITING_CRS_OFF,
                delete message;
            );
            FSMA_Event_Transition(RECEPTION_END_OF_COMMIT_WITHOUT_ESD,
                                  event == RECEPTION_END && receivedSignalType == COMMIT && receivedEsd1 == ESDNONE,
                                  WAITING_DATA,
                delete message;
            );
            FSMA_Event_Transition(DELAY_FRAME_TRANSMISSION,
                                  event == START_FRAME_TRANSMISSION,
                                  RECEIVING_COMMIT,
                delayTransmission(check_and_cast<Packet *>(message));
            );
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(RECEIVING_DATA) {
            FSMA_Enter(
                ASSERT(curID != local_nodeID);
                if (!packetPending)
                    FSMA_Delay_Action(mac->handleCarrierSenseStart());
            );
            FSMA_Event_Transition(RECEPTION_END_WITH_ESD,
                                  event == RECEPTION_END && receivedSignalType == DATA && receivedEsd1 == ESD,
                                  WAITING_CRS_OFF,
                FSMA_Delay_Action(mac->handleReceptionEnd(DATA, check_and_cast<Packet *>(message), ESDNONE));
                if (!packetPending)
                    FSMA_Delay_Action(mac->handleCarrierSenseEnd());
            );
            FSMA_Event_Transition(RECEPTION_END_WITH_ESDBRS,
                                  event == RECEPTION_END && receivedSignalType == DATA && receivedEsd1 == ESDBRS,
                                  WAITING_COMMIT,
                FSMA_Delay_Action(mac->handleReceptionEnd(DATA, check_and_cast<Packet *>(message), ESDNONE));
                if (!packetPending)
                    FSMA_Delay_Action(mac->handleCarrierSenseEnd());
            );
            FSMA_Fail_On_Unhandled_Event();
        }
        FSMA_State(END_TO) {
            FSMA_No_Event_Transition(COMMIT_TO,
                                     packetPending && local_nodeID == curID + 1,
                                     SENDING_COMMIT,
                setCurID(curID + 1);
            );
            FSMA_No_Event_Transition(NOT_LAST_TO,
                                     curID < plca_node_count - 1,
                                     WAITING_TO,
                setCurID(curID + 1);
                scheduleAfter(to_interval, toTimer));
            FSMA_No_Event_Transition(LAST_TO_AND_CONTROLLER,
                                     curID == plca_node_count - 1 && local_nodeID == 0,
                                     SENDING_BEACON,
                FSMA_Delay_Action(sendBeacon());
            );
            FSMA_No_Event_Transition(LAST_TO_AND_NODE,
                                     curID == plca_node_count - 1 && local_nodeID != 0,
                                     WAITING_BEACON,
                waitBeacon();
            );
            FSMA_Fail_On_Unhandled_Event();
        }
    } }
    fsm.executeDelayedActions();
}

void SimplifiedEthernetPlca::sendBeacon()
{
    bc = 0;
    setCurID(-1);
    scheduleAfter(beacon_duration, beaconTimer);
    phy->startSignalTransmission(BEACON);
}

void SimplifiedEthernetPlca::waitBeacon()
{
    bc = 0;
    setCurID(-1);
}

void SimplifiedEthernetPlca::delayTransmission(Packet *packet)
{
    EV_DEBUG << "Delaying frame transmission" << EV_ENDL;
    packetPending = true;
    mac->handleCarrierSenseStart();
    mac->handleCollisionStart();
    mac->handleCollisionEnd();
    delete packet;
}

void SimplifiedEthernetPlca::setCurID(int value)
{
    EV_DEBUG << "Setting curID to " << value << EV_ENDL;
    curID = value;
    emit(curIDSignal, curID);
}

} // namespace physicallayer

} // namespace inet


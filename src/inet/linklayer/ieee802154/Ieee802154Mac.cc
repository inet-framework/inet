/* -*- mode:c++ -*- ********************************************************
 * file:        Ieee802154Mac.cc
 *
 * author:      Jerome Rousselot, Marcel Steine, Amre El-Hoiydi,
 *              Marc Loebbers, Yosia Hadisusanto, Andreas Koepke
 *
 * copyright:   (C) 2007-2009 CSEM SA
 *              (C) 2009 T.U. Eindhoven
 *                (C) 2004,2005,2006
 *              Telecommunication Networks Group (TKN) at Technische
 *              Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 *
 * Funding: This work was partially financed by the European Commission under the
 * Framework 6 IST Project "Wirelessly Accessible Sensor Populations"
 * (WASP) under contract IST-034963.
 ***************************************************************************
 * part of:    Modifications to the MF-2 framework by CSEM
 **************************************************************************/
#include <cassert>

#include "inet/common/FindModule.h"
#include "inet/common/INETMath.h"
#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ieee802154/Ieee802154Mac.h"
#include "inet/linklayer/ieee802154/Ieee802154MacHeader_m.h"
#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {

using namespace physicallayer;

Define_Module(Ieee802154Mac);

void Ieee802154Mac::initialize(int stage)
{
    MacProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        useMACAcks = par("useMACAcks");
        sifs = par("sifs");
        headerLength = par("headerLength");
        transmissionAttemptInterruptedByRx = false;
        nbTxFrames = 0;
        nbRxFrames = 0;
        nbMissedAcks = 0;
        nbTxAcks = 0;
        nbRecvdAcks = 0;
        nbDroppedFrames = 0;
        nbDuplicates = 0;
        nbBackoffs = 0;
        backoffValues = 0;
        macMaxCSMABackoffs = par("macMaxCSMABackoffs");
        macMaxFrameRetries = par("macMaxFrameRetries");
        macAckWaitDuration = par("macAckWaitDuration");
        aUnitBackoffPeriod = par("aUnitBackoffPeriod");
        ccaDetectionTime = par("ccaDetectionTime");
        rxSetupTime = par("rxSetupTime");
        aTurnaroundTime = par("aTurnaroundTime");
        bitrate = par("bitrate");
        ackLength = par("ackLength");
        ackMessage = nullptr;

        //init parameters for backoff method
        std::string backoffMethodStr = par("backoffMethod").stdstringValue();
        if (backoffMethodStr == "exponential") {
            backoffMethod = EXPONENTIAL;
            macMinBE = par("macMinBE");
            macMaxBE = par("macMaxBE");
        }
        else {
            if (backoffMethodStr == "linear") {
                backoffMethod = LINEAR;
            }
            else if (backoffMethodStr == "constant") {
                backoffMethod = CONSTANT;
            }
            else {
                throw cRuntimeError("Unknown backoff method \"%s\".\
                       Use \"constant\", \"linear\" or \"\
                       \"exponential\".", backoffMethodStr.c_str());
            }
            initialCW = par("contentionWindow");
        }
        NB = 0;

        // initialize the timers
        backoffTimer = new cMessage("timer-backoff");
        ccaTimer = new cMessage("timer-cca");
        sifsTimer = new cMessage("timer-sifs");
        rxAckTimer = new cMessage("timer-rxAck");
        macState = IDLE_1;
        txAttempts = 0;
        txQueue = check_and_cast<queueing::IPacketQueue *>(getSubmodule("queue"));
        radio = getModuleFromPar<IRadio>(par("radioModule"), this);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        cModule *radioModule = check_and_cast<cModule *>(radio);
        //check parameters for consistency
        //aTurnaroundTime should match (be equal or bigger) the RX to TX
        //switching time of the radio
        if (radioModule->hasPar("timeRXToTX")) {
            simtime_t rxToTx = radioModule->par("timeRXToTX");
            if (rxToTx > aTurnaroundTime) {
                throw cRuntimeError("Parameter \"aTurnaroundTime\" (%f) does not match"
                        " the radios RX to TX switching time (%f)! It"
                        " should be equal or bigger",
                        SIMTIME_DBL(aTurnaroundTime), SIMTIME_DBL(rxToTx));
            }
        }
        radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);

        EV_DETAIL << " bitrate = " << bitrate
                  << " backoff method = " << par("backoffMethod").stringValue() << endl;
    }
}

void Ieee802154Mac::finish()
{
    recordScalar("nbTxFrames", nbTxFrames);
    recordScalar("nbRxFrames", nbRxFrames);
    recordScalar("nbDroppedFrames", nbDroppedFrames);
    recordScalar("nbMissedAcks", nbMissedAcks);
    recordScalar("nbRecvdAcks", nbRecvdAcks);
    recordScalar("nbTxAcks", nbTxAcks);
    recordScalar("nbDuplicates", nbDuplicates);
    if (nbBackoffs > 0) {
        recordScalar("meanBackoff", backoffValues / nbBackoffs);
    }
    else {
        recordScalar("meanBackoff", 0);
    }
    recordScalar("nbBackoffs", nbBackoffs);
    recordScalar("backoffDurations", backoffValues);
}

Ieee802154Mac::~Ieee802154Mac()
{
    cancelAndDelete(backoffTimer);
    cancelAndDelete(ccaTimer);
    cancelAndDelete(sifsTimer);
    cancelAndDelete(rxAckTimer);
    if (ackMessage)
        delete ackMessage;
}

void Ieee802154Mac::configureInterfaceEntry()
{
    MacAddress address = parseMacAddressParameter(par("address"));

    // data rate
    interfaceEntry->setDatarate(bitrate);

    // generate a link-layer address to be used as interface token for IPv6
    interfaceEntry->setMacAddress(address);
    interfaceEntry->setInterfaceToken(address.formInterfaceIdentifier());

    // capabilities
    interfaceEntry->setMtu(par("mtu"));
    interfaceEntry->setMulticast(true);
    interfaceEntry->setBroadcast(true);
}

/**
 * Encapsulates the message to be transmitted and pass it on
 * to the FSM main method for further processing.
 */
void Ieee802154Mac::handleUpperPacket(Packet *packet)
{
    //MacPkt*macPkt = encapsMsg(msg);
    auto macPkt = makeShared<Ieee802154MacHeader>();
    assert(headerLength % 8 == 0);
    macPkt->setChunkLength(b(headerLength));
    MacAddress dest = packet->getTag<MacAddressReq>()->getDestAddress();
    EV_DETAIL << "CSMA received a message from upper layer, name is " << packet->getName() << ", CInfo removed, mac addr=" << dest << endl;
    macPkt->setNetworkProtocol(ProtocolGroup::ethertype.getProtocolNumber(packet->getTag<PacketProtocolTag>()->getProtocol()));
    macPkt->setDestAddr(dest);
    delete packet->removeControlInfo();
    macPkt->setSrcAddr(interfaceEntry->getMacAddress());

    if (useMACAcks) {
        if (SeqNrParent.find(dest) == SeqNrParent.end()) {
            //no record of current parent -> add next sequence number to map
            SeqNrParent[dest] = 1;
            macPkt->setSequenceId(0);
            EV_DETAIL << "Adding a new parent to the map of Sequence numbers:" << dest << endl;
        }
        else {
            macPkt->setSequenceId(SeqNrParent[dest]);
            EV_DETAIL << "Packet send with sequence number = " << SeqNrParent[dest] << endl;
            SeqNrParent[dest]++;
        }
    }

    //RadioAccNoise3PhyControlInfo *pco = new RadioAccNoise3PhyControlInfo(bitrate);
    //macPkt->setControlInfo(pco);
    packet->insertAtFront(macPkt);
    packet->getTag<PacketProtocolTag>()->setProtocol(&Protocol::ieee802154);
    EV_DETAIL << "pkt encapsulated, length: " << macPkt->getChunkLength() << "\n";
    executeMac(EV_SEND_REQUEST, packet);
}

void Ieee802154Mac::updateStatusIdle(t_mac_event event, cMessage *msg)
{
    switch (event) {
        case EV_SEND_REQUEST:
            txQueue->pushPacket(static_cast<Packet *>(msg));
            if (!txQueue->isEmpty()) {
                EV_DETAIL << "(1) FSM State IDLE_1, EV_SEND_REQUEST and [TxBuff avail]: startTimerBackOff -> BACKOFF." << endl;
                updateMacState(BACKOFF_2);
                NB = 0;
                //BE = macMinBE;
                startTimer(TIMER_BACKOFF);
            }
            break;

        case EV_DUPLICATE_RECEIVED:
            EV_DETAIL << "(15) FSM State IDLE_1, EV_DUPLICATE_RECEIVED: setting up radio tx -> WAITSIFS." << endl;
            //sendUp(decapsMsg(static_cast<MacSeqPkt *>(msg)));
            delete msg;

            if (useMACAcks) {
                radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                updateMacState(WAITSIFS_6);
                startTimer(TIMER_SIFS);
            }
            break;

        case EV_FRAME_RECEIVED:
            EV_DETAIL << "(15) FSM State IDLE_1, EV_FRAME_RECEIVED: setting up radio tx -> WAITSIFS." << endl;
            decapsulate(check_and_cast<Packet *>(msg));
            sendUp(msg);
            nbRxFrames++;

            if (useMACAcks) {
                radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                updateMacState(WAITSIFS_6);
                startTimer(TIMER_SIFS);
            }
            break;

        case EV_BROADCAST_RECEIVED:
            EV_DETAIL << "(23) FSM State IDLE_1, EV_BROADCAST_RECEIVED: Nothing to do." << endl;
            nbRxFrames++;
            decapsulate(check_and_cast<Packet *>(msg));
            sendUp(msg);
            break;

        default:
            fsmError(event, msg);
            break;
    }
}

void Ieee802154Mac::updateStatusBackoff(t_mac_event event, cMessage *msg)
{
    switch (event) {
        case EV_TIMER_BACKOFF:
            EV_DETAIL << "(2) FSM State BACKOFF, EV_TIMER_BACKOFF:"
                      << " starting CCA timer." << endl;
            startTimer(TIMER_CCA);
            updateMacState(CCA_3);
            radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
            break;

        case EV_DUPLICATE_RECEIVED:
            // suspend current transmission attempt,
            // transmit ack,
            // and resume transmission when entering manageQueue()
            EV_DETAIL << "(28) FSM State BACKOFF, EV_DUPLICATE_RECEIVED:";
            if (useMACAcks) {
                EV_DETAIL << "suspending current transmit tentative and transmitting ack";
                transmissionAttemptInterruptedByRx = true;
                cancelEvent(backoffTimer);
                radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                updateMacState(WAITSIFS_6);
                startTimer(TIMER_SIFS);
            }
            else {
                EV_DETAIL << "Nothing to do.";
            }
            //sendUp(decapsMsg(static_cast<MacSeqPkt *>(msg)));
            delete msg;

            break;

        case EV_FRAME_RECEIVED:
            // suspend current transmission attempt,
            // transmit ack,
            // and resume transmission when entering manageQueue()
            EV_DETAIL << "(28) FSM State BACKOFF, EV_FRAME_RECEIVED:";
            if (useMACAcks) {
                EV_DETAIL << "suspending current transmit tentative and transmitting ack";
                transmissionAttemptInterruptedByRx = true;
                cancelEvent(backoffTimer);

                radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                updateMacState(WAITSIFS_6);
                startTimer(TIMER_SIFS);
            }
            else {
                EV_DETAIL << "sending frame up and resuming normal operation.";
            }
            decapsulate(check_and_cast<Packet *>(msg));
            sendUp(msg);
            break;

        case EV_BROADCAST_RECEIVED:
            EV_DETAIL << "(29) FSM State BACKOFF, EV_BROADCAST_RECEIVED:"
                      << "sending frame up and resuming normal operation." << endl;
            decapsulate(check_and_cast<Packet *>(msg));
            sendUp(msg);
            break;

        default:
            fsmError(event, msg);
            break;
    }
}

void Ieee802154Mac::attachSignal(Packet *mac, simtime_t_cref startTime)
{
    simtime_t duration = mac->getBitLength() / bitrate;
    mac->setDuration(duration);
}

void Ieee802154Mac::updateStatusCCA(t_mac_event event, cMessage *msg)
{
    switch (event) {
        case EV_TIMER_CCA: {
            EV_DETAIL << "(25) FSM State CCA_3, EV_TIMER_CCA" << endl;
            bool isIdle = radio->getReceptionState() == IRadio::RECEPTION_STATE_IDLE;
            if (isIdle) {
                EV_DETAIL << "(3) FSM State CCA_3, EV_TIMER_CCA, [Channel Idle]: -> TRANSMITFRAME_4." << endl;
                updateMacState(TRANSMITFRAME_4);
                radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                if (currentTxFrame == nullptr)
                    popTxQueue();
                Packet *mac = currentTxFrame->dup();
                attachSignal(mac, simTime() + aTurnaroundTime);
                //sendDown(msg);
                // give time for the radio to be in Tx state before transmitting
                sendDelayed(mac, aTurnaroundTime, lowerLayerOutGateId);
                nbTxFrames++;
            }
            else {
                // Channel was busy, increment 802.15.4 backoff timers as specified.
                EV_DETAIL << "(7) FSM State CCA_3, EV_TIMER_CCA, [Channel Busy]: "
                          << " increment counters." << endl;
                NB = NB + 1;
                //BE = std::min(BE+1, macMaxBE);

                // decide if we go for another backoff or if we drop the frame.
                if (NB > macMaxCSMABackoffs) {
                    // drop the frame
                    EV_DETAIL << "Tried " << NB << " backoffs, all reported a busy "
                              << "channel. Dropping the packet." << endl;
                    txAttempts = 0;
                    if (currentTxFrame) {
                        nbDroppedFrames++;
                        PacketDropDetails details;
                        details.setReason(CONGESTION);
                        details.setLimit(macMaxCSMABackoffs);
                        dropCurrentTxFrame(details);
                    }
                    else {
                        EV_ERROR << "too many Backoffs, but currentTxFrame is empty\n";    //TODO is it good, or model error?
                    }
                    manageQueue();
                }
                else {
                    // redo backoff
                    updateMacState(BACKOFF_2);
                    startTimer(TIMER_BACKOFF);
                }
            }
            break;
        }

        case EV_DUPLICATE_RECEIVED:
            EV_DETAIL << "(26) FSM State CCA_3, EV_DUPLICATE_RECEIVED:";
            if (useMACAcks) {
                EV_DETAIL << " setting up radio tx -> WAITSIFS." << endl;
                // suspend current transmission attempt,
                // transmit ack,
                // and resume transmission when entering manageQueue()
                transmissionAttemptInterruptedByRx = true;
                cancelEvent(ccaTimer);

                radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                updateMacState(WAITSIFS_6);
                startTimer(TIMER_SIFS);
            }
            else {
                EV_DETAIL << " Nothing to do." << endl;
            }
            //sendUp(decapsMsg(static_cast<MacPkt*>(msg)));
            delete msg;
            break;

        case EV_FRAME_RECEIVED:
            EV_DETAIL << "(26) FSM State CCA_3, EV_FRAME_RECEIVED:";
            if (useMACAcks) {
                EV_DETAIL << " setting up radio tx -> WAITSIFS." << endl;
                // suspend current transmission attempt,
                // transmit ack,
                // and resume transmission when entering manageQueue()
                transmissionAttemptInterruptedByRx = true;
                cancelEvent(ccaTimer);
                radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                updateMacState(WAITSIFS_6);
                startTimer(TIMER_SIFS);
            }
            else {
                EV_DETAIL << " Nothing to do." << endl;
            }
            decapsulate(check_and_cast<Packet *>(msg));
            sendUp(msg);
            break;

        case EV_BROADCAST_RECEIVED:
            EV_DETAIL << "(24) FSM State BACKOFF, EV_BROADCAST_RECEIVED:"
                      << " Nothing to do." << endl;
            decapsulate(check_and_cast<Packet *>(msg));
            sendUp(msg);
            break;

        default:
            fsmError(event, msg);
            break;
    }
}

void Ieee802154Mac::updateStatusTransmitFrame(t_mac_event event, cMessage *msg)
{
    if (event == EV_FRAME_TRANSMITTED && currentTxFrame != nullptr) {
        //    delete msg;
        Packet *packet = currentTxFrame;
        const auto& csmaHeader = packet->peekAtFront<Ieee802154MacHeader>();
        radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);

        bool expectAck = useMACAcks;
        if (!csmaHeader->getDestAddr().isBroadcast() && !csmaHeader->getDestAddr().isMulticast()) {
            //unicast
            EV_DETAIL << "(4) FSM State TRANSMITFRAME_4, "
                      << "EV_FRAME_TRANSMITTED [Unicast]: ";
        }
        else {
            //broadcast
            EV_DETAIL << "(27) FSM State TRANSMITFRAME_4, EV_FRAME_TRANSMITTED "
                      << " [Broadcast]";
            expectAck = false;
        }

        if (expectAck) {
            EV_DETAIL << "RadioSetupRx -> WAITACK." << endl;
            updateMacState(WAITACK_5);
            startTimer(TIMER_RX_ACK);
        }
        else {
            EV_DETAIL << ": RadioSetupRx, manageQueue..." << endl;
            deleteCurrentTxFrame();
            manageQueue();
        }
        delete msg;
    }
    else {
        fsmError(event, msg);
    }
}

void Ieee802154Mac::updateStatusWaitAck(t_mac_event event, cMessage *msg)
{
    assert(useMACAcks);

    switch (event) {
        case EV_ACK_RECEIVED: {
            EV_DETAIL << "(5) FSM State WAITACK_5, EV_ACK_RECEIVED: "
                      << " ProcessAck, manageQueue..." << endl;
            if (rxAckTimer->isScheduled())
                cancelEvent(rxAckTimer);
            deleteCurrentTxFrame();
            txAttempts = 0;
            delete msg;
            manageQueue();
            break;
        }
        case EV_ACK_TIMEOUT:
            EV_DETAIL << "(12) FSM State WAITACK_5, EV_ACK_TIMEOUT:"
                      << " incrementCounter/dropPacket, manageQueue..." << endl;
            manageMissingAck(event, msg);
            break;

        case EV_BROADCAST_RECEIVED:
        case EV_FRAME_RECEIVED:
            decapsulate(check_and_cast<Packet *>(msg));
            sendUp(msg);
            break;

        case EV_DUPLICATE_RECEIVED:
            EV_DETAIL << "Error ! Received a frame during SIFS !" << endl;
            delete msg;
            break;

        default:
            fsmError(event, msg);
            break;
    }
}

void Ieee802154Mac::manageMissingAck(t_mac_event    /*event*/, cMessage *    /*msg*/)
{
    if (txAttempts < macMaxFrameRetries) {
        // increment counter
        txAttempts++;
        EV_DETAIL << "I will retransmit this packet (I already tried "
                  << txAttempts << " times)." << endl;
    }
    else {
        // drop packet
        EV_DETAIL << "Packet was transmitted " << txAttempts
                  << " times and I never got an Ack. I drop the packet." << endl;
        txAttempts = 0;
        PacketDropDetails details;
        details.setReason(RETRY_LIMIT_REACHED);
        details.setLimit(macMaxFrameRetries);
        dropCurrentTxFrame(details);
    }
    manageQueue();
}

void Ieee802154Mac::updateStatusSIFS(t_mac_event event, cMessage *msg)
{
    assert(useMACAcks);

    switch (event) {
        case EV_TIMER_SIFS:
            EV_DETAIL << "(17) FSM State WAITSIFS_6, EV_TIMER_SIFS:"
                      << " sendAck -> TRANSMITACK." << endl;
            updateMacState(TRANSMITACK_7);
            attachSignal(ackMessage, simTime());
            sendDown(ackMessage);
            nbTxAcks++;
            //        sendDelayed(ackMessage, aTurnaroundTime, lowerLayerOut);
            ackMessage = nullptr;
            break;

        case EV_TIMER_BACKOFF:
            // Backoff timer has expired while receiving a frame. Restart it
            // and stay here.
            EV_DETAIL << "(16) FSM State WAITSIFS_6, EV_TIMER_BACKOFF. "
                      << "Restart backoff timer and don't move." << endl;
            startTimer(TIMER_BACKOFF);
            break;

        case EV_BROADCAST_RECEIVED:
        case EV_FRAME_RECEIVED:
            EV << "Error ! Received a frame during SIFS !" << endl;
            decapsulate(check_and_cast<Packet *>(msg));
            sendUp(msg);
            break;

        default:
            fsmError(event, msg);
            break;
    }
}

void Ieee802154Mac::updateStatusTransmitAck(t_mac_event event, cMessage *msg)
{
    assert(useMACAcks);

    if (event == EV_FRAME_TRANSMITTED) {
        EV_DETAIL << "(19) FSM State TRANSMITACK_7, EV_FRAME_TRANSMITTED:"
                  << " ->manageQueue." << endl;
        radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
        delete msg;
        manageQueue();
    }
    else {
        fsmError(event, msg);
    }
}

void Ieee802154Mac::updateStatusNotIdle(cMessage *msg)
{
    EV_DETAIL << "(20) FSM State NOT IDLE, EV_SEND_REQUEST. Is a TxBuffer available ?" << endl;
    txQueue->pushPacket(static_cast<Packet *>(msg));
}

/**
 * Updates state machine.
 */
void Ieee802154Mac::executeMac(t_mac_event event, cMessage *msg)
{
    EV_DETAIL << "In executeMac" << endl;
    if (macState != IDLE_1 && event == EV_SEND_REQUEST) {
        updateStatusNotIdle(msg);
    }
    else {
        switch (macState) {
            case IDLE_1:
                updateStatusIdle(event, msg);
                break;

            case BACKOFF_2:
                updateStatusBackoff(event, msg);
                break;

            case CCA_3:
                updateStatusCCA(event, msg);
                break;

            case TRANSMITFRAME_4:
                updateStatusTransmitFrame(event, msg);
                break;

            case WAITACK_5:
                updateStatusWaitAck(event, msg);
                break;

            case WAITSIFS_6:
                updateStatusSIFS(event, msg);
                break;

            case TRANSMITACK_7:
                updateStatusTransmitAck(event, msg);
                break;

            default:
                EV << "Error in CSMA FSM: an unknown state has been reached. macState=" << macState << endl;
                break;
        }
    }
}

void Ieee802154Mac::manageQueue()
{
    if (currentTxFrame != nullptr || !txQueue->isEmpty()) {
        EV_DETAIL << "(manageQueue) there are " << txQueue->getNumPackets() + (currentTxFrame == nullptr ? 0 : 1) << " packets to send, entering backoff wait state." << endl;
        if (transmissionAttemptInterruptedByRx) {
            // resume a transmission cycle which was interrupted by
            // a frame reception during CCA check
            transmissionAttemptInterruptedByRx = false;
        }
        else {
            // initialize counters if we start a new transmission
            // cycle from zero
            NB = 0;
            //BE = macMinBE;
        }
        if (!backoffTimer->isScheduled()) {
            startTimer(TIMER_BACKOFF);
        }
        updateMacState(BACKOFF_2);
    }
    else {
        EV_DETAIL << "(manageQueue) no packets to send, entering IDLE state." << endl;
        updateMacState(IDLE_1);
    }
}

void Ieee802154Mac::updateMacState(t_mac_states newMacState)
{
    macState = newMacState;
}

/*
 * Called by the FSM machine when an unknown transition is requested.
 */
void Ieee802154Mac::fsmError(t_mac_event event, cMessage *msg)
{
    EV << "FSM Error ! In state " << macState << ", received unknown event:" << event << "." << endl;
    if (msg != nullptr)
        delete msg;
}

void Ieee802154Mac::startTimer(t_mac_timer timer)
{
    if (timer == TIMER_BACKOFF) {
        scheduleAt(scheduleBackoff(), backoffTimer);
    }
    else if (timer == TIMER_CCA) {
        simtime_t ccaTime = rxSetupTime + ccaDetectionTime;
        EV_DETAIL << "(startTimer) ccaTimer value=" << ccaTime
                  << "(rxSetupTime,ccaDetectionTime:" << rxSetupTime
                  << "," << ccaDetectionTime << ")." << endl;
        scheduleAfter(rxSetupTime + ccaDetectionTime, ccaTimer);
    }
    else if (timer == TIMER_SIFS) {
        assert(useMACAcks);
        EV_DETAIL << "(startTimer) sifsTimer value=" << sifs << endl;
        scheduleAfter(sifs, sifsTimer);
    }
    else if (timer == TIMER_RX_ACK) {
        assert(useMACAcks);
        EV_DETAIL << "(startTimer) rxAckTimer value=" << macAckWaitDuration << endl;
        scheduleAfter(macAckWaitDuration, rxAckTimer);
    }
    else {
        EV << "Unknown timer requested to start:" << timer << endl;
    }
}

simtime_t Ieee802154Mac::scheduleBackoff()
{
    simtime_t backoffTime;

    switch (backoffMethod) {
        case EXPONENTIAL: {
            int BE = std::min(macMinBE + NB, macMaxBE);
            int v = (1 << BE) - 1;
            int r = intuniform(0, v, 0);
            backoffTime = r * aUnitBackoffPeriod;

            EV_DETAIL << "(startTimer) backoffTimer value=" << backoffTime
                      << " (BE=" << BE << ", 2^BE-1= " << v << "r="
                      << r << ")" << endl;
            break;
        }

        case LINEAR: {
            int slots = intuniform(1, initialCW + NB, 0);
            backoffTime = slots * aUnitBackoffPeriod;
            EV_DETAIL << "(startTimer) backoffTimer value=" << backoffTime << endl;
            break;
        }

        case CONSTANT: {
            int slots = intuniform(1, initialCW, 0);
            backoffTime = slots * aUnitBackoffPeriod;
            EV_DETAIL << "(startTimer) backoffTimer value=" << backoffTime << endl;
            break;
        }

        default:
            throw cRuntimeError("Unknown backoff method!");
            break;
    }

    nbBackoffs = nbBackoffs + 1;
    backoffValues = backoffValues + SIMTIME_DBL(backoffTime);

    return backoffTime + simTime();
}

/*
 * Binds timers to events and executes FSM.
 */
void Ieee802154Mac::handleSelfMessage(cMessage *msg)
{
    EV_DETAIL << "timer routine." << endl;
    if (msg == backoffTimer)
        executeMac(EV_TIMER_BACKOFF, msg);
    else if (msg == ccaTimer)
        executeMac(EV_TIMER_CCA, msg);
    else if (msg == sifsTimer)
        executeMac(EV_TIMER_SIFS, msg);
    else if (msg == rxAckTimer) {
        nbMissedAcks++;
        executeMac(EV_ACK_TIMEOUT, msg);
    }
    else
        EV << "CSMA Error: unknown timer fired:" << msg << endl;
}

/**
 * Compares the address of this Host with the destination address in
 * frame. Generates the corresponding event.
 */
void Ieee802154Mac::handleLowerPacket(Packet *packet)
{
    if (packet->hasBitError()) {
        EV << "Received " << packet << " contains bit errors or collision, dropping it\n";
        PacketDropDetails details;
        details.setReason(INCORRECTLY_RECEIVED);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
        return;
    }
    const auto& csmaHeader = packet->peekAtFront<Ieee802154MacHeader>();
    const MacAddress& src = csmaHeader->getSrcAddr();
    const MacAddress& dest = csmaHeader->getDestAddr();
    long ExpectedNr = 0;
    MacAddress address = interfaceEntry->getMacAddress();

    EV_DETAIL << "Received frame name= " << csmaHeader->getName()
              << ", myState=" << macState << " src=" << src
              << " dst=" << dest << " myAddr="
              << address << endl;

    if (dest == address) {
        if (!useMACAcks) {
            EV_DETAIL << "Received a data packet addressed to me." << endl;
//            nbRxFrames++;
            executeMac(EV_FRAME_RECEIVED, packet);
        }
        else {
            long SeqNr = csmaHeader->getSequenceId();

            if (strcmp(packet->getName(), "CSMA-Ack") != 0) {
                // This is a data message addressed to us
                // and we should send an ack.
                // we build the ack packet here because we need to
                // copy data from macPkt (src).
                EV_DETAIL << "Received a data packet addressed to me,"
                          << " preparing an ack..." << endl;

//                nbRxFrames++;

                if (ackMessage != nullptr)
                    delete ackMessage;
                auto csmaHeader = makeShared<Ieee802154MacHeader>();
                csmaHeader->setSrcAddr(address);
                csmaHeader->setDestAddr(src);
                csmaHeader->setChunkLength(b(ackLength));
                ackMessage = new Packet("CSMA-Ack");
                ackMessage->insertAtFront(csmaHeader);
                ackMessage->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ieee802154);
                //Check for duplicates by checking expected seqNr of sender
                if (SeqNrChild.find(src) == SeqNrChild.end()) {
                    //no record of current child -> add expected next number to map
                    SeqNrChild[src] = SeqNr + 1;
                    EV_DETAIL << "Adding a new child to the map of Sequence numbers:" << src << endl;
                    executeMac(EV_FRAME_RECEIVED, packet);
                }
                else {
                    ExpectedNr = SeqNrChild[src];
                    EV_DETAIL << "Expected Sequence number is " << ExpectedNr
                              << " and number of packet is " << SeqNr << endl;
                    if (SeqNr < ExpectedNr) {
                        //Duplicate Packet, count and do not send to upper layer
                        nbDuplicates++;
                        executeMac(EV_DUPLICATE_RECEIVED, packet);
                    }
                    else {
                        SeqNrChild[src] = SeqNr + 1;
                        executeMac(EV_FRAME_RECEIVED, packet);
                    }
                }
            }
            else if (currentTxFrame != nullptr) {
                // message is an ack, and it is for us.
                // Is it from the right node ?
                const auto& csmaHeader = currentTxFrame->peekAtFront<Ieee802154MacHeader>();
                if (src == csmaHeader->getDestAddr()) {
                    nbRecvdAcks++;
                    executeMac(EV_ACK_RECEIVED, packet);
                }
                else {
                    EV << "Error! Received an ack from an unexpected source: src=" << src << ", I was expecting from node addr=" << csmaHeader->getDestAddr() << endl;
                    delete packet;
                }
            }
            else {
                EV << "Error! Received an Ack while my send queue was empty. src=" << src << "." << endl;
                delete packet;
            }
        }
    }
    else if (dest.isBroadcast() || dest.isMulticast()) {
        executeMac(EV_BROADCAST_RECEIVED, packet);
    }
    else {
        EV_DETAIL << "packet not for me, deleting...\n";
        PacketDropDetails details;
        details.setReason(NOT_ADDRESSED_TO_US);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
    }
}

void Ieee802154Mac::receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details)
{
    Enter_Method_Silent();
    if (signalID == IRadio::transmissionStateChangedSignal) {
        IRadio::TransmissionState newRadioTransmissionState = static_cast<IRadio::TransmissionState>(value);
        if (transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING && newRadioTransmissionState == IRadio::TRANSMISSION_STATE_IDLE) {
            // KLUDGE: we used to get a cMessage from the radio (the identity was not important)
            executeMac(EV_FRAME_TRANSMITTED, new cMessage("Transmission over"));
        }
        transmissionState = newRadioTransmissionState;
    }
}

void Ieee802154Mac::decapsulate(Packet *packet)
{
    const auto& csmaHeader = packet->popAtFront<Ieee802154MacHeader>();
    packet->addTagIfAbsent<MacAddressInd>()->setSrcAddress(csmaHeader->getSrcAddr());
    packet->addTagIfAbsent<InterfaceInd>()->setInterfaceId(interfaceEntry->getInterfaceId());
    auto payloadProtocol = ProtocolGroup::ethertype.getProtocol(csmaHeader->getNetworkProtocol());
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(payloadProtocol);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(payloadProtocol);
}

void Ieee802154Mac::handleStartOperation(LifecycleOperation *operation)
{
    updateMacState(IDLE_1);
    //manageQueue() to see waiting packets or set to idle if none
    MacProtocolBase::handleStartOperation(operation);

    cModule *radioModule = check_and_cast<cModule *>(radio);
    radioModule->subscribe(IRadio::radioModeChangedSignal, this);
    radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);
}

void Ieee802154Mac::handleStopOperation(LifecycleOperation *operation)
{
    //TODO: More gracefully allow current Tx to end (delay operation)
    //Cancel all self message timers
    cancelEvent(backoffTimer);
    cancelEvent(ccaTimer);
    cancelEvent(sifsTimer);
    cancelEvent(rxAckTimer);

    cModule *radioModule = check_and_cast<cModule *>(radio);
    radioModule->unsubscribe(IRadio::radioModeChangedSignal, this);
    radioModule->unsubscribe(IRadio::transmissionStateChangedSignal, this);

    MacProtocolBase::handleStopOperation(operation);
}

void Ieee802154Mac::handleCrashOperation(LifecycleOperation *operation)
{
    cancelEvent(backoffTimer);
    cancelEvent(ccaTimer);
    cancelEvent(sifsTimer);
    cancelEvent(rxAckTimer);

    cModule *radioModule = check_and_cast<cModule *>(radio);
    radioModule->unsubscribe(IRadio::radioModeChangedSignal, this);
    radioModule->unsubscribe(IRadio::transmissionStateChangedSignal, this);

    MacProtocolBase::handleCrashOperation(operation);
}

} // namespace inet


/* -*- mode:c++ -*- ********************************************************
 * file:        CSMA.cc
 *
 * author:      Jerome Rousselot, Marcel Steine, Amre El-Hoiydi,
 *              Marc Loebbers, Yosia Hadisusanto, Andreas Koepke
 *
 * copyright:   (C) 2007-2009 CSEM SA
 *              (C) 2009 T.U. Eindhoven
 *				(C) 2004,2005,2006
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
#include "inet/linklayer/csma/CSMA.h"

#include <cassert>

#include "inet/common/INETUtils.h"
#include "inet/common/INETMath.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/linklayer/contract/IMACProtocolControlInfo.h"
#include "inet/common/FindModule.h"
#include "inet/linklayer/common/SimpleLinkLayerControlInfo.h"
#include "inet/linklayer/csma/CSMAFrame_m.h"

namespace inet {

Define_Module(CSMA);

void CSMA::initialize(int stage)
{
    MACProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        useMACAcks = par("useMACAcks").boolValue();
        queueLength = par("queueLength");
        sifs = par("sifs");
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
        macAckWaitDuration = par("macAckWaitDuration").doubleValue();
        aUnitBackoffPeriod = par("aUnitBackoffPeriod").doubleValue();
        ccaDetectionTime = par("ccaDetectionTime").doubleValue();
        rxSetupTime = par("rxSetupTime").doubleValue();
        aTurnaroundTime = par("aTurnaroundTime").doubleValue();
        bitrate = par("bitrate");
        ackLength = par("ackLength");
        ackMessage = NULL;

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
                error("Unknown backoff method \"%s\".\
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

        initializeMACAddress();
        registerInterface();

        cModule *radioModule = getParentModule()->getSubmodule("radio");
        radioModule->subscribe(IRadio::radioModeChangedSignal, this);
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);
        radio = check_and_cast<IRadio *>(radioModule);

        //check parameters for consistency
        //aTurnaroundTime should match (be equal or bigger) the RX to TX
        //switching time of the radio
        if (radioModule->hasPar("timeRXToTX")) {
            simtime_t rxToTx = radioModule->par("timeRXToTX").doubleValue();
            if (rxToTx > aTurnaroundTime) {
                opp_warning("Parameter \"aTurnaroundTime\" (%f) does not match"
                            " the radios RX to TX switching time (%f)! It"
                            " should be equal or bigger",
                        SIMTIME_DBL(aTurnaroundTime), SIMTIME_DBL(rxToTx));
            }
        }
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
        EV_DETAIL << "queueLength = " << queueLength
                  << " bitrate = " << bitrate
                  << " backoff method = " << par("backoffMethod").stringValue() << endl;

        EV_DETAIL << "finished csma init stage 1." << endl;
    }
}

void CSMA::finish()
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

CSMA::~CSMA()
{
    cancelAndDelete(backoffTimer);
    cancelAndDelete(ccaTimer);
    cancelAndDelete(sifsTimer);
    cancelAndDelete(rxAckTimer);
    if (ackMessage)
        delete ackMessage;
    MacQueue::iterator it;
    for (it = macQueue.begin(); it != macQueue.end(); ++it) {
        delete (*it);
    }
}

void CSMA::initializeMACAddress()
{
    const char *addrstr = par("address");

    if (!strcmp(addrstr, "auto")) {
        // assign automatic address
        address = MACAddress::generateAutoAddress();

        // change module parameter from "auto" to concrete address
        par("address").setStringValue(address.str().c_str());
    }
    else {
        address.setAddress(addrstr);
    }
}

InterfaceEntry *CSMA::createInterfaceEntry()
{
    InterfaceEntry *e = new InterfaceEntry(this);

    // interface name: NIC module's name without special characters ([])
    e->setName(utils::stripnonalnum(getParentModule()->getFullName()).c_str());

    // data rate
    e->setDatarate(bitrate);

    // generate a link-layer address to be used as interface token for IPv6
    e->setMACAddress(address);
    e->setInterfaceToken(address.formInterfaceIdentifier());

    // capabilities
    e->setMtu(par("mtu").longValue());
    e->setMulticast(false);
    e->setBroadcast(true);

    return e;
}

/**
 * Encapsulates the message to be transmitted and pass it on
 * to the FSM main method for further processing.
 */
void CSMA::handleUpperPacket(cPacket *msg)
{
    //MacPkt*macPkt = encapsMsg(msg);
    CSMAFrame *macPkt = new CSMAFrame(msg->getName());
    macPkt->setBitLength(headerLength);
    IMACProtocolControlInfo *const cInfo = check_and_cast<IMACProtocolControlInfo *>(msg->removeControlInfo());
    EV_DETAIL << "CSMA received a message from upper layer, name is " << msg->getName() << ", CInfo removed, mac addr=" << cInfo->getDestinationAddress() << endl;
    MACAddress dest = cInfo->getDestinationAddress();
    macPkt->setDestAddr(dest);
    delete cInfo;
    macPkt->setSrcAddr(address);

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
    assert(static_cast<cPacket *>(msg));
    macPkt->encapsulate(static_cast<cPacket *>(msg));
    EV_DETAIL << "pkt encapsulated, length: " << macPkt->getBitLength() << "\n";
    executeMac(EV_SEND_REQUEST, macPkt);
}

void CSMA::updateStatusIdle(t_mac_event event, cMessage *msg)
{
    switch (event) {
        case EV_SEND_REQUEST:
            if (macQueue.size() <= queueLength) {
                macQueue.push_back(static_cast<CSMAFrame *>(msg));
                EV_DETAIL << "(1) FSM State IDLE_1, EV_SEND_REQUEST and [TxBuff avail]: startTimerBackOff -> BACKOFF." << endl;
                updateMacState(BACKOFF_2);
                NB = 0;
                //BE = macMinBE;
                startTimer(TIMER_BACKOFF);
            }
            else {
                // queue is full, message has to be deleted
                EV_DETAIL << "(12) FSM State IDLE_1, EV_SEND_REQUEST and [TxBuff not avail]: dropping packet -> IDLE." << endl;
                emit(packetFromUpperDroppedSignal, msg);
                delete msg;
                updateMacState(IDLE_1);
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
            sendUp(decapsMsg(static_cast<CSMAFrame *>(msg)));
            nbRxFrames++;
            delete msg;

            if (useMACAcks) {
                radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                updateMacState(WAITSIFS_6);
                startTimer(TIMER_SIFS);
            }
            break;

        case EV_BROADCAST_RECEIVED:
            EV_DETAIL << "(23) FSM State IDLE_1, EV_BROADCAST_RECEIVED: Nothing to do." << endl;
            nbRxFrames++;
            sendUp(decapsMsg(static_cast<CSMAFrame *>(msg)));
            delete msg;
            break;

        default:
            fsmError(event, msg);
            break;
    }
}

void CSMA::updateStatusBackoff(t_mac_event event, cMessage *msg)
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
            sendUp(decapsMsg(static_cast<CSMAFrame *>(msg)));
            delete msg;
            break;

        case EV_BROADCAST_RECEIVED:
            EV_DETAIL << "(29) FSM State BACKOFF, EV_BROADCAST_RECEIVED:"
                      << "sending frame up and resuming normal operation." << endl;
            sendUp(decapsMsg(static_cast<CSMAFrame *>(msg)));
            delete msg;
            break;

        default:
            fsmError(event, msg);
            break;
    }
}

void CSMA::flushQueue()
{
    // TODO:
    macQueue.clear();
}

void CSMA::clearQueue()
{
    macQueue.clear();
}

void CSMA::attachSignal(CSMAFrame *mac, simtime_t_cref startTime)
{
    simtime_t duration = mac->getBitLength() / bitrate;
    mac->setDuration(duration);
}

void CSMA::updateStatusCCA(t_mac_event event, cMessage *msg)
{
    switch (event) {
        case EV_TIMER_CCA: {
            EV_DETAIL << "(25) FSM State CCA_3, EV_TIMER_CCA" << endl;
            bool isIdle = radio->getReceptionState() == IRadio::RECEPTION_STATE_IDLE;
            if (isIdle) {
                EV_DETAIL << "(3) FSM State CCA_3, EV_TIMER_CCA, [Channel Idle]: -> TRANSMITFRAME_4." << endl;
                updateMacState(TRANSMITFRAME_4);
                radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                CSMAFrame *mac = check_and_cast<CSMAFrame *>(macQueue.front()->dup());
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
                    cMessage *mac = macQueue.front();
                    macQueue.pop_front();
                    txAttempts = 0;
                    nbDroppedFrames++;
                    emit(packetFromUpperDroppedSignal, mac);
                    delete mac;
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
            sendUp(decapsMsg(static_cast<CSMAFrame *>(msg)));
            delete msg;
            break;

        case EV_BROADCAST_RECEIVED:
            EV_DETAIL << "(24) FSM State BACKOFF, EV_BROADCAST_RECEIVED:"
                      << " Nothing to do." << endl;
            sendUp(decapsMsg(static_cast<CSMAFrame *>(msg)));
            delete msg;
            break;

        default:
            fsmError(event, msg);
            break;
    }
}

void CSMA::updateStatusTransmitFrame(t_mac_event event, cMessage *msg)
{
    if (event == EV_FRAME_TRANSMITTED) {
        //    delete msg;
        CSMAFrame *packet = macQueue.front();
        radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);

        bool expectAck = useMACAcks;
        if (!packet->getDestAddr().isBroadcast()) {
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
            macQueue.pop_front();
            delete packet;
            manageQueue();
        }
        delete msg;
    }
    else {
        fsmError(event, msg);
    }
}

void CSMA::updateStatusWaitAck(t_mac_event event, cMessage *msg)
{
    assert(useMACAcks);

    switch (event) {
        case EV_ACK_RECEIVED:
            EV_DETAIL << "(5) FSM State WAITACK_5, EV_ACK_RECEIVED: "
                      << " ProcessAck, manageQueue..." << endl;
            if (rxAckTimer->isScheduled())
                cancelEvent(rxAckTimer);
            macQueue.pop_front();
            txAttempts = 0;
            delete msg;
            manageQueue();
            break;

        case EV_ACK_TIMEOUT:
            EV_DETAIL << "(12) FSM State WAITACK_5, EV_ACK_TIMEOUT:"
                      << " incrementCounter/dropPacket, manageQueue..." << endl;
            manageMissingAck(event, msg);
            break;

        case EV_BROADCAST_RECEIVED:
        case EV_FRAME_RECEIVED:
            sendUp(decapsMsg(static_cast<CSMAFrame *>(msg)));
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

void CSMA::manageMissingAck(t_mac_event    /*event*/, cMessage *    /*msg*/)
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
        cMessage *mac = macQueue.front();
        macQueue.pop_front();
        txAttempts = 0;
        // TODO: send dropped signal
        // emit(packetDropped, mac);
        delete mac;
    }
    manageQueue();
}

void CSMA::updateStatusSIFS(t_mac_event event, cMessage *msg)
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
            //		sendDelayed(ackMessage, aTurnaroundTime, lowerLayerOut);
            ackMessage = NULL;
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
            sendUp(decapsMsg(static_cast<CSMAFrame *>(msg)));
            delete msg;
            break;

        default:
            fsmError(event, msg);
            break;
    }
}

void CSMA::updateStatusTransmitAck(t_mac_event event, cMessage *msg)
{
    assert(useMACAcks);

    if (event == EV_FRAME_TRANSMITTED) {
        EV_DETAIL << "(19) FSM State TRANSMITACK_7, EV_FRAME_TRANSMITTED:"
                  << " ->manageQueue." << endl;
        radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
        //		delete msg;
        manageQueue();
    }
    else {
        fsmError(event, msg);
    }
}

void CSMA::updateStatusNotIdle(cMessage *msg)
{
    EV_DETAIL << "(20) FSM State NOT IDLE, EV_SEND_REQUEST. Is a TxBuffer available ?" << endl;
    if (macQueue.size() <= queueLength) {
        macQueue.push_back(static_cast<CSMAFrame *>(msg));
        EV_DETAIL << "(21) FSM State NOT IDLE, EV_SEND_REQUEST"
                  << " and [TxBuff avail]: enqueue packet and don't move." << endl;
    }
    else {
        // queue is full, message has to be deleted
        EV_DETAIL << "(22) FSM State NOT IDLE, EV_SEND_REQUEST"
                  << " and [TxBuff not avail]: dropping packet and don't move."
                  << endl;
        // TODO: send dropped signal
        // emit(packetDropped, msg);
        delete msg;
    }
}

/**
 * Updates state machine.
 */
void CSMA::executeMac(t_mac_event event, cMessage *msg)
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

void CSMA::manageQueue()
{
    if (macQueue.size() != 0) {
        EV_DETAIL << "(manageQueue) there are " << macQueue.size() << " packets to send, entering backoff wait state." << endl;
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

void CSMA::updateMacState(t_mac_states newMacState)
{
    macState = newMacState;
}

/*
 * Called by the FSM machine when an unknown transition is requested.
 */
void CSMA::fsmError(t_mac_event event, cMessage *msg)
{
    EV << "FSM Error ! In state " << macState << ", received unknown event:" << event << "." << endl;
    if (msg != NULL)
        delete msg;
}

void CSMA::startTimer(t_mac_timer timer)
{
    if (timer == TIMER_BACKOFF) {
        scheduleAt(scheduleBackoff(), backoffTimer);
    }
    else if (timer == TIMER_CCA) {
        simtime_t ccaTime = rxSetupTime + ccaDetectionTime;
        EV_DETAIL << "(startTimer) ccaTimer value=" << ccaTime
                  << "(rxSetupTime,ccaDetectionTime:" << rxSetupTime
                  << "," << ccaDetectionTime << ")." << endl;
        scheduleAt(simTime() + rxSetupTime + ccaDetectionTime, ccaTimer);
    }
    else if (timer == TIMER_SIFS) {
        assert(useMACAcks);
        EV_DETAIL << "(startTimer) sifsTimer value=" << sifs << endl;
        scheduleAt(simTime() + sifs, sifsTimer);
    }
    else if (timer == TIMER_RX_ACK) {
        assert(useMACAcks);
        EV_DETAIL << "(startTimer) rxAckTimer value=" << macAckWaitDuration << endl;
        scheduleAt(simTime() + macAckWaitDuration, rxAckTimer);
    }
    else {
        EV << "Unknown timer requested to start:" << timer << endl;
    }
}

simtime_t CSMA::scheduleBackoff()
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
            error("Unknown backoff method!");
            break;
    }

    nbBackoffs = nbBackoffs + 1;
    backoffValues = backoffValues + SIMTIME_DBL(backoffTime);

    return backoffTime + simTime();
}

/*
 * Binds timers to events and executes FSM.
 */
void CSMA::handleSelfMessage(cMessage *msg)
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
void CSMA::handleLowerPacket(cPacket *msg)
{
    CSMAFrame *macPkt = static_cast<CSMAFrame *>(msg);
    const MACAddress& src = macPkt->getSrcAddr();
    const MACAddress& dest = macPkt->getDestAddr();
    long ExpectedNr = 0;

    EV_DETAIL << "Received frame name= " << macPkt->getName()
              << ", myState=" << macState << " src=" << src
              << " dst=" << dest << " myAddr="
              << address << endl;

    if (dest == address) {
        if (!useMACAcks) {
            EV_DETAIL << "Received a data packet addressed to me." << endl;
//			nbRxFrames++;
            executeMac(EV_FRAME_RECEIVED, macPkt);
        }
        else {
            long SeqNr = macPkt->getSequenceId();

            if (strcmp(macPkt->getName(), "CSMA-Ack") != 0) {
                // This is a data message addressed to us
                // and we should send an ack.
                // we build the ack packet here because we need to
                // copy data from macPkt (src).
                EV_DETAIL << "Received a data packet addressed to me,"
                          << " preparing an ack..." << endl;

//				nbRxFrames++;

                if (ackMessage != NULL)
                    delete ackMessage;
                ackMessage = new CSMAFrame("CSMA-Ack");
                ackMessage->setSrcAddr(address);
                ackMessage->setDestAddr(src);
                ackMessage->setBitLength(ackLength);
                //Check for duplicates by checking expected seqNr of sender
                if (SeqNrChild.find(src) == SeqNrChild.end()) {
                    //no record of current child -> add expected next number to map
                    SeqNrChild[src] = SeqNr + 1;
                    EV_DETAIL << "Adding a new child to the map of Sequence numbers:" << src << endl;
                    executeMac(EV_FRAME_RECEIVED, macPkt);
                }
                else {
                    ExpectedNr = SeqNrChild[src];
                    EV_DETAIL << "Expected Sequence number is " << ExpectedNr
                              << " and number of packet is " << SeqNr << endl;
                    if (SeqNr < ExpectedNr) {
                        //Duplicate Packet, count and do not send to upper layer
                        nbDuplicates++;
                        executeMac(EV_DUPLICATE_RECEIVED, macPkt);
                    }
                    else {
                        SeqNrChild[src] = SeqNr + 1;
                        executeMac(EV_FRAME_RECEIVED, macPkt);
                    }
                }
            }
            else if (macQueue.size() != 0) {
                // message is an ack, and it is for us.
                // Is it from the right node ?
                CSMAFrame *firstPacket = static_cast<CSMAFrame *>(macQueue.front());
                if (src == firstPacket->getDestAddr()) {
                    nbRecvdAcks++;
                    executeMac(EV_ACK_RECEIVED, macPkt);
                }
                else {
                    EV << "Error! Received an ack from an unexpected source: src=" << src << ", I was expecting from node addr=" << firstPacket->getDestAddr() << endl;
                    delete macPkt;
                }
            }
            else {
                EV << "Error! Received an Ack while my send queue was empty. src=" << src << "." << endl;
                delete macPkt;
            }
        }
    }
    else if (dest.isBroadcast()) {
        executeMac(EV_BROADCAST_RECEIVED, macPkt);
    }
    else {
        EV_DETAIL << "packet not for me, deleting...\n";
        delete macPkt;
    }
}

void CSMA::receiveSignal(cComponent *source, simsignal_t signalID, long value)
{
    Enter_Method_Silent();
    if (signalID == IRadio::transmissionStateChangedSignal) {
        IRadio::TransmissionState newRadioTransmissionState = (IRadio::TransmissionState)value;
        if (transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING && newRadioTransmissionState == IRadio::TRANSMISSION_STATE_IDLE) {
            // KLUDGE: we used to get a cMessage from the radio (the identity was not important)
            executeMac(EV_FRAME_TRANSMITTED, new cMessage("Transmission over"));
        }
        transmissionState = newRadioTransmissionState;
    }
}

cPacket *CSMA::decapsMsg(CSMAFrame *macPkt)
{
    cPacket *msg = macPkt->decapsulate();
    setUpControlInfo(msg, macPkt->getSrcAddr());

    return msg;
}

/**
 * Attaches a "control info" (MacToNetw) structure (object) to the message pMsg.
 */
cObject *CSMA::setUpControlInfo(cMessage *const pMsg, const MACAddress& pSrcAddr)
{
    SimpleLinkLayerControlInfo *const cCtrlInfo = new SimpleLinkLayerControlInfo();
    cCtrlInfo->setSrc(pSrcAddr);
    pMsg->setControlInfo(cCtrlInfo);
    return cCtrlInfo;
}

} // namespace inet


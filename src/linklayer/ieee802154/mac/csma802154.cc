/* -*- mode:c++ -*- ********************************************************
 * file:        csma.cc
 *
 * author:      Jerome Rousselot, Marcel Steine, Amre El-Hoiydi,
 *              Marc Loebbers, Yosia Hadisusanto, Andreas Koepke
 *
 * copyright:   (C) 2007-2009 CSEM SA
 *              (C) 2009 T.U. Eindhoven
 *              (C) 2004,2005,2006
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

#include "csma802154.h"
#include "InterfaceTableAccess.h"
#include "MACAddress.h"
#include "Ieee802Ctrl_m.h"
#include <assert.h>

Define_Module(csma802154);
static uint64_t MacToUint64(const MACAddress &add)
{
    uint64_t aux;
    uint64_t lo=0;
    for (int i=0; i<MAC_ADDRESS_BYTES; i++)
    {
        aux  = add.getAddressByte(MAC_ADDRESS_BYTES-i-1);
        aux <<= 8*i;
        lo  |= aux ;
    }
    return lo;
}

static MACAddress Uint64ToMac(uint64_t lo)
{
    MACAddress add;
    add.setAddressByte(0, (lo>>40)&0xff);
    add.setAddressByte(1, (lo>>32)&0xff);
    add.setAddressByte(2, (lo>>24)&0xff);
    add.setAddressByte(3, (lo>>16)&0xff);
    add.setAddressByte(4, (lo>>8)&0xff);
    add.setAddressByte(5, lo&0xff);
    return add;
}

/**
 * Initialize the of the omnetpp.ini variables in stage 1. In stage
 * two subscribe to the RadioState.
 */

#define L2BROADCAST def_macCoordExtendedAddress
void csma802154::sendUp(cMessage *msg)
{
    mpNb->fireChangeNotification(NF_LINK_PROMISCUOUS, msg);
    send(msg, mUppergateOut);
}

void csma802154::initialize(int stage)
{

    cSimpleModule::initialize(stage);
    if (stage == 0)
    {
        //get my mac address
        useIeee802Ctrl=true;

        const char *addressString = par("address");
        if (!strcmp(addressString, "auto"))
        {
            // assign automatic address
            macaddress = MACAddress::generateAutoAddress();
            // change module parameter from "auto" to concrete address
            par("address").setStringValue(macaddress.str().c_str());
        }
        else
            macaddress.setAddress(addressString);

        registerInterface();

        // get gate ID
        mUppergateIn  = findGate("uppergateIn");
        mUppergateOut = findGate("uppergateOut");
        mLowergateIn  = findGate("lowergateIn");
        mLowergateOut = findGate("lowergateOut");

        // get a pointer to the NotificationBoard module
        mpNb = NotificationBoardAccess().get();
        // subscribe for the information of the carrier sense
        mpNb->subscribe(this, NF_RADIOSTATE_CHANGED);
        //mpNb->subscribe(this, NF_BITRATE_CHANGED);
        mpNb->subscribe(this, NF_RADIO_CHANNEL_CHANGED);
        radioState = RadioState::IDLE;

        // obtain pointer to external queue
        initializeQueueModule();

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
        stats = par("stats");
        trace = par("trace");
        macMaxCSMABackoffs = par("macMaxCSMABackoffs");
        macMaxFrameRetries = par("macMaxFrameRetries");
        macAckWaitDuration = par("macAckWaitDuration").doubleValue();
        aUnitBackoffPeriod = par("aUnitBackoffPeriod");
        ccaDetectionTime = par("ccaDetectionTime").doubleValue();
        rxSetupTime = par("rxSetupTime").doubleValue();
        aTurnaroundTime = par("aTurnaroundTime").doubleValue();
        bitrate = getRate('b');//par("bitrate");
        ackLength = par("ackLength");
        ackMessage = NULL;

        //init parameters for backoff method
        std::string backoffMethodStr = par("backoffMethod").stdstringValue();
        if (backoffMethodStr == "exponential")
        {
            backoffMethod = EXPONENTIAL;
            macMinBE = par("macMinBE");
            macMaxBE = par("macMaxBE");
        }
        else
        {
            if (backoffMethodStr == "linear")
            {
                backoffMethod = LINEAR;
            }
            else if (backoffMethodStr == "constant")
            {
                backoffMethod = CONSTANT;
            }
            else
            {
                error("Unknown backoff method \"%s\".\
					   Use \"constant\", \"linear\" or \"\
					   \"exponential\".", backoffMethodStr.c_str());
            }
            initialCW = par("contentionWindow");
        }
        NB = 0;

        // txPower = par("txPower").doubleValue();


        nicId = getParentModule()->getId();



        // initialize the timers
        backoffTimer = new cMessage("timer-backoff");
        ccaTimer = new cMessage("timer-cca");
        sifsTimer = new cMessage("timer-sifs");
        rxAckTimer = new cMessage("timer-rxAck");
        macState = IDLE_1;
        txAttempts = 0;

    }
    else if (stage == 2)
    {
        EV << "queueLength = " << queueLength
        << " bitrate = " << bitrate
        << " backoff method = " << par("backoffMethod").stringValue() << endl;

        EV << "finished csma init stage 1." << endl;
    }
}

void csma802154::finish()
{
    if (stats)
    {
        recordScalar("nbTxFrames", nbTxFrames);
        recordScalar("nbRxFrames", nbRxFrames);
        recordScalar("nbDroppedFrames", nbDroppedFrames);
        recordScalar("nbMissedAcks", nbMissedAcks);
        recordScalar("nbRecvdAcks", nbRecvdAcks);
        recordScalar("nbTxAcks", nbTxAcks);
        recordScalar("nbDuplicates", nbDuplicates);
        if (nbBackoffs > 0)
        {
            recordScalar("meanBackoff", backoffValues / nbBackoffs);
        }
        else
        {
            recordScalar("meanBackoff", 0);
        }
        recordScalar("nbBackoffs", nbBackoffs);
        recordScalar("backoffDurations", backoffValues);
    }
}

csma802154::~csma802154()
{
    cancelAndDelete(backoffTimer);
    cancelAndDelete(ccaTimer);
    cancelAndDelete(sifsTimer);
    cancelAndDelete(rxAckTimer);
    if (ackMessage)
        delete ackMessage;
    MacQueue::iterator it;
    for (it = macQueue.begin(); it != macQueue.end(); ++it)
    {
        delete (*it);
    }
}


void csma802154::handleMessage(cMessage* msg)
{

    if (msg->getArrivalGateId() == mLowergateIn && !msg->isPacket())
    {
        if (msg->getKind()==0)
            error("[MAC]: message '%s' with length==0 is supposed to be a primitive, but msg kind is also zero", msg->getName());
        handleLowerControl(msg);
        return;
    }

    if (msg->getArrivalGateId() == mLowergateIn)
    {
        mpNb->fireChangeNotification(NF_LINK_FULL_PROMISCUOUS, msg);
        handleLowerMsg(msg);
    }
    else if (msg->isSelfMessage())
    {
        handleSelfMsg(msg);
    }
    else
    {
        handleUpperMsg(msg);
    }
}

/**
 * Encapsulates the message to be transmitted and pass it on
 * to the FSM main method for further processing.
 */
void csma802154::handleUpperMsg(cMessage *msg)
{
    //MacPkt *macPkt = encapsMsg(msg);
    reqtMsgFromIFq();
    Ieee802154Frame *macPkt = new Ieee802154Frame(msg->getName());
    macPkt->setBitLength(def_phyHeaderLength);
    cObject *controlInfo = msg->removeControlInfo();
    IE3ADDR dest;
    if (dynamic_cast<Ieee802Ctrl *>(controlInfo))
    {
        useIeee802Ctrl = true;
        Ieee802Ctrl* cInfo = check_and_cast<Ieee802Ctrl *>(controlInfo);
        MACAddress destination = cInfo->getDest();
        dest = static_cast<IE3ADDR> (MacToUint64(destination));
    }
    else
    {
        useIeee802Ctrl = false;
        Ieee802154NetworkCtrlInfo* cInfo = check_and_cast<Ieee802154NetworkCtrlInfo *>(controlInfo);

        if (cInfo->getNetwAddr()==-1)
        {
            if (!simulation.getModuleByPath(cInfo->getDestName()))
                error("[MAC]: address conversion fails, destination host does not exist!");
            cModule* module = simulation.getModuleByPath(cInfo->getDestName())->getModuleByRelativePath("nic.mac");
            Ieee802154Mac* macModule = check_and_cast<Ieee802154Mac *>(module);
            dest = macModule->getMacAddr();
        }
        else
            dest=cInfo->getNetwAddr();

    }
    delete controlInfo;
    macPkt->setDstAddr(dest);

    EV<<"CSMA received a message from upper layer, name is " << msg->getName() <<", CInfo removed, mac addr="<< dest <<endl;
    macPkt->setSrcAddr(getMacAddr());

    if (useMACAcks)
    {
        if (SeqNrParent.find(dest) == SeqNrParent.end())
        {
            //no record of current parent -> add next sequence number to map
            SeqNrParent[dest] = 1;
            macPkt->setBdsn(0);
            EV << "Adding a new parent to the map of Sequence numbers:" << dest << endl;
        }
        else
        {
            macPkt->setBdsn(SeqNrParent[dest]);
            EV << "Packet send with sequence number = " << SeqNrParent[dest] << endl;
            SeqNrParent[dest]++;
        }
    }

    //RadioAccNoise3PhyControlInfo *pco = new RadioAccNoise3PhyControlInfo(bitrate);
    //macPkt->setControlInfo(pco);
    assert(static_cast<cPacket*>(msg));
    macPkt->encapsulate(PK(msg));
    EV <<"pkt encapsulated, length: " << macPkt->getBitLength() << "\n";
    executeMac(EV_SEND_REQUEST, macPkt);
}

void csma802154::updateStatusIdle(t_mac_event event, cMessage *msg)
{
    switch (event)
    {
    case EV_SEND_REQUEST:
        if (macQueue.size() <= queueLength)
        {
            macQueue.push_back(static_cast<Ieee802154Frame *> (msg));
            EV<<"(1) FSM State IDLE_1, EV_SEND_REQUEST and [TxBuff avail]: startTimerBackOff -> BACKOFF." << endl;
            updateMacState(BACKOFF_2);
            NB = 0;
            //BE = macMinBE;
            startTimer(TIMER_BACKOFF);
        }
        else
        {
            // queue is full, message has to be deleted
            EV << "(12) FSM State IDLE_1, EV_SEND_REQUEST and [TxBuff not avail]: dropping packet -> IDLE." << endl;
            //msg->setName("MAC ERROR");
            //msg->setKind(PACKET_DROPPED);
            //sendControlUp(msg);
            delete msg;
            //droppedPacket.setReason(DroppedPacket::QUEUE);
            //utility->publishBBItem(catDroppedPacket, &droppedPacket, nicId);
            updateMacState(IDLE_1);
        }
        break;
    case EV_DUPLICATE_RECEIVED:
        EV << "(15) FSM State IDLE_1, EV_DUPLICATE_RECEIVED: setting up radio tx -> WAITSIFS." << endl;
        //sendUp(decapsMsg(static_cast<MacSeqPkt *>(msg)));
        delete msg;

        if (useMACAcks)
        {
            PLME_SET_TRX_STATE_request(phy_TX_ON);
            //phy->setRadioState(Radio::TX);
            updateMacState(WAITSIFS_6);
            startTimer(TIMER_SIFS);
        }
        break;

    case EV_FRAME_RECEIVED:
        EV << "(15) FSM State IDLE_1, EV_FRAME_RECEIVED: setting up radio tx -> WAITSIFS." << endl;
        sendUp(decapsMsg(static_cast<Ieee802154Frame *>(msg)));
        delete msg;

        if (useMACAcks)
        {
            PLME_SET_TRX_STATE_request(phy_TX_ON);
            //phy->setRadioState(Radio::TX);
            updateMacState(WAITSIFS_6);
            startTimer(TIMER_SIFS);
        }
        break;

    case EV_BROADCAST_RECEIVED:
        EV << "(23) FSM State IDLE_1, EV_BROADCAST_RECEIVED: Nothing to do." << endl;
        sendUp(decapsMsg(static_cast<Ieee802154Frame *>(msg)));
        delete msg;
        break;
    default:
        fsmError(event, msg);
    }
}

void csma802154::updateStatusBackoff(t_mac_event event, cMessage *msg)
{
    switch (event)
    {
    case EV_TIMER_BACKOFF:
        EV<< "(2) FSM State BACKOFF, EV_TIMER_BACKOFF:"
        << " starting CCA timer." << endl;
        startTimer(TIMER_CCA);
        updateMacState(CCA_3);
        PLME_SET_TRX_STATE_request(phy_RX_ON);
        //phy->setRadioState(Radio::RX);
        break;
    case EV_DUPLICATE_RECEIVED:
        // suspend current transmission attempt,
        // transmit ack,
        // and resume transmission when entering manageQueue()
        EV << "(28) FSM State BACKOFF, EV_DUPLICATE_RECEIVED:";
        if (useMACAcks)
        {
            EV << "suspending current transmit tentative and transmitting ack";
            transmissionAttemptInterruptedByRx = true;
            cancelEvent(backoffTimer);
            PLME_SET_TRX_STATE_request(phy_TX_ON);
            //phy->setRadioState(Radio::TX);
            updateMacState(WAITSIFS_6);
            startTimer(TIMER_SIFS);
        }
        else
        {
            EV << "Nothing to do.";
        }
        //sendUp(decapsMsg(static_cast<MacSeqPkt *>(msg)));
        delete msg;

        break;
    case EV_FRAME_RECEIVED:
        // suspend current transmission attempt,
        // transmit ack,
        // and resume transmission when entering manageQueue()
        EV << "(28) FSM State BACKOFF, EV_FRAME_RECEIVED:";
        if (useMACAcks)
        {
            EV << "suspending current transmit tentative and transmitting ack";
            transmissionAttemptInterruptedByRx = true;
            cancelEvent(backoffTimer);
            PLME_SET_TRX_STATE_request(phy_TX_ON);
            //phy->setRadioState(Radio::TX);
            updateMacState(WAITSIFS_6);
            startTimer(TIMER_SIFS);
        }
        else
        {
            EV << "sending frame up and resuming normal operation.";
        }
        sendUp(decapsMsg(static_cast<Ieee802154Frame *>(msg)));
        delete msg;
        break;
    case EV_BROADCAST_RECEIVED:
        EV << "(29) FSM State BACKOFF, EV_BROADCAST_RECEIVED:"
        << "sending frame up and resuming normal operation." <<endl;
        sendUp(decapsMsg(static_cast<Ieee802154Frame *>(msg)));
        delete msg;
        break;
    default:
        fsmError(event, msg);
    }
}


void csma802154::updateStatusCCA(t_mac_event event, cMessage *msg)
{
    switch (event)
    {
    case EV_TIMER_CCA:
    {
        EV<< "(25) FSM State CCA_3, EV_TIMER_CCA" << endl;
        bool isIdle = radioState  == RadioState::IDLE;
        if (isIdle)
        {
            EV << "(3) FSM State CCA_3, EV_TIMER_CCA, [Channel Idle]: -> TRANSMITFRAME_4." << endl;
            updateMacState(TRANSMITFRAME_4);
            PLME_SET_TRX_STATE_request(phy_TX_ON);
            //phy->setRadioState(Radio::TX);
            Ieee802154Frame * mac = check_and_cast<Ieee802154Frame *>(macQueue.front()->dup());
            //sendDown(msg);
            // give time for the radio to be in Tx state before transmitting
            //sendDelayed(mac, aTurnaroundTime, mLowergateOut);
            sendNewPacketInTx(mac);
            nbTxFrames++;
        }
        else
        {
            // Channel was busy, increment 802.15.4 backoff timers as specified.
            EV << "(7) FSM State CCA_3, EV_TIMER_CCA, [Channel Busy]: "
            << " increment counters." << endl;
            NB = NB+1;
            //BE = std::min(BE+1, macMaxBE);

            // decide if we go for another backoff or if we drop the frame.
            if (NB> macMaxCSMABackoffs)
            {
                // drop the frame
                EV << "Tried " << NB << " backoffs, all reported a busy "
                << "channel. Dropping the packet." << endl;
                cMessage * mac = macQueue.front();
                macQueue.pop_front();
                txAttempts = 0;
                nbDroppedFrames++;
                //mac->setName("MAC ERROR");
                //mac->setKind(PACKET_DROPPED);
                //sendControlUp(mac);
                delete mac;
                manageQueue();
            }
            else
            {
                // redo backoff
                updateMacState(BACKOFF_2);
                startTimer(TIMER_BACKOFF);
            }
        }
        break;
    }
    case EV_DUPLICATE_RECEIVED:
        EV << "(26) FSM State CCA_3, EV_DUPLICATE_RECEIVED:";
        if (useMACAcks)
        {
            EV << " setting up radio tx -> WAITSIFS." << endl;
            // suspend current transmission attempt,
            // transmit ack,
            // and resume transmission when entering manageQueue()
            transmissionAttemptInterruptedByRx = true;
            cancelEvent(ccaTimer);
            PLME_SET_TRX_STATE_request(phy_TX_ON);
            //phy->setRadioState(Radio::TX);
            updateMacState(WAITSIFS_6);
            startTimer(TIMER_SIFS);
        }
        else
        {
            EV << " Nothing to do." << endl;
        }
        //sendUp(decapsMsg(static_cast<MacPkt *>(msg)));
        delete msg;
        break;

    case EV_FRAME_RECEIVED:
        EV << "(26) FSM State CCA_3, EV_FRAME_RECEIVED:";
        if (useMACAcks)
        {
            EV << " setting up radio tx -> WAITSIFS." << endl;
            // suspend current transmission attempt,
            // transmit ack,
            // and resume transmission when entering manageQueue()
            transmissionAttemptInterruptedByRx = true;
            cancelEvent(ccaTimer);
            PLME_SET_TRX_STATE_request(phy_TX_ON);
            //phy->setRadioState(Radio::TX);
            updateMacState(WAITSIFS_6);
            startTimer(TIMER_SIFS);
        }
        else
        {
            EV << " Nothing to do." << endl;
        }
        sendUp(decapsMsg(static_cast<Ieee802154Frame *>(msg)));
        delete msg;
        break;
    case EV_BROADCAST_RECEIVED:
        EV << "(24) FSM State BACKOFF, EV_BROADCAST_RECEIVED:"
        << " Nothing to do." << endl;
        sendUp(decapsMsg(static_cast<Ieee802154Frame *>(msg)));
        delete msg;
        break;
    default:
        fsmError(event, msg);
    }
}

void csma802154::updateStatusTransmitFrame(t_mac_event event, cMessage *msg)
{
    if (event == EV_FRAME_TRANSMITTED)
    {
        //    delete msg;
        Ieee802154Frame * packet = macQueue.front();
        PLME_SET_TRX_STATE_request(phy_RX_ON);
        //phy->setRadioState(Radio::RX);

        bool expectAck = useMACAcks;
        if (packet->getDstAddr() != L2BROADCAST)
        {
            //unicast
            EV << "(4) FSM State TRANSMITFRAME_4, "
            << "EV_FRAME_TRANSMITTED [Unicast]: ";
        }
        else
        {
            //broadcast
            EV << "(27) FSM State TRANSMITFRAME_4, EV_FRAME_TRANSMITTED "
            << " [Broadcast]";
            expectAck = false;
        }

        if (expectAck)
        {
            EV << "RadioSetupRx -> WAITACK." << endl;
            updateMacState(WAITACK_5);
            startTimer(TIMER_RX_ACK);
        }
        else
        {
            EV << ": RadioSetupRx, manageQueue..." << endl;
            macQueue.pop_front();
            delete packet;
            manageQueue();
        }
    }
    else
    {
        fsmError(event, msg);
    }
}

void csma802154::updateStatusWaitAck(t_mac_event event, cMessage *msg)
{
    assert(useMACAcks);

    cMessage * mac;
    switch (event)
    {
    case EV_ACK_RECEIVED:
        EV<< "(5) FSM State WAITACK_5, EV_ACK_RECEIVED: "
        << " ProcessAck, manageQueue..." << endl;
        if (rxAckTimer->isScheduled())
            cancelEvent(rxAckTimer);
        mac = static_cast<cMessage *>(macQueue.front());
        macQueue.pop_front();
        txAttempts = 0;
//      mac->setName("MAC SUCCESS");
//      mac->setKind(TX_OVER);
//      sendControlUp(mac);
        delete mac;
        delete msg;
        manageQueue();
        break;
    case EV_ACK_TIMEOUT:
        EV << "(12) FSM State WAITACK_5, EV_ACK_TIMEOUT:"
        << " incrementCounter/dropPacket, manageQueue..." << endl;
        manageMissingAck(event, msg);
        break;
    case EV_BROADCAST_RECEIVED:
    case EV_FRAME_RECEIVED:
        sendUp(decapsMsg(static_cast<Ieee802154Frame*>(msg)));
    case EV_DUPLICATE_RECEIVED:
        EV << "Error ! Received a frame during SIFS !" << endl;
        delete msg;
        break;
    default:
        fsmError(event, msg);
    }

}

void csma802154::manageMissingAck(t_mac_event event, cMessage *msg)
{
    if (txAttempts < macMaxFrameRetries + 1)
    {
        // increment counter
        txAttempts++;
        EV<< "I will retransmit this packet (I already tried "
        << txAttempts << " times)." << endl;
    }
    else
    {
        // drop packet
        EV << "Packet was transmitted " << txAttempts
        << " times and I never got an Ack. I drop the packet." << endl;
        cMessage * mac = macQueue.front();
        macQueue.pop_front();
        txAttempts = 0;
        //mac->setName("MAC ERROR");
        //mac->setKind(PACKET_DROPPED);
        //sendControlUp(mac);
        mpNb->fireChangeNotification(NF_LINK_BREAK, mac);
        delete mac;
    }
    manageQueue();
}
void csma802154::updateStatusSIFS(t_mac_event event, cMessage *msg)
{
    assert(useMACAcks);

    switch (event)
    {
    case EV_TIMER_SIFS:
        EV<< "(17) FSM State WAITSIFS_6, EV_TIMER_SIFS:"
        << " sendAck -> TRANSMITACK." << endl;
        updateMacState(TRANSMITACK_7);
        //attachSignal(ackMessage, simTime());
        //sendDown(ackMessage);
        sendNewPacketInTx(ackMessage);
        nbTxAcks++;
        //      sendDelayed(ackMessage, aTurnaroundTime, lowergateOut);
        ackMessage = NULL;
        break;
    case EV_TIMER_BACKOFF:
        // Backoff timer has expired while receiving a frame. Restart it
        // and stay here.
        EV << "(16) FSM State WAITSIFS_6, EV_TIMER_BACKOFF. "
        << "Restart backoff timer and don't move." << endl;
        startTimer(TIMER_BACKOFF);
        break;
    case EV_BROADCAST_RECEIVED:
    case EV_FRAME_RECEIVED:
        EV << "Error ! Received a frame during SIFS !" << endl;
        sendUp(decapsMsg(static_cast<Ieee802154Frame*>(msg)));
        delete msg;
        break;
    default:
        fsmError(event, msg);
    }
}

void csma802154::updateStatusTransmitAck(t_mac_event event, cMessage *msg)
{
    assert(useMACAcks);

    if (event == EV_FRAME_TRANSMITTED)
    {
        EV<< "(19) FSM State TRANSMITACK_7, EV_FRAME_TRANSMITTED:"
        << " ->manageQueue." << endl;
        PLME_SET_TRX_STATE_request(phy_RX_ON);
        //phy->setRadioState(Radio::RX);
        //      delete msg;
        manageQueue();
    }
    else
    {
        fsmError(event, msg);
    }
}

void csma802154::updateStatusNotIdle(cMessage *msg)
{
    EV<< "(20) FSM State NOT IDLE, EV_SEND_REQUEST. Is a TxBuffer available ?" << endl;
    if (macQueue.size() <= queueLength)
    {
        macQueue.push_back(static_cast<Ieee802154Frame *>(msg));
        EV << "(21) FSM State NOT IDLE, EV_SEND_REQUEST"
        <<" and [TxBuff avail]: enqueue packet and don't move." << endl;
    }
    else
    {
        // queue is full, message has to be deleted
        EV << "(22) FSM State NOT IDLE, EV_SEND_REQUEST"
        << " and [TxBuff not avail]: dropping packet and don't move."
        << endl;
        //msg->setName("MAC ERROR");
        //msg->setKind(PACKET_DROPPED);
        //sendControlUp(msg);
        delete msg;
        //droppedPacket.setReason(DroppedPacket::QUEUE);
        //utility->publishBBItem(catDroppedPacket, &droppedPacket, nicId);
    }

}
/**
 * Updates state machine.
 */
void csma802154::executeMac(t_mac_event event, cMessage *msg)
{
    EV<< "In executeMac" << endl;
    if (macState != IDLE_1 && event == EV_SEND_REQUEST)
    {
        updateStatusNotIdle(msg);
    }
    else
    {
        switch (macState)
        {
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
        }
    }
}

void csma802154::manageQueue()
{
    if (macQueue.size() != 0)
    {
        EV<< "(manageQueue) there are " << macQueue.size() << " packets to send, entering backoff wait state." << endl;
        if (! backoffTimer->isScheduled())
            startTimer(TIMER_BACKOFF);
        updateMacState(BACKOFF_2);
        if ( transmissionAttemptInterruptedByRx)
        {
            // resume a transmission cycle which was interrupted by
            // a frame reception during CCA check
            transmissionAttemptInterruptedByRx = false;
        }
        else
        {
            // initialize counters if we start a new transmission
            // cycle from zero
            NB = 0;
            //BE = macMinBE;
        }
    }
    else
    {
        EV << "(manageQueue) no packets to send, entering IDLE state." << endl;
        updateMacState(IDLE_1);
    }
}

void csma802154::updateMacState(t_mac_states newMacState)
{
    macState = newMacState;
}

/*
 * Called by the FSM machine when an unknown transition is requested.
 */
void csma802154::fsmError(t_mac_event event, cMessage *msg)
{
    EV<< "FSM Error ! In state " << macState << ", received unknown event:" << event << "." << endl;
    if (msg != NULL)
        delete msg;
}

void csma802154::startTimer(t_mac_timer timer)
{
    if (timer == TIMER_BACKOFF)
    {
        scheduleAt(scheduleBackoff(), backoffTimer);
    }
    else if (timer == TIMER_CCA)
    {
        simtime_t ccaTime = rxSetupTime + ccaDetectionTime;
        EV<< "(startTimer) ccaTimer value=" << ccaTime
        << "(rxSetupTime,ccaDetectionTime:" << rxSetupTime
        << "," << ccaDetectionTime <<")." << endl;
        scheduleAt(simTime()+rxSetupTime+ccaDetectionTime, ccaTimer);
    }
    else if (timer==TIMER_SIFS)
    {
        assert(useMACAcks);
        EV << "(startTimer) sifsTimer value=" << sifs << endl;
        scheduleAt(simTime()+sifs, sifsTimer);
    }
    else if (timer==TIMER_RX_ACK)
    {
        assert(useMACAcks);
        simtime_t ackDuration = ((double)(ackLength+(def_phyHeaderLength*8)))/bitrate;
        EV << "(startTimer) rxAckTimer value=" << macAckWaitDuration <<" + " << ackDuration<< endl;
        scheduleAt(simTime()+ackDuration+macAckWaitDuration, rxAckTimer);
    }
    else
    {
        EV << "Unknown timer requested to start:" << timer << endl;
    }
}

double csma802154::scheduleBackoff()
{

    double backoffTime;

    switch (backoffMethod)
    {
    case EXPONENTIAL:
    {
        int BE = std::min(macMinBE + NB, macMaxBE);
        double d = pow((double) 2, (int) BE);
        int v = (int) d - 1;
        int r = intuniform(1, v, 0);
        backoffTime = r * aUnitBackoffPeriod.dbl();

        EV<< "(startTimer) backoffTimer value=" << backoffTime
        << " (BE=" << BE << ", 2^BE-1= " << v << "r="
        << r << ")" << endl;
        break;
    }
    case LINEAR:
    {
        int slots = intuniform(1, initialCW + NB, 0);
        backoffTime = slots * aUnitBackoffPeriod.dbl();
        EV<< "(startTimer) backoffTimer value=" << backoffTime << endl;
        break;
    }
    case CONSTANT:
    {
        int slots = intuniform(1, initialCW, 0);
        backoffTime = slots * aUnitBackoffPeriod.dbl();
        EV<< "(startTimer) backoffTimer value=" << backoffTime << endl;
        break;
    }
    default:
        error("Unknown backoff method!");
    }

    nbBackoffs = nbBackoffs + 1;
    backoffValues = backoffValues + backoffTime;

    return backoffTime + simTime().dbl();
}

/*
 * Binds timers to events and executes FSM.
 */
void csma802154::handleSelfMsg(cMessage *msg)
{
    EV<< "timer routine." << endl;
    if (msg==backoffTimer)
        executeMac(EV_TIMER_BACKOFF, msg);
    else if (msg==ccaTimer)
        executeMac(EV_TIMER_CCA, msg);
    else if (msg==sifsTimer)
        executeMac(EV_TIMER_SIFS, msg);
    else if (msg==rxAckTimer)
    {
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
void csma802154::handleLowerMsg(cMessage *msg)
{
    Ieee802154Frame *macPkt = static_cast<Ieee802154Frame *> (msg);
    long src = macPkt->getSrcAddr();
    long dest = macPkt->getDstAddr();
    //long ExpectedNr = 0;
    uint8_t ExpectedNr = 0;

    if (macPkt->getKind()!=PACKETOK)
    {
        EV<< "Received with errors frame name= " << macPkt->getName()
        << ", myState=" << macState << " src=" << macPkt->getSrcAddr()
        << " dst=" << macPkt->getDstAddr() << " myAddr="
        << getMacAddr() << endl;
        if (macPkt->getKind() == COLLISION)
        {
            EV << "[MAC]: frame corrupted due to collision, dropped" << endl;
            numCollision++;
        }
        delete macPkt;
        return;
    }

    EV<< "Received frame name= " << macPkt->getName()
    << ", myState=" << macState << " src=" << macPkt->getSrcAddr()
    << " dst=" << macPkt->getDstAddr() << " myAddr="
    << getMacAddr() << endl;

    if (macPkt->getDstAddr() == getMacAddr())
    {
        if (!useMACAcks)
        {
            EV << "Received a data packet addressed to me." << endl;
            nbRxFrames++;
            executeMac(EV_FRAME_RECEIVED, macPkt);
        }
        else
        {
            //long SeqNr = macPkt->getSequenceId();
            //long SeqNr = macPkt->getBdsn();
            uint8_t SeqNr = macPkt->getBdsn();

            if (strcmp(macPkt->getName(), "CSMA-Ack") != 0)
            {
                // This is a data message addressed to us
                // and we should send an ack.
                // we build the ack packet here because we need to
                // copy data from macPkt (src).
                EV << "Received a data packet addressed to me,"
                << " preparing an ack..." << endl;

                nbRxFrames++;

                if (ackMessage != NULL)
                    delete ackMessage;
                ackMessage = new Ieee802154Frame("CSMA-Ack");
                ackMessage->setSrcAddr(getMacAddr());
                ackMessage->setDstAddr(macPkt->getSrcAddr());
                ackMessage->setBitLength(ackLength);
                //Check for duplicates by checking expected seqNr of sender
                if (SeqNrChild.find(src) == SeqNrChild.end())
                {
                    //no record of current child -> add expected next number to map
                    SeqNrChild[src] = SeqNr + 1;
                    EV << "Adding a new child to the map of Sequence numbers:" << src << endl;
                    executeMac(EV_FRAME_RECEIVED, macPkt);
                }
                else
                {
                    ExpectedNr = SeqNrChild[src];
                    EV << "Expected Sequence number is " << ExpectedNr <<
                    " and number of packet is " << SeqNr << endl;
                    int8_t sub     = ((int8_t)SeqNr) - ((int8_t) ExpectedNr);
                    //if (SeqNr < ExpectedNr)
                    if (sub < 0)
                    {
                        //Duplicate Packet, count and do not send to upper layer
                        nbDuplicates++;
                        executeMac(EV_DUPLICATE_RECEIVED, macPkt);
                    }
                    else
                    {
                        SeqNrChild[src] = SeqNr + 1;
                        executeMac(EV_FRAME_RECEIVED, macPkt);
                    }
                }

            }
            else if (macQueue.size() != 0)
            {

                // message is an ack, and it is for us.
                // Is it from the right node ?
                Ieee802154Frame * firstPacket = static_cast<Ieee802154Frame *>(macQueue.front());
                if (macPkt->getSrcAddr() == firstPacket->getDstAddr())
                {
                    nbRecvdAcks++;
                    executeMac(EV_ACK_RECEIVED, macPkt);
                }
                else
                {
                    EV << "Error! Received an ack from an unexpected source: src=" << macPkt->getSrcAddr() << ", I was expecting from node addr=" << firstPacket->getDstAddr() << endl;
                    delete macPkt;
                }
            }
            else
            {
                EV << "Error! Received an Ack while my send queue was empty. src=" << macPkt->getSrcAddr() << "." << endl;
                delete macPkt;
            }
        }
    }
    else if (dest == L2BROADCAST)
    {
        executeMac(EV_BROADCAST_RECEIVED, macPkt);
    }
    else
    {
        EV << "packet not for me, deleting...\n";
        delete macPkt;
    }
}


void csma802154::handleLowerControl(cMessage *msg)
{
    if (msg->getKind() == PD_DATA_CONFIRM)
        executeMac(EV_FRAME_TRANSMITTED, msg);
    else if (msg->getKind() == PLME_SET_TRX_STATE_CONFIRM)
    {
        Ieee802154MacPhyPrimitives* primitive = check_and_cast<Ieee802154MacPhyPrimitives *>(msg);
        phystatus = PHYenum(primitive->getStatus());
        if (primitive->getStatus()==phy_TX_ON && sendPacket)
        {
            sendDown(sendPacket);
            sendPacket=NULL;
        }
    }

    /*
    if (msg->getKind() == MacToPhyInterface::TX_OVER) {
        executeMac(EV_FRAME_TRANSMITTED, msg);
    } else if (msg->getKind() == BaseDecider::PACKET_DROPPED) {
        EV<< "control message: PACKED DROPPED" << endl;
    } else {
        EV << "Invalid control message type (type=NOTHING) : name="
        << msg->getName() << " modulesrc="
        << msg->getSenderModule()->getFullPath()
        << "." << endl;
    }*/
    delete msg;
}

/**
 * Update the internal copies of interesting BB variables
 *
 */
//void csma::receiveBBItem(int category, const BBItem *details, int scopeModuleId) {
//  Enter_Method_Silent();
//  BasicLayer::receiveBBItem(category, details, scopeModuleId);
//
//  if (category == catRadioState) {
//      radioState
//              = static_cast<const RadioAccNoise3State *> (details)->getState();
//      // radio just told us its state
//  } else if (category == catRSSI) {
//      rssi = static_cast<const RSSI *> (details)->getRSSI();
//      if (radioState == RadioAccNoise3State::RX) {
//          // we could do something here if we wanted to.
//      }
//  }
//}

cPacket *csma802154::decapsMsg(Ieee802154Frame * macPkt)
{
    cPacket * msg = macPkt->decapsulate();
    if (useIeee802Ctrl)
    {
        Ieee802Ctrl* cinfo = new Ieee802Ctrl();
        MACAddress destination = Uint64ToMac (macPkt->getSrcAddr());
        cinfo->setSrc(destination);
        msg->setControlInfo(cinfo);
    }
    else
    {
        Ieee802154NetworkCtrlInfo * cinfo = new Ieee802154NetworkCtrlInfo();
        cinfo->setNetwAddr(macPkt->getSrcAddr());
    }
    return msg;
}

void csma802154::receiveChangeNotification(int category, const cPolymorphic *details)
{
    Enter_Method_Silent();
    printNotificationBanner(category, details);

    switch (category)
    {

        if (check_and_cast<RadioState *>(details)->getRadioId()!=getRadioModuleId())
            return;

    case NF_RADIO_CHANNEL_CHANGED:
        ppib.phyCurrentChannel = check_and_cast<RadioState *>(details)->getChannelNumber();
        bitrate = getRate('b');
        phy_bitrate = bitrate;
        phy_symbolrate = getRate('s');
        bPeriod = aUnitBackoffPeriod / phy_symbolrate;
        break;
    case NF_RADIOSTATE_CHANGED:
        radioState = check_and_cast<RadioState *>(details)->getState();
        break;

        /*case NF_CHANNELS_SUPPORTED_CHANGED:
            ppib.phyChannelsSupported = check_and_cast<Ieee802154RadioState *>(details)->getPhyChannelsSupported();
            break;

        case NF_TRANSMIT_POWER_CHANGED:
            ppib.phyTransmitPower = check_and_cast<Ieee802154RadioState *>(details)->getPhyTransmitPower();
            break;

        case NF_CCA_MODE_CHANGED:
            ppib.phyCCAMode = check_and_cast<Ieee802154RadioState *>(details)->getPhyCCAMode();
            break;*/

    default:
        break;
    }
}

//
// Copyright (C) 2017 Jan Peter Drees
// Copyright (C) 2015 Joaquim Oller
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "XMacLayer.h"

#include <cassert>
#include "inet/common/ModuleAccess.h"
#include "inet/common/FindModule.h"
#include "inet/common/INETUtils.h"
#include "inet/common/queue/IPassiveQueue.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/common/SimpleLinkLayerControlInfo.h"
#include "inet/linklayer/contract/IMACProtocolControlInfo.h"
#include "inet/linklayer/xmac/XMacFrame_m.h"


namespace inet {

Define_Module(XMacLayer);

simsignal_t XMacLayer::packetFromUpperDroppedSignal = registerSignal("packetFromUpperDroppedSignal");

/**
 * Initialize method of XMacLayer. Init all parameters, schedule timers.
 */
void XMacLayer::initialize(int stage)
{
    MACProtocolBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {

        queueLength   = hasPar("queueLength")   ? par("queueLength")   : 10;
        animation     = hasPar("animation")     ? par("animation")     : true;
        slotDuration  = hasPar("slotDuration")  ? par("slotDuration")  : 1.;
        bitrate       = hasPar("bitrate")       ? par("bitrate")       : 15360.;
        headerLength = par("headerLength").intValue();
        checkInterval = hasPar("checkInterval") ? par("checkInterval") : 0.1;
        txPower       = hasPar("txPower")       ? par("txPower")       : 50.;
        useMacAcks    = hasPar("useMACAcks")    ? par("useMACAcks")    : false;
        maxTxAttempts = hasPar("maxTxAttempts") ? par("maxTxAttempts") : 2;
        EV_DEBUG << "headerLength: " << headerLength << ", bitrate: " << bitrate << endl;

        stats = par("stats");
        nbTxDataPackets = 0;
        nbTxPreambles = 0;
        nbRxDataPackets = 0;
        nbRxPreambles = 0;
        nbMissedAcks = 0;
        nbRecvdAcks=0;
        nbDroppedDataPackets=0;
        nbRxBrokenDataPackets = 0;
        nbTxAcks=0;

        txAttempts = 0;
        lastDataPktDestAddr = MACAddress::BROADCAST_ADDRESS;
        lastDataPktSrcAddr  = MACAddress::BROADCAST_ADDRESS;

        cModule *radioModule = getModuleFromPar<cModule>(par("radioModule"), this);
        radioModule->subscribe(IRadio::radioModeChangedSignal, this);
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);
        radio = check_and_cast<IRadio *>(radioModule);

        macState = INIT;
        WATCH(macState);
    }
    else if(stage == INITSTAGE_LINK_LAYER) {
        initializeMACAddress();
        registerInterface();

        wakeup = new cMessage("wakeup");
        wakeup->setKind(XMAC_WAKE_UP);

        data_timeout = new cMessage("data_timeout");
        data_timeout->setKind(XMAC_DATA_TIMEOUT);
        data_timeout->setSchedulingPriority(100);

        data_tx_over = new cMessage("data_tx_over");
        data_tx_over->setKind(XMAC_DATA_TX_OVER);

        stop_preambles = new cMessage("stop_preambles");
        stop_preambles->setKind(XMAC_STOP_PREAMBLES);

        send_preamble = new cMessage("send_preamble");
        send_preamble->setKind(XMAC_SEND_PREAMBLE);

        ack_tx_over = new cMessage("ack_tx_over");
        ack_tx_over->setKind(XMAC_ACK_TX_OVER);

        cca_timeout = new cMessage("cca_timeout");
        cca_timeout->setKind(XMAC_CCA_TIMEOUT);
        cca_timeout->setSchedulingPriority(100);

        send_ack = new cMessage("send_ack");
        send_ack->setKind(XMAC_SEND_ACK);

        start_xmac = new cMessage("start_xmac");
        start_xmac->setKind(XMAC_START_XMAC);

        ack_timeout = new cMessage("ack_timeout");
        ack_timeout->setKind(XMAC_ACK_TIMEOUT);

        resend_data = new cMessage("resend_data");
        resend_data->setKind(XMAC_RESEND_DATA);
        resend_data->setSchedulingPriority(100);

        switch_preamble_phase = new cMessage("switch_preamble_phase");
        switch_preamble_phase->setKind(SWITCH_PREAMBLE_PHASE);

        delay_for_ack_within_remote_rx = new cMessage("delay_for_ack_within_remote_rx");
        delay_for_ack_within_remote_rx->setKind(DELAY_FOR_ACK_WITHIN_REMOTE_RX);

        switching_done = new cMessage("switching_done");
        switching_done->setKind(XMAC_SWITCHING_FINISHED);

        scheduleAt(0.0, start_xmac);
    }
}

XMacLayer::~XMacLayer()
{
    cancelAndDelete(wakeup);
    cancelAndDelete(data_timeout);
    cancelAndDelete(data_tx_over);
    cancelAndDelete(stop_preambles);
    cancelAndDelete(send_preamble);
    cancelAndDelete(ack_tx_over);
    cancelAndDelete(cca_timeout);
    cancelAndDelete(send_ack);
    cancelAndDelete(start_xmac);
    cancelAndDelete(ack_timeout);
    cancelAndDelete(resend_data);
    cancelAndDelete(switch_preamble_phase);
    cancelAndDelete(delay_for_ack_within_remote_rx);
    cancelAndDelete(switching_done);
}

void XMacLayer::flushQueue()
{
    MacQueue::iterator it;
    for(it = macQueue.begin(); it != macQueue.end(); ++it)
    {
        delete (*it);
    }
    macQueue.clear();
}

void XMacLayer::clearQueue()
{
    macQueue.clear();
}

void XMacLayer::finish()
{
    // record stats
    if (stats)
    {
        recordScalar("nbTxDataPackets", nbTxDataPackets);
        recordScalar("nbTxPreambles", nbTxPreambles);
        recordScalar("nbRxDataPackets", nbRxDataPackets);
        recordScalar("nbRxPreambles", nbRxPreambles);
        recordScalar("nbMissedAcks", nbMissedAcks);
        recordScalar("nbRecvdAcks", nbRecvdAcks);
        recordScalar("nbTxAcks", nbTxAcks);
        recordScalar("nbDroppedDataPackets", nbDroppedDataPackets);
        recordScalar("nbRxBrokenDataPackets", nbRxBrokenDataPackets);
        //recordScalar("timeSleep", timeSleep);
        //recordScalar("timeRX", timeRX);
        //recordScalar("timeTX", timeTX);
    }
}

void XMacLayer::initializeMACAddress()
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

InterfaceEntry *XMacLayer::createInterfaceEntry()
{
    InterfaceEntry *e = new InterfaceEntry(this);

    // data rate
    e->setDatarate(bitrate);

    // generate a link-layer address to be used as interface token for IPv6
    e->setMACAddress(address);
    e->setInterfaceToken(address.formInterfaceIdentifier());

    // capabilities
    e->setMtu(par("mtu").intValue());
    e->setMulticast(false);
    e->setBroadcast(true);

    return e;
}

/**
 * Check whether the queue is not full: if yes, print a warning and drop the
 * packet. Then initiate sending of the packet, if the node is sleeping. Do
 * nothing, if node is working.
 */
void XMacLayer::handleUpperPacket(cPacket *msg)
{
    bool pktAdded = addToQueue(msg);
    if (!pktAdded)
        return;
    // force wakeup now
    if (wakeup->isScheduled() && (macState == SLEEP))
    {
        cancelEvent(wakeup);
        scheduleAt(simTime() + dblrand()*0.01f, wakeup);
    }else if(macState == SLEEP){
    }
}

/**
 * Send one short preamble packet immediately.
 */
void XMacLayer::sendPreamble(MACAddress preamble_address)
{
    //~ diff with BMAC, @ in preamble!
    XMacFrame* preamble = new XMacFrame("Preamble");
    preamble->setSrc(address);
    preamble->setDest(preamble_address);
    preamble->setKind(XMAC_PREAMBLE);
    preamble->setBitLength(headerLength);
    attachSignal(preamble, simTime());
    sendDown(preamble);
    nbTxPreambles++;
}

/**
 * Send one short preamble packet immediately.
 */
void XMacLayer::sendMacAck()
{
    XMacFrame* ack = new XMacFrame("Acknowledgment");
    ack->setSrc(address);
    //~ diff with BMAC, ack_preamble_based
    ack->setDest(lastPreamblePktSrcAddr);
    ack->setKind(XMAC_ACK);
    ack->setBitLength(headerLength);
    attachSignal(ack, simTime());
    sendDown(ack);
    nbTxAcks++;
}

/**
 * Handle own messages:
 */
void XMacLayer::handleSelfMessage(cMessage *msg)
{
    switch (macState)
    {
    case INIT:
        if (msg->getKind() == XMAC_START_XMAC)
        {
            EV_DEBUG << "State INIT, message XMAC_START, new state SLEEP" << endl;
            changeDisplayColor(BLACK);
            radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
            macState = SLEEP;
            scheduleAt(simTime()+dblrand()*slotDuration, wakeup);
            return;
        }
        break;
    case SLEEP:
        if (msg->getKind() == XMAC_WAKE_UP)
        {
            EV_DEBUG << "node " << address << " : State SLEEP, message XMAC_WAKEUP, new state CCA, simTime " <<
                    simTime() << " to " << simTime() + 1.7f * checkInterval << endl;
            // this CCA is useful when in RX to detect preamble and has to make room for
            // 0.2f = Tx switch, 0.5f = Tx send_preamble, 1f = time_for_ack_back
            scheduleAt(simTime() + 1.7f * checkInterval, cca_timeout);
            radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
            changeDisplayColor(GREEN);
            macState = CCA;
            return;
        }
        // we receive an ACK back but it is too late
        else if (msg->getKind() == XMAC_ACK) {
            nbMissedAcks++;
            delete msg;
            return;
        }
        // received messages prior real-switching to SLEEP? I'm sorry, out
        else { return; }
        break;
    case CCA:
        if (msg->getKind() == XMAC_CCA_TIMEOUT)
        {
            // channel is clear and we wanna SEND
            if (macQueue.size() > 0)
            {
                radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                changeDisplayColor(YELLOW);
                macState = SEND_PREAMBLE;
                // We send the preamble for a whole SLOT duration :)
                scheduleAt(simTime() + slotDuration, stop_preambles);
                // if 0.2f * CI = 2ms to switch to TX -> has to be accounted for RX_preamble_detection
                scheduleAt(simTime() + 0.2f * checkInterval, switch_preamble_phase);
                return;
            }
            // if anything to send, go back to sleep and wake up after a full period
            else
            {
                scheduleAt(simTime() + slotDuration, wakeup);
                macState = SLEEP;
                radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                changeDisplayColor(BLACK);
                return;
            }
        }
        // during CCA, we received a preamble. Go to state WAIT_DATA and
        // schedule the timeout.
        if (msg->getKind() == XMAC_PREAMBLE)
        {
            XMacFrame* incoming_preamble = static_cast<XMacFrame *>(msg);

            // preamble is for me
            if (incoming_preamble->getDest() == address || incoming_preamble->getDest().isBroadcast() || incoming_preamble->getDest().isMulticast()) {
                cancelEvent(cca_timeout);
                nbRxPreambles++;
                EV << "node " << address << " : State CCA, message XMAC_PREAMBLE received, new state SEND_ACK" << endl;
                macState = SEND_ACK;
                lastPreamblePktSrcAddr = incoming_preamble->getSrc();
                changeDisplayColor(YELLOW);
                radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
            }
            // the preamble is not for us
            else {
                EV << "node " << address << " : State CCA, message XMAC_PREAMBLE not for me." << endl;
                //~ better overhearing management? :)
                cancelEvent(cca_timeout);
                scheduleAt(simTime() + slotDuration, wakeup);
                macState = SLEEP;
                radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                changeDisplayColor(BLACK);
            }

            delete msg;
            return;
        }
        //in case we get an ACK, we simply dicard it, because it means the end
        //of another communication
        if (msg->getKind() == XMAC_ACK)
        {
            EV_DEBUG << "State CCA, message XMAC_ACK, new state CCA" << endl;
            delete msg;
            return;
        }
        // this case is very, very, very improbable, but let's do it.
        // if in CCA the node receives directly the data packet, accept it
        // even if we increased nbMissedAcks in state SLEEP
        if (msg->getKind() == XMAC_DATA)
        {
            XMacFrame* incoming_data = static_cast<XMacFrame *>(msg);

            // packet is for me
            if (incoming_data->getDest() == address) {
                EV << "node " << address << " : State CCA, received XMAC_DATA, accepting it." << endl;
                cancelEvent(cca_timeout);
                cancelEvent(switch_preamble_phase);
                cancelEvent(stop_preambles);
                macState = WAIT_DATA;
                scheduleAt(simTime(), msg);
            }
            return;
        }
        break;

    case SEND_PREAMBLE:
        if (msg->getKind() == SWITCH_PREAMBLE_PHASE)
        {
            //~ make room for preamble + time_for_ack_back, check_interval is 10ms by default (from NetworkXMAC.ini)
            // 0.5f* = 5ms
            if (radio->getRadioMode() == IRadio::RADIO_MODE_RECEIVER) {
                radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                changeDisplayColor(YELLOW);
                EV_DEBUG << "node " << address << " : preamble_phase tx, simTime = " << simTime() << endl;
                scheduleAt(simTime() + 0.5f * checkInterval, switch_preamble_phase);
            }
            // 1.0f* = 10ms
            else if (radio->getRadioMode() == IRadio::RADIO_MODE_TRANSMITTER) {
                radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
                changeDisplayColor(GREEN);
                EV_DEBUG << "node " << address << " : preamble_phase rx, simTime = " << simTime() << endl;
                scheduleAt(simTime() + 1.0f *checkInterval, switch_preamble_phase);
            }
            return;
        }
        // radio switch from above
        if(msg->getKind() == XMAC_SWITCHING_FINISHED) {
            if (radio->getRadioMode() == IRadio::RADIO_MODE_TRANSMITTER) {
                XMacFrame *pkt_preamble = macQueue.front();
                sendPreamble(pkt_preamble->getDest());
            }
            return;
        }
        // ack_rx within sending_preamble or preamble_timeout without an ACK
        if ((msg->getKind() == XMAC_ACK) || (msg->getKind() == XMAC_STOP_PREAMBLES))
        {
            //~ ADDED THE SECOND CONDITION! :) if not, below
            if (msg->getKind() == XMAC_ACK){
                delete msg;
                EV << "node " << address << " : State SEND_PREAMBLE, message XMAC_ACK, new state SEND_DATA" << endl;
            }else if (msg->getKind() == XMAC_STOP_PREAMBLES){
                EV << "node " << address << " : State SEND_PREAMBLE, message XMAC_STOP_PREAMBLES" << endl;
            }
            macState = SEND_DATA;
            cancelEvent(stop_preambles);
            cancelEvent(switch_preamble_phase);
            changeDisplayColor(RED);
            radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
            txAttempts = 1;
            return;
        }

        // next is the case of a node receiving 1 preamble or data while in his preamble gaps, ignore, we are sending!
        if ((msg->getKind() == XMAC_PREAMBLE) || (msg->getKind() == XMAC_DATA)) {
            if(msg->getKind() == XMAC_DATA){
                nbDroppedDataPackets++;
            }
            delete msg;
            return;
        }
        else {
            EV << "**node " << address << " : State SEND_PREAMBLE, received message " << msg->getKind() << endl;
            return;
        }
        break;

    case SEND_DATA:
        if (msg->getKind() == XMAC_STOP_PREAMBLES)
        {
            EV << "node " << address << " : State SEND_DATA, message XMAC_STOP_PREAMBLES" << endl;
            // send the data packet
            sendDataPacket();
            macState = WAIT_TX_DATA_OVER;
            return;
        }else if (msg->getKind() == XMAC_SWITCHING_FINISHED) {
            EV << "node " << address << " : State SEND_DATA, message RADIO_SWITCHING OVER, sending packet..." << endl;
            // send the data packet
            sendDataPacket();
            macState = WAIT_TX_DATA_OVER;
            return;
        }
        else {
            return;
        }
        break;

    case WAIT_TX_DATA_OVER:
        if (msg->getKind() == XMAC_DATA_TX_OVER)
        {
            EV_DEBUG << "node " << address << " : State WAIT_TX_DATA_OVER, message XMAC_DATA_TX_OVER, new state  SLEEP" << endl;
            // remove the packet just served from the queue
            delete macQueue.front();
            macQueue.pop_front();
            // if something in the queue, wakeup soon.
            if (macQueue.size() > 0)
                scheduleAt(simTime() + dblrand()*checkInterval, wakeup);
            else
                scheduleAt(simTime() + slotDuration, wakeup);
            macState = SLEEP;
            radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
            changeDisplayColor(BLACK);
            return;
        }
        break;
    case WAIT_ACK:
        //not used
        break;
    case WAIT_DATA:
        if (msg->getKind() == XMAC_PREAMBLE)
        {
            //nothing happens
            nbRxPreambles++;
            delete msg;
            return;
        }
        if (msg->getKind() == XMAC_ACK)
        {
            //nothing happens
            delete msg;
            return;
        }
        if (msg->getKind() == XMAC_DATA)
        {
            XMacFrame* mac  = static_cast<XMacFrame *>(msg);
            const MACAddress& dest = mac->getDest();

            if ((dest == address) || dest.isBroadcast() || dest.isMulticast()) {
                sendUp(decapsMsg(mac));
                delete mac;
                nbRxDataPackets++;
                cancelEvent(data_timeout);

                // if something in the queue, wakeup soon.
                if (macQueue.size() > 0)
                    scheduleAt(simTime() + dblrand()*checkInterval, wakeup);
                else
                    scheduleAt(simTime() + slotDuration, wakeup);
                macState = SLEEP;
                radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                changeDisplayColor(BLACK);

            } else {
                delete msg;
                msg = NULL;
                mac = NULL;
            }

            EV << "node " << address << " : State WAIT_DATA, message XMAC_DATA, new state  SLEEP" << endl;
            return;
        }
        // data does not arrives in time
        if (msg->getKind() == XMAC_DATA_TIMEOUT)
        {
            EV << "node " << address << " : State WAIT_DATA, message XMAC_DATA_TIMEOUT, new state SLEEP" << endl;
            // if something in the queue, wakeup soon.
            if (macQueue.size() > 0)
                scheduleAt(simTime() + dblrand()*checkInterval, wakeup);
            else
                scheduleAt(simTime() + slotDuration, wakeup);
            macState = SLEEP;
            radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
            changeDisplayColor(BLACK);
            return;
        }
        break;
    case SEND_ACK:
        // send now the ack packet
        if (msg->getKind() == DELAY_FOR_ACK_WITHIN_REMOTE_RX) {
            EV_DEBUG << "node " << address << " : State SEND_ACK, message XMAC_SEND_ACK, new state WAIT_ACK_TX" << endl;
            sendMacAck();
            macState = WAIT_ACK_TX;
            return;
        }
        break;
    case WAIT_ACK_TX:
        // wait for the ACK to be sent back to the Transmitter
        if (msg->getKind() == XMAC_ACK_TX_OVER)
        {
            EV_DEBUG << "node " << address << " : State WAIT_ACK_TX, message XMAC_ACK_TX_OVER, new state WAIT_DATA" << endl;
            changeDisplayColor(GREEN);
            macState = WAIT_DATA;
            cancelEvent(cca_timeout);
            scheduleAt(simTime() + (slotDuration / 2), data_timeout);
            radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
            return;
        }
        break;
    }
    throw cRuntimeError("Undefined event of type %d in state %d (Radio state %d)!",
              msg->getKind(), macState, radio->getRadioMode());
}


/**
 * Handle XMAC preambles and received data packets.
 */
void XMacLayer::handleLowerPacket(cPacket *msg)
{
    if (msg->hasBitError()) {
        EV << "Received " << msg << " contains bit errors or collision, dropping it\n";
        nbRxBrokenDataPackets++;
        delete msg;
        return;
    }
    // simply pass the massage as self message, to be processed by the FSM.
    handleSelfMessage(msg);
}

void XMacLayer::sendDataPacket()
{
    nbTxDataPackets++;
    XMacFrame* pkt = check_and_cast<XMacFrame *>(macQueue.front()->dup());
    attachSignal(pkt, simTime());
    lastDataPktDestAddr = pkt->getDest();
    pkt->setKind(XMAC_DATA);
    sendDown(pkt);
}

/**
 * Handle transmission over messages: either send another preambles or the data
 * packet itself.
 */
void XMacLayer::receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details)
{
    Enter_Method_Silent();
    if (signalID == IRadio::transmissionStateChangedSignal) {
        IRadio::TransmissionState newRadioTransmissionState = (IRadio::TransmissionState)value;
        if (transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING && newRadioTransmissionState == IRadio::TRANSMISSION_STATE_IDLE) {
            // Transmission of one packet is over
            if (macState == WAIT_TX_DATA_OVER)
            {
                scheduleAt(simTime(), data_tx_over);
            }
            if (macState == WAIT_ACK_TX)
            {
                scheduleAt(simTime(), ack_tx_over);
            }
        }
        transmissionState = newRadioTransmissionState;
    }else if(signalID ==IRadio::radioModeChangedSignal){
        // Radio switching (to RX or TX) is over, ignore switching to SLEEP.
        if(macState == SEND_PREAMBLE) {
            scheduleAt(simTime(), switching_done);
        }
        else if(macState == SEND_ACK) {
            scheduleAt(simTime() + 0.5f * checkInterval, delay_for_ack_within_remote_rx);
        }
        else if(macState == SEND_DATA) {
            scheduleAt(simTime(), switching_done);
        }
    }
}

/**
 * Encapsulates the received network-layer packet into a MacPkt and set all
 * needed header fields.
 */
bool XMacLayer::addToQueue(cMessage *msg)
{
    if (macQueue.size() >= queueLength) {
        // queue is full, message has to be deleted
        EV_DEBUG << "New packet arrived, but queue is FULL, so new packet is"
                  " deleted\n";
        emit(packetFromUpperDroppedSignal, msg);
        delete msg;
        nbDroppedDataPackets++;

        return false;
    }

    XMacFrame *macPkt = new XMacFrame(msg->getName());
    macPkt->setBitLength(headerLength);
    IMACProtocolControlInfo *const cInfo = check_and_cast<IMACProtocolControlInfo *>(msg->removeControlInfo());
    EV_DETAIL << "CSMA received a message from upper layer, name is " << msg->getName() << ", CInfo removed, mac addr=" << cInfo->getDestinationAddress() << endl;
    MACAddress dest = cInfo->getDestinationAddress();
    macPkt->setDest(dest);
    delete cInfo;
    macPkt->setSrc(address);

    assert(static_cast<cPacket*>(msg));
    macPkt->encapsulate(static_cast<cPacket*>(msg));
    EV_DETAIL << "pkt encapsulated, length: " << macPkt->getBitLength() << "\n";
    macQueue.push_back(macPkt);
    EV_DEBUG << "Max queue length: " << queueLength << ", packet put in queue"
              "\n  queue size: " << macQueue.size() << " macState: "
              << macState << endl;
    return true;
}

void XMacLayer::attachSignal(XMacFrame *mac, simtime_t_cref startTime)
{
    simtime_t duration = mac->getBitLength() / bitrate;
    mac->setDuration(duration);
}

/**
 * Change the color of the node for animation purposes.
 */
void XMacLayer::changeDisplayColor(XMAC_COLORS color)
{
    if (!animation)
        return;
    cDisplayString& dispStr = getContainingNode(this)->getDisplayString();
    //b=40,40,rect,black,black,2"
    if (color == GREEN)
        dispStr.setTagArg("b", 3, "green");
        //dispStr.parse("b=40,40,rect,green,green,2");
    if (color == BLUE)
        dispStr.setTagArg("b", 3, "blue");
                //dispStr.parse("b=40,40,rect,blue,blue,2");
    if (color == RED)
        dispStr.setTagArg("b", 3, "red");
                //dispStr.parse("b=40,40,rect,red,red,2");
    if (color == BLACK)
        dispStr.setTagArg("b", 3, "black");
                //dispStr.parse("b=40,40,rect,black,black,2");
    if (color == YELLOW)
        dispStr.setTagArg("b", 3, "yellow");
                //dispStr.parse("b=40,40,rect,yellow,yellow,2");
}

/**
 * Decapsulate a X-MAC frame.
 */
cPacket *XMacLayer::decapsMsg(XMacFrame *macPkt)
{
    cPacket *msg = macPkt->decapsulate();
    setUpControlInfo(msg, macPkt->getSrc());

    return msg;
}


/**
 * Attaches a "control info" (MacToNetw) structure (object) to the message pMsg.
 */
cObject *XMacLayer::setUpControlInfo(cMessage *const pMsg, const MACAddress& pSrcAddr)
{
    SimpleLinkLayerControlInfo *const cCtrlInfo = new SimpleLinkLayerControlInfo();
    cCtrlInfo->setSrc(pSrcAddr);
    pMsg->setControlInfo(cCtrlInfo);
    return cCtrlInfo;
}

} // namespace inet

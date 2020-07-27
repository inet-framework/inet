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

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/xmac/XMac.h"
#include "inet/linklayer/xmac/XMacHeader_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

using namespace physicallayer;

Define_Module(XMac);

/**
 * Initialize method of XMac. Init all parameters, schedule timers.
 */
void XMac::initialize(int stage)
{
    MacProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        animation     = par("animation");
        slotDuration  = par("slotDuration");
        bitrate       = par("bitrate");
        headerLength  = b(par("headerLength"));
        ctrlFrameLength  = b(par("ctrlFrameLength"));
        checkInterval = par("checkInterval");
        txPower       = par("txPower");
        useMacAcks    = par("useMACAcks");
        maxTxAttempts = par("maxTxAttempts");
        EV_DEBUG << "headerLength: " << headerLength << "ctrlFrameLength: " << ctrlFrameLength << ", bitrate: " << bitrate << endl;

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
        lastDataPktDestAddr = MacAddress::BROADCAST_ADDRESS;
        lastDataPktSrcAddr  = MacAddress::BROADCAST_ADDRESS;

        macState = INIT;
        txQueue = check_and_cast<queueing::IPacketQueue *>(getSubmodule("queue"));
        WATCH(macState);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        cModule *radioModule = getModuleFromPar<cModule>(par("radioModule"), this);
        radioModule->subscribe(IRadio::radioModeChangedSignal, this);
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);
        radio = check_and_cast<IRadio *>(radioModule);

        wakeup = new cMessage("wakeup", XMAC_WAKE_UP);

        data_timeout = new cMessage("data_timeout", XMAC_DATA_TIMEOUT);
        data_timeout->setSchedulingPriority(100);

        data_tx_over = new cMessage("data_tx_over", XMAC_DATA_TX_OVER);
        stop_preambles = new cMessage("stop_preambles", XMAC_STOP_PREAMBLES);
        send_preamble = new cMessage("send_preamble", XMAC_SEND_PREAMBLE);
        ack_tx_over = new cMessage("ack_tx_over", XMAC_ACK_TX_OVER);
        cca_timeout = new cMessage("cca_timeout", XMAC_CCA_TIMEOUT);
        cca_timeout->setSchedulingPriority(100);
        send_ack = new cMessage("send_ack", XMAC_SEND_ACK);
        start_xmac = new cMessage("start_xmac", XMAC_START_XMAC);
        ack_timeout = new cMessage("ack_timeout", XMAC_ACK_TIMEOUT);
        resend_data = new cMessage("resend_data", XMAC_RESEND_DATA);
        resend_data->setSchedulingPriority(100);
        switch_preamble_phase = new cMessage("switch_preamble_phase", SWITCH_PREAMBLE_PHASE);
        delay_for_ack_within_remote_rx = new cMessage("delay_for_ack_within_remote_rx", DELAY_FOR_ACK_WITHIN_REMOTE_RX);
        switching_done = new cMessage("switching_done", XMAC_SWITCHING_FINISHED);

        scheduleAfter(SIMTIME_ZERO, start_xmac);
    }
}

XMac::~XMac()
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

void XMac::finish()
{
    // record stats
    if (stats) {
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

void XMac::configureInterfaceEntry()
{
    MacAddress address = parseMacAddressParameter(par("address"));

    // data rate
    interfaceEntry->setDatarate(bitrate);

    // generate a link-layer address to be used as interface token for IPv6
    interfaceEntry->setMacAddress(address);
    interfaceEntry->setInterfaceToken(address.formInterfaceIdentifier());

    // capabilities
    interfaceEntry->setMtu(par("mtu"));
    interfaceEntry->setMulticast(false);
    interfaceEntry->setBroadcast(true);
}

/**
 * Check whether the queue is not full: if yes, print a warning and drop the
 * packet. Then initiate sending of the packet, if the node is sleeping. Do
 * nothing, if node is working.
 */
void XMac::handleUpperPacket(Packet *packet)
{
    encapsulate(packet);
    EV_DETAIL << "CSMA received a message from upper layer, name is " << packet->getName() << ", CInfo removed, mac addr=" << packet->peekAtFront<XMacHeaderBase>()->getDestAddr() << endl;
    EV_DETAIL << "pkt encapsulated, length: " << packet->getBitLength() << "\n";
    txQueue->pushPacket(packet);
    EV_DEBUG << "Max queue length: " << txQueue->getMaxNumPackets() << ", packet put in queue"
              "\n  queue size: " << txQueue->getNumPackets() << " macState: "
              << macState << endl;
    // force wakeup now
    if (!txQueue->isEmpty() && wakeup->isScheduled() && (macState == SLEEP)) {
        cancelEvent(wakeup);
        scheduleAfter(dblrand()*0.01f, wakeup);
    }
}

/**
 * Send one short preamble packet immediately.
 */
void XMac::sendPreamble(MacAddress preamble_address)
{
    //~ diff with XMAC, @ in preamble!
    auto preamble = makeShared<XMacControlFrame>();
    preamble->setSrcAddr(interfaceEntry->getMacAddress());
    preamble->setDestAddr(preamble_address);
    preamble->setChunkLength(ctrlFrameLength);
    preamble->setType(XMAC_PREAMBLE);
    auto packet = new Packet("Preamble", preamble);
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::xmac);
    attachSignal(packet, simTime());
    sendDown(packet);
    nbTxPreambles++;
}

/**
 * Send one short preamble packet immediately.
 */
void XMac::sendMacAck()
{
    auto ack = makeShared<XMacControlFrame>();
    ack->setSrcAddr(interfaceEntry->getMacAddress());
    //~ diff with XMAC, ack_preamble_based
    ack->setDestAddr(lastPreamblePktSrcAddr);
    ack->setChunkLength(ctrlFrameLength);
    ack->setType(XMAC_ACK);
    auto packet = new Packet("XMacAck", ack);
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::xmac);
    attachSignal(packet, simTime());
    sendDown(packet);
    nbTxAcks++;
}

/**
 * Handle own messages:
 */
void XMac::handleSelfMessage(cMessage *msg)
{
    MacAddress address = interfaceEntry->getMacAddress();

    switch (macState)
    {
    case INIT:
        if (msg->getKind() == XMAC_START_XMAC) {
            EV_DEBUG << "State INIT, message XMAC_START, new state SLEEP" << endl;
            changeDisplayColor(BLACK);
            radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
            macState = SLEEP;
            scheduleAfter(dblrand()*slotDuration, wakeup);
            return;
        }
        break;
    case SLEEP:
        if (msg->getKind() == XMAC_WAKE_UP) {
            EV_DEBUG << "node " << address << " : State SLEEP, message XMAC_WAKEUP, new state CCA, simTime " <<
                    simTime() << " to " << simTime() + 1.7f * checkInterval << endl;
            // this CCA is useful when in RX to detect preamble and has to make room for
            // 0.2f = Tx switch, 0.5f = Tx send_preamble, 1f = time_for_ack_back
            scheduleAfter(1.7f * checkInterval, cca_timeout);
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
            if (!txQueue->isEmpty())
            {
                radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                changeDisplayColor(YELLOW);
                macState = SEND_PREAMBLE;
                // We send the preamble for a whole SLOT duration :)
                scheduleAfter(slotDuration, stop_preambles);
                // if 0.2f * CI = 2ms to switch to TX -> has to be accounted for RX_preamble_detection
                scheduleAfter(0.2f * checkInterval, switch_preamble_phase);
                return;
            }
            // if anything to send, go back to sleep and wake up after a full period
            else
            {
                scheduleAfter(slotDuration, wakeup);
                macState = SLEEP;
                radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                changeDisplayColor(BLACK);
                return;
            }
        }
        // during CCA, we received a preamble. Go to state WAIT_DATA and
        // schedule the timeout.
        if (msg->getKind() == XMAC_PREAMBLE) {
            auto incoming_preamble = check_and_cast<Packet *>(msg)->peekAtFront<XMacControlFrame>();

            // preamble is for me
            if (incoming_preamble->getDestAddr() == address || incoming_preamble->getDestAddr().isBroadcast() || incoming_preamble->getDestAddr().isMulticast()) {
                cancelEvent(cca_timeout);
                nbRxPreambles++;
                EV << "node " << address << " : State CCA, message XMAC_PREAMBLE received, new state SEND_ACK" << endl;
                macState = SEND_ACK;
                lastPreamblePktSrcAddr = incoming_preamble->getSrcAddr();
                changeDisplayColor(YELLOW);
                radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
            }
            // the preamble is not for us
            else {
                EV << "node " << address << " : State CCA, message XMAC_PREAMBLE not for me." << endl;
                //~ better overhearing management? :)
                cancelEvent(cca_timeout);
                scheduleAfter(slotDuration, wakeup);
                macState = SLEEP;
                radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                changeDisplayColor(BLACK);
            }

            delete msg;
            return;
        }
        //in case we get an ACK, we simply dicard it, because it means the end
        //of another communication
        if (msg->getKind() == XMAC_ACK) {
            EV_DEBUG << "State CCA, message XMAC_ACK, new state CCA" << endl;
            delete msg;
            return;
        }
        // this case is very, very, very improbable, but let's do it.
        // if in CCA the node receives directly the data packet, accept it
        // even if we increased nbMissedAcks in state SLEEP
        if (msg->getKind() == XMAC_DATA) {
            auto incoming_data = check_and_cast<Packet *>(msg)->peekAtFront<XMacDataFrameHeader>();

            // packet is for me
            if (incoming_data->getDestAddr() == address) {
                EV << "node " << address << " : State CCA, received XMAC_DATA, accepting it." << endl;
                cancelEvent(cca_timeout);
                cancelEvent(switch_preamble_phase);
                cancelEvent(stop_preambles);
                macState = WAIT_DATA;
                scheduleAfter(SIMTIME_ZERO, msg);
            }
            return;
        }
        break;

    case SEND_PREAMBLE:
        if (msg->getKind() == SWITCH_PREAMBLE_PHASE) {
            //~ make room for preamble + time_for_ack_back, check_interval is 10ms by default (from NetworkXMAC.ini)
            // 0.5f* = 5ms
            if (radio->getRadioMode() == IRadio::RADIO_MODE_RECEIVER) {
                radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                changeDisplayColor(YELLOW);
                EV_DEBUG << "node " << address << " : preamble_phase tx, simTime = " << simTime() << endl;
                scheduleAfter(0.5f * checkInterval, switch_preamble_phase);
            }
            // 1.0f* = 10ms
            else if (radio->getRadioMode() == IRadio::RADIO_MODE_TRANSMITTER) {
                radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
                changeDisplayColor(GREEN);
                EV_DEBUG << "node " << address << " : preamble_phase rx, simTime = " << simTime() << endl;
                scheduleAfter(1.0f *checkInterval, switch_preamble_phase);
            }
            return;
        }
        // radio switch from above
        if (msg->getKind() == XMAC_SWITCHING_FINISHED) {
            if (radio->getRadioMode() == IRadio::RADIO_MODE_TRANSMITTER) {
                if (currentTxFrame == nullptr)
                    popTxQueue();
                auto pkt_preamble = currentTxFrame->peekAtFront<XMacHeaderBase>();
                sendPreamble(pkt_preamble->getDestAddr());
            }
            return;
        }
        // ack_rx within sending_preamble or preamble_timeout without an ACK
        if ((msg->getKind() == XMAC_ACK) || (msg->getKind() == XMAC_STOP_PREAMBLES)) {
            //~ ADDED THE SECOND CONDITION! :) if not, below
            if (msg->getKind() == XMAC_ACK) {
                delete msg;
                EV << "node " << address << " : State SEND_PREAMBLE, message XMAC_ACK, new state SEND_DATA" << endl;
            }
            else if (msg->getKind() == XMAC_STOP_PREAMBLES) {
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
            if (msg->getKind() == XMAC_DATA) {
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
        if (msg->getKind() == XMAC_STOP_PREAMBLES) {
            EV << "node " << address << " : State SEND_DATA, message XMAC_STOP_PREAMBLES" << endl;
            // send the data packet
            sendDataPacket();
            macState = WAIT_TX_DATA_OVER;
            return;
        }
        else if (msg->getKind() == XMAC_SWITCHING_FINISHED) {
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
        if (msg->getKind() == XMAC_DATA_TX_OVER) {
            EV_DEBUG << "node " << address << " : State WAIT_TX_DATA_OVER, message XMAC_DATA_TX_OVER, new state  SLEEP" << endl;
            // remove the packet just served from the queue
            deleteCurrentTxFrame();
            // if something in the queue, wakeup soon.
            if (!txQueue->isEmpty())
                scheduleAfter(dblrand()*checkInterval, wakeup);
            else
                scheduleAfter(slotDuration, wakeup);
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
        if (msg->getKind() == XMAC_PREAMBLE) {
            //nothing happens
            nbRxPreambles++;
            delete msg;
            return;
        }
        if (msg->getKind() == XMAC_ACK) {
            //nothing happens
            delete msg;
            return;
        }
        if (msg->getKind() == XMAC_DATA) {
            auto packet = check_and_cast<Packet *>(msg);
            auto mac = packet->peekAtFront<XMacDataFrameHeader>();
            const MacAddress& dest = mac->getDestAddr();

            if ((dest == address) || dest.isBroadcast() || dest.isMulticast()) {
                decapsulate(packet);
                sendUp(packet);
                nbRxDataPackets++;
                cancelEvent(data_timeout);

                // if something in the queue, wakeup soon.
                if (!txQueue->isEmpty())
                    scheduleAfter(dblrand()*checkInterval, wakeup);
                else
                    scheduleAfter(slotDuration, wakeup);
                macState = SLEEP;
                radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                changeDisplayColor(BLACK);

            }
            else {
                delete msg;
                msg = NULL;
                mac = NULL;
            }

            EV << "node " << address << " : State WAIT_DATA, message XMAC_DATA, new state  SLEEP" << endl;
            return;
        }
        // data does not arrives in time
        if (msg->getKind() == XMAC_DATA_TIMEOUT) {
            EV << "node " << address << " : State WAIT_DATA, message XMAC_DATA_TIMEOUT, new state SLEEP" << endl;
            // if something in the queue, wakeup soon.
            if (!txQueue->isEmpty())
                scheduleAfter(dblrand()*checkInterval, wakeup);
            else
                scheduleAfter(slotDuration, wakeup);
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
        if (msg->getKind() == XMAC_ACK_TX_OVER) {
            EV_DEBUG << "node " << address << " : State WAIT_ACK_TX, message XMAC_ACK_TX_OVER, new state WAIT_DATA" << endl;
            changeDisplayColor(GREEN);
            macState = WAIT_DATA;
            cancelEvent(cca_timeout);
            scheduleAfter((slotDuration / 2), data_timeout);
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
void XMac::handleLowerPacket(Packet *msg)
{
    if (msg->hasBitError()) {
        EV << "Received " << msg << " contains bit errors or collision, dropping it\n";
        nbRxBrokenDataPackets++;
        delete msg;
        return;
    }
    // simply pass the massage as self message, to be processed by the FSM.
    const auto& hdr = msg->peekAtFront<XMacHeaderBase>();
    msg->setKind(hdr->getType());
    handleSelfMessage(msg);
}

void XMac::sendDataPacket()
{
    nbTxDataPackets++;
    if (currentTxFrame == nullptr)
        popTxQueue();
    auto packet = currentTxFrame->dup();
    const auto& hdr = packet->peekAtFront<XMacHeaderBase>();
    lastDataPktDestAddr = hdr->getDestAddr();
    ASSERT(hdr->getType() == XMAC_DATA);
    attachSignal(packet, simTime());
    sendDown(packet);
}

/**
 * Handle transmission over messages: either send another preambles or the data
 * packet itself.
 */
void XMac::receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details)
{
    Enter_Method_Silent();
    if (signalID == IRadio::transmissionStateChangedSignal) {
        IRadio::TransmissionState newRadioTransmissionState = (IRadio::TransmissionState)value;
        if (transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING && newRadioTransmissionState == IRadio::TRANSMISSION_STATE_IDLE) {
            // Transmission of one packet is over
            if (macState == WAIT_TX_DATA_OVER) {
                scheduleAfter(SIMTIME_ZERO, data_tx_over);
            }
            if (macState == WAIT_ACK_TX) {
                scheduleAfter(SIMTIME_ZERO, ack_tx_over);
            }
        }
        transmissionState = newRadioTransmissionState;
    }
    else if (signalID ==IRadio::radioModeChangedSignal) {
        // Radio switching (to RX or TX) is over, ignore switching to SLEEP.
        if (macState == SEND_PREAMBLE) {
            scheduleAfter(SIMTIME_ZERO, switching_done);
        }
        else if (macState == SEND_ACK) {
            scheduleAfter(0.5f * checkInterval, delay_for_ack_within_remote_rx);
        }
        else if (macState == SEND_DATA) {
            scheduleAfter(SIMTIME_ZERO, switching_done);
        }
    }
}

void XMac::attachSignal(Packet *packet, simtime_t_cref startTime)
{
    simtime_t duration = packet->getBitLength() / bitrate;
    packet->setDuration(duration);
}

/**
 * Change the color of the node for animation purposes.
 */
void XMac::changeDisplayColor(XMAC_COLORS color)
{
    if (!animation)
        return;
    cDisplayString& dispStr = getContainingNode(this)->getDisplayString();
    switch (macState) {
        case INIT:
            dispStr.setTagArg("t", 0, "INIT");
            break;

        case SLEEP:
            dispStr.setTagArg("t", 0, "SLEEP");
            break;

        case CCA:
            dispStr.setTagArg("t", 0, "CCA");
            break;

        case SEND_ACK:
        case SEND_PREAMBLE:
        case SEND_DATA:
            dispStr.setTagArg("t", 0, "SEND");
            break;

        case WAIT_ACK:
        case WAIT_DATA:
        case WAIT_TX_DATA_OVER:
        case WAIT_ACK_TX:
            dispStr.setTagArg("t", 0, "WAIT");
            break;

        default:
            dispStr.setTagArg("t", 0, "");
            break;
    }
}


void XMac::decapsulate(Packet *packet)
{
    const auto& xmacHeader = packet->popAtFront<XMacDataFrameHeader>();
    packet->addTagIfAbsent<MacAddressInd>()->setSrcAddress(xmacHeader->getSrcAddr());
    packet->addTagIfAbsent<InterfaceInd>()->setInterfaceId(interfaceEntry->getInterfaceId());
    auto payloadProtocol = ProtocolGroup::ethertype.getProtocol(xmacHeader->getNetworkProtocol());
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(payloadProtocol);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(payloadProtocol);
    EV_DETAIL << " message decapsulated " << endl;
}

void XMac::encapsulate(Packet *packet)
{
    auto pkt = makeShared<XMacDataFrameHeader>();
    pkt->setChunkLength(headerLength);

    // copy dest address from the Control Info attached to the network
    // message by the network layer
    auto dest = packet->getTag<MacAddressReq>()->getDestAddress();
    EV_DETAIL << "CInfo removed, mac addr=" << dest << endl;
    pkt->setNetworkProtocol(ProtocolGroup::ethertype.getProtocolNumber(packet->getTag<PacketProtocolTag>()->getProtocol()));
    pkt->setDestAddr(dest);

    //delete the control info
    delete packet->removeControlInfo();

    //set the src address to own mac address (nic module getId())
    pkt->setSrcAddr(interfaceEntry->getMacAddress());

    pkt->setType(XMAC_DATA);
    packet->setKind(XMAC_DATA);

    //encapsulate the network packet
    packet->insertAtFront(pkt);
    packet->getTag<PacketProtocolTag>()->setProtocol(&Protocol::xmac);
    EV_DETAIL << "pkt encapsulated\n";
}

} // namespace inet

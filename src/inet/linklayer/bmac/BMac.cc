//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/common/INETUtils.h"
#include "inet/common/INETMath.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/bmac/BMac.h"
#include "inet/linklayer/bmac/BMacHeader_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {

using namespace physicallayer;

Define_Module(BMac);

void BMac::initialize(int stage)
{
    MacProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        animation = par("animation");
        slotDuration = par("slotDuration");
        bitrate = par("bitrate");
        headerLength = b(par("headerLength"));
        ctrlFrameLength = b(par("ctrlFrameLength"));
        checkInterval = par("checkInterval");
        useMacAcks = par("useMACAcks");
        maxTxAttempts = par("maxTxAttempts");
        EV_DETAIL << "headerLength: " << headerLength << ", bitrate: " << bitrate << endl;

        nbTxDataPackets = 0;
        nbTxPreambles = 0;
        nbRxDataPackets = 0;
        nbRxPreambles = 0;
        nbMissedAcks = 0;
        nbRecvdAcks = 0;
        nbDroppedDataPackets = 0;
        nbTxAcks = 0;

        txAttempts = 0;
        lastDataPktDestAddr = MacAddress::BROADCAST_ADDRESS;
        lastDataPktSrcAddr = MacAddress::BROADCAST_ADDRESS;

        macState = INIT;
        txQueue = check_and_cast<queueing::IPacketQueue *>(getSubmodule("queue"));
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        cModule *radioModule = getModuleFromPar<cModule>(par("radioModule"), this);
        radioModule->subscribe(IRadio::radioModeChangedSignal, this);
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);
        radio = check_and_cast<IRadio *>(radioModule);

        // init the dropped packet info
        WATCH(macState);

        wakeup = new cMessage("wakeup", BMAC_WAKE_UP);

        data_timeout = new cMessage("data_timeout", BMAC_DATA_TIMEOUT);
        data_timeout->setSchedulingPriority(100);

        data_tx_over = new cMessage("data_tx_over", BMAC_DATA_TX_OVER);

        stop_preambles = new cMessage("stop_preambles", BMAC_STOP_PREAMBLES);

        send_preamble = new cMessage("send_preamble", BMAC_SEND_PREAMBLE);

        ack_tx_over = new cMessage("ack_tx_over", BMAC_ACK_TX_OVER);

        cca_timeout = new cMessage("cca_timeout", BMAC_CCA_TIMEOUT);
        cca_timeout->setSchedulingPriority(100);

        send_ack = new cMessage("send_ack", BMAC_SEND_ACK);

        start_bmac = new cMessage("start_bmac", BMAC_START_BMAC);

        ack_timeout = new cMessage("ack_timeout", BMAC_ACK_TIMEOUT);

        resend_data = new cMessage("resend_data", BMAC_RESEND_DATA);
        resend_data->setSchedulingPriority(100);

        scheduleAfter(SIMTIME_ZERO, start_bmac);
    }
}

BMac::~BMac()
{
    cancelAndDelete(wakeup);
    cancelAndDelete(data_timeout);
    cancelAndDelete(data_tx_over);
    cancelAndDelete(stop_preambles);
    cancelAndDelete(send_preamble);
    cancelAndDelete(ack_tx_over);
    cancelAndDelete(cca_timeout);
    cancelAndDelete(send_ack);
    cancelAndDelete(start_bmac);
    cancelAndDelete(ack_timeout);
    cancelAndDelete(resend_data);
}

void BMac::finish()
{
    recordScalar("nbTxDataPackets", nbTxDataPackets);
    recordScalar("nbTxPreambles", nbTxPreambles);
    recordScalar("nbRxDataPackets", nbRxDataPackets);
    recordScalar("nbRxPreambles", nbRxPreambles);
    recordScalar("nbMissedAcks", nbMissedAcks);
    recordScalar("nbRecvdAcks", nbRecvdAcks);
    recordScalar("nbTxAcks", nbTxAcks);
    recordScalar("nbDroppedDataPackets", nbDroppedDataPackets);
    //recordScalar("timeSleep", timeSleep);
    //recordScalar("timeRX", timeRX);
    //recordScalar("timeTX", timeTX);
}

void BMac::configureInterfaceEntry()
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
void BMac::handleUpperPacket(Packet *packet)
{
    encapsulate(packet);
    txQueue->pushPacket(packet);
    EV_DETAIL << "Max queue length: " << txQueue->getMaxNumPackets() << ", packet put in queue\n"
              << "  queue size: " << txQueue->getNumPackets() << " macState: " << macState << endl;
    // force wakeup now
    if (!txQueue->isEmpty() && wakeup->isScheduled() && (macState == SLEEP)) {
        cancelEvent(wakeup);
        scheduleAfter(dblrand() * 0.1f, wakeup);
    }
}

/**
 * Send one short preamble packet immediately.
 */
void BMac::sendPreamble()
{
    auto preamble = makeShared<BMacControlFrame>();
    preamble->setSrcAddr(interfaceEntry->getMacAddress());
    preamble->setDestAddr(MacAddress::BROADCAST_ADDRESS);
    preamble->setChunkLength(ctrlFrameLength);

    //attach signal and send down
    auto packet = new Packet("Preamble");
    preamble->setType(BMAC_PREAMBLE);
    packet->insertAtFront(preamble);
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::bmac);
    attachSignal(packet);
    sendDown(packet);
    nbTxPreambles++;
}

/**
 * Send one short preamble packet immediately.
 */
void BMac::sendMacAck()
{
    auto ack = makeShared<BMacControlFrame>();
    ack->setSrcAddr(interfaceEntry->getMacAddress());
    ack->setDestAddr(lastDataPktSrcAddr);
    ack->setChunkLength(ctrlFrameLength);

    //attach signal and send down
    auto packet = new Packet("BMacAck");
    ack->setType(BMAC_ACK);
    packet->insertAtFront(ack);
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::bmac);
    attachSignal(packet);
    sendDown(packet);
    nbTxAcks++;
    //endSimulation();
}

/**
 * Handle own messages:
 * BMAC_WAKEUP: wake up the node, check the channel for some time.
 * BMAC_CHECK_CHANNEL: if the channel is free, check whether there is something
 * in the queue and switch the radio to TX. When switched to TX, the node will
 * start sending preambles for a full slot duration. If the channel is busy,
 * stay awake to receive message. Schedule a timeout to handle false alarms.
 * BMAC_SEND_PREAMBLES: sending of preambles over. Next time the data packet
 * will be send out (single one).
 * BMAC_TIMEOUT_DATA: timeout the node after a false busy channel alarm. Go
 * back to sleep.
 */
void BMac::handleSelfMessage(cMessage *msg)
{
    switch (macState) {
        case INIT:
            if (msg->getKind() == BMAC_START_BMAC) {
                EV_DETAIL << "State INIT, message BMAC_START, new state SLEEP" << endl;
                radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                macState = SLEEP;
                scheduleAfter(dblrand() * slotDuration, wakeup);
                return;
            }
            break;

        case SLEEP:
            if (msg->getKind() == BMAC_WAKE_UP) {
                EV_DETAIL << "State SLEEP, message BMAC_WAKEUP, new state CCA" << endl;
                scheduleAfter(checkInterval, cca_timeout);
                radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
                macState = CCA;
                return;
            }
            break;

        case CCA:
            if (msg->getKind() == BMAC_CCA_TIMEOUT) {
                // channel is clear
                // something waiting in eth queue?
                if (!txQueue->isEmpty()) {
                    EV_DETAIL << "State CCA, message CCA_TIMEOUT, new state"
                                 " SEND_PREAMBLE" << endl;
                    macState = SEND_PREAMBLE;
                    radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                    scheduleAfter(slotDuration, stop_preambles);
                    return;
                }
                // if not, go back to sleep and wake up after a full period
                else {
                    EV_DETAIL << "State CCA, message CCA_TIMEOUT, new state SLEEP"
                              << endl;
                    scheduleAfter(slotDuration, wakeup);
                    macState = SLEEP;
                    radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                    return;
                }
            }
            // during CCA, we received a preamble. Go to state WAIT_DATA and
            // schedule the timeout.
            if (msg->getKind() == BMAC_PREAMBLE) {
                nbRxPreambles++;
                EV_DETAIL << "State CCA, message BMAC_PREAMBLE received, new state"
                             " WAIT_DATA" << endl;
                macState = WAIT_DATA;
                cancelEvent(cca_timeout);
                scheduleAfter(slotDuration + checkInterval, data_timeout);
                delete msg;
                return;
            }
            // this case is very, very, very improbable, but let's do it.
            // if in CCA and the node receives directly the data packet, switch to
            // state WAIT_DATA and re-send the message
            if (msg->getKind() == BMAC_DATA) {
                nbRxDataPackets++;
                EV_DETAIL << "State CCA, message BMAC_DATA, new state WAIT_DATA"
                          << endl;
                macState = WAIT_DATA;
                cancelEvent(cca_timeout);
                scheduleAfter(slotDuration + checkInterval, data_timeout);
                scheduleAfter(SIMTIME_ZERO, msg);
                return;
            }
            //in case we get an ACK, we simply dicard it, because it means the end
            //of another communication
            if (msg->getKind() == BMAC_ACK) {
                EV_DETAIL << "State CCA, message BMAC_ACK, new state CCA" << endl;
                delete msg;
                return;
            }
            break;

        case SEND_PREAMBLE:
            if (msg->getKind() == BMAC_SEND_PREAMBLE) {
                EV_DETAIL << "State SEND_PREAMBLE, message BMAC_SEND_PREAMBLE, new"
                             " state SEND_PREAMBLE" << endl;
                sendPreamble();
                scheduleAfter(0.5f * checkInterval, send_preamble);
                macState = SEND_PREAMBLE;
                return;
            }
            // simply change the state to SEND_DATA
            if (msg->getKind() == BMAC_STOP_PREAMBLES) {
                EV_DETAIL << "State SEND_PREAMBLE, message BMAC_STOP_PREAMBLES, new"
                             " state SEND_DATA" << endl;
                macState = SEND_DATA;
                txAttempts = 1;
                return;
            }
            break;

        case SEND_DATA:
            if ((msg->getKind() == BMAC_SEND_PREAMBLE)
                || (msg->getKind() == BMAC_RESEND_DATA))
            {
                EV_DETAIL << "State SEND_DATA, message BMAC_SEND_PREAMBLE or"
                             " BMAC_RESEND_DATA, new state WAIT_TX_DATA_OVER" << endl;
                // send the data packet
                if (msg->getKind() == BMAC_SEND_PREAMBLE) {
                    popTxQueue();
                }
                ASSERT(currentTxFrame != nullptr);
                sendDataPacket();
                macState = WAIT_TX_DATA_OVER;
                return;
            }
            break;

        case WAIT_TX_DATA_OVER:
            if (msg->getKind() == BMAC_DATA_TX_OVER) {
                if ((useMacAcks) && !lastDataPktDestAddr.isBroadcast()) {
                    EV_DETAIL << "State WAIT_TX_DATA_OVER, message BMAC_DATA_TX_OVER,"
                                 " new state WAIT_ACK" << endl;
                    macState = WAIT_ACK;
                    radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
                    scheduleAfter(checkInterval, ack_timeout);
                }
                else {
                    EV_DETAIL << "State WAIT_TX_DATA_OVER, message BMAC_DATA_TX_OVER,"
                                 " new state  SLEEP" << endl;
                    deleteCurrentTxFrame();
                    // if something in the queue, wakeup soon.
                    if (!txQueue->isEmpty())
                        scheduleAfter(dblrand() * checkInterval, wakeup);
                    else
                        scheduleAfter(slotDuration, wakeup);
                    macState = SLEEP;
                    radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                }
                return;
            }
            break;

        case WAIT_ACK:
            if (msg->getKind() == BMAC_ACK_TIMEOUT) {
                // No ACK received. try again or drop.
                if (txAttempts < maxTxAttempts) {
                    EV_DETAIL << "State WAIT_ACK, message BMAC_ACK_TIMEOUT, new state"
                                 " SEND_DATA" << endl;
                    txAttempts++;
                    macState = SEND_PREAMBLE;
                    scheduleAfter(slotDuration, stop_preambles);
                    radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                }
                else {
                    EV_DETAIL << "State WAIT_ACK, message BMAC_ACK_TIMEOUT, new state"
                                 " SLEEP" << endl;
                    //drop the packet
                    emit(linkBrokenSignal, currentTxFrame);
                    PacketDropDetails details;
                    details.setReason(OTHER_PACKET_DROP);
                    dropCurrentTxFrame(details);

                    // if something in the queue, wakeup soon.
                    if (!txQueue->isEmpty())
                        scheduleAfter(dblrand() * checkInterval, wakeup);
                    else
                        scheduleAfter(slotDuration, wakeup);
                    macState = SLEEP;
                    radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                    nbMissedAcks++;
                }
                return;
            }
            //ignore and other packets
            if ((msg->getKind() == BMAC_DATA) || (msg->getKind() == BMAC_PREAMBLE)) {
                EV_DETAIL << "State WAIT_ACK, message BMAC_DATA or BMAC_PREMABLE, new"
                             " state WAIT_ACK" << endl;
                delete msg;
                return;
            }
            if (msg->getKind() == BMAC_ACK) {
                EV_DETAIL << "State WAIT_ACK, message BMAC_ACK" << endl;
                auto packet = check_and_cast<Packet *>(msg);
                const MacAddress src = packet->peekAtFront<BMacControlFrame>()->getSrcAddr();
                // the right ACK is received..
                EV_DETAIL << "We are waiting for ACK from : " << lastDataPktDestAddr
                          << ", and ACK came from : " << src << endl;
                if (src == lastDataPktDestAddr) {
                    EV_DETAIL << "New state SLEEP" << endl;
                    nbRecvdAcks++;
                    lastDataPktDestAddr = MacAddress::BROADCAST_ADDRESS;
                    cancelEvent(ack_timeout);
                    deleteCurrentTxFrame();
                    // if something in the queue, wakeup soon.
                    if (!txQueue->isEmpty())
                        scheduleAfter(dblrand() * checkInterval, wakeup);
                    else
                        scheduleAfter(slotDuration, wakeup);
                    macState = SLEEP;
                    radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                    lastDataPktDestAddr = MacAddress::BROADCAST_ADDRESS;
                }
                delete msg;
                return;
            }
            break;

        case WAIT_DATA:
            if (msg->getKind() == BMAC_PREAMBLE) {
                //nothing happens
                EV_DETAIL << "State WAIT_DATA, message BMAC_PREAMBLE, new state"
                             " WAIT_DATA" << endl;
                nbRxPreambles++;
                delete msg;
                return;
            }
            if (msg->getKind() == BMAC_ACK) {
                //nothing happens
                EV_DETAIL << "State WAIT_DATA, message BMAC_ACK, new state WAIT_DATA"
                          << endl;
                delete msg;
                return;
            }
            if (msg->getKind() == BMAC_DATA) {
                MacAddress address = interfaceEntry->getMacAddress();
                nbRxDataPackets++;
                auto packet = check_and_cast<Packet *>(msg);
                const auto bmacHeader = packet->peekAtFront<BMacDataFrameHeader>();
                const MacAddress& dest = bmacHeader->getDestAddr();
                const MacAddress& src = bmacHeader->getSrcAddr();
                if ((dest == address) || dest.isBroadcast()) {
                    EV_DETAIL << "Local delivery " << packet << endl;
                    decapsulate(packet);
                    sendUp(packet);
                }
                else {
                    EV_DETAIL << "Received " << packet << " is not for us, dropping frame." << endl;
                    PacketDropDetails details;
                    details.setReason(NOT_ADDRESSED_TO_US);
                    emit(packetDroppedSignal, msg, &details);
                    delete msg;
                    msg = nullptr;
                    packet = nullptr;
                }

                cancelEvent(data_timeout);
                if ((useMacAcks) && (dest == address)) {
                    EV_DETAIL << "State WAIT_DATA, message BMAC_DATA, new state"
                                 " SEND_ACK" << endl;
                    macState = SEND_ACK;
                    lastDataPktSrcAddr = src;
                    radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                }
                else {
                    EV_DETAIL << "State WAIT_DATA, message BMAC_DATA, new state SLEEP"
                              << endl;
                    // if something in the queue, wakeup soon.
                    if (!txQueue->isEmpty())
                        scheduleAfter(dblrand() * checkInterval, wakeup);
                    else
                        scheduleAfter(slotDuration, wakeup);
                    macState = SLEEP;
                    radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                }
                return;
            }
            if (msg->getKind() == BMAC_DATA_TIMEOUT) {
                EV_DETAIL << "State WAIT_DATA, message BMAC_DATA_TIMEOUT, new state"
                             " SLEEP" << endl;
                // if something in the queue, wakeup soon.
                if (!txQueue->isEmpty())
                    scheduleAfter(dblrand() * checkInterval, wakeup);
                else
                    scheduleAfter(slotDuration, wakeup);
                macState = SLEEP;
                radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                return;
            }
            break;

        case SEND_ACK:
            if (msg->getKind() == BMAC_SEND_ACK) {
                EV_DETAIL << "State SEND_ACK, message BMAC_SEND_ACK, new state"
                             " WAIT_ACK_TX" << endl;
                // send now the ack packet
                sendMacAck();
                macState = WAIT_ACK_TX;
                return;
            }
            break;

        case WAIT_ACK_TX:
            if (msg->getKind() == BMAC_ACK_TX_OVER) {
                EV_DETAIL << "State WAIT_ACK_TX, message BMAC_ACK_TX_OVER, new state"
                             " SLEEP" << endl;
                // ack sent, go to sleep now.
                // if something in the queue, wakeup soon.
                if (!txQueue->isEmpty())
                    scheduleAfter(dblrand() * checkInterval, wakeup);
                else
                    scheduleAfter(slotDuration, wakeup);
                macState = SLEEP;
                radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                lastDataPktSrcAddr = MacAddress::BROADCAST_ADDRESS;
                return;
            }
            break;
    }
    throw cRuntimeError("Undefined event of type %d in state %d (radio mode %d, radio reception state %d, radio transmission state %d)!",
            msg->getKind(), macState, radio->getRadioMode(), radio->getReceptionState(), radio->getTransmissionState());
}

/**
 * Handle BMAC preambles and received data packets.
 */
void BMac::handleLowerPacket(Packet *packet)
{
    if (packet->hasBitError()) {
        EV << "Received " << packet << " contains bit errors or collision, dropping it\n";
        PacketDropDetails details;
        details.setReason(INCORRECTLY_RECEIVED);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
        return;
    }
    else {
        const auto& hdr = packet->peekAtFront<BMacHeaderBase>();
        packet->setKind(hdr->getType());
        // simply pass the message as self message, to be processed by the FSM.
        handleSelfMessage(packet);
    }
}

void BMac::sendDataPacket()
{
    nbTxDataPackets++;

    Packet *pkt = currentTxFrame->dup();
    attachSignal(pkt);
    const auto& hdr = pkt->peekAtFront<BMacDataFrameHeader>();
    lastDataPktDestAddr = hdr->getDestAddr();
    ASSERT(hdr->getType() == BMAC_DATA);
    sendDown(pkt);
}

void BMac::receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details)
{
    Enter_Method_Silent();
    if (signalID == IRadio::radioModeChangedSignal) {
        IRadio::RadioMode radioMode = static_cast<IRadio::RadioMode>(value);
        if (radioMode == IRadio::RADIO_MODE_TRANSMITTER) {
            // we just switched to TX after CCA, so simply send the first
            // sendPremable self message
            if (macState == SEND_PREAMBLE)
                scheduleAfter(SIMTIME_ZERO, send_preamble);
            else if (macState == SEND_ACK)
                scheduleAfter(SIMTIME_ZERO, send_ack);
            // we were waiting for acks, but none came. we switched to TX and now
            // need to resend data
            else if (macState == SEND_DATA)
                scheduleAfter(SIMTIME_ZERO, resend_data);
        }
    }
    // Transmission of one packet is over
    else if (signalID == IRadio::transmissionStateChangedSignal) {
        IRadio::TransmissionState newRadioTransmissionState = static_cast<IRadio::TransmissionState>(value);
        if (transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING && newRadioTransmissionState == IRadio::TRANSMISSION_STATE_IDLE) {
            if (macState == WAIT_TX_DATA_OVER)
                scheduleAfter(SIMTIME_ZERO, data_tx_over);
            else if (macState == WAIT_ACK_TX)
                scheduleAfter(SIMTIME_ZERO, ack_tx_over);
        }
        transmissionState = newRadioTransmissionState;
    }
}

void BMac::attachSignal(Packet *macPkt)
{
    //calc signal duration
    simtime_t duration = macPkt->getBitLength() / bitrate;
    //create and initialize control info with new signal
    macPkt->setDuration(duration);
}

/**
 * Change the color of the node for animation purposes.
 */
void BMac::refreshDisplay() const
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

/*void BMac::changeMacState(States newState)
   {
    switch (macState)
    {
    case RX:
        timeRX += (simTime() - lastTime);
        break;
    case TX:
        timeTX += (simTime() - lastTime);
        break;
    case SLEEP:
        timeSleep += (simTime() - lastTime);
        break;
    case CCA:
        timeRX += (simTime() - lastTime);
    }
    lastTime = simTime();

    switch (newState)
    {
    case CCA:
        changeDisplayColor(GREEN);
        break;
    case TX:
        changeDisplayColor(BLUE);
        break;
    case SLEEP:
        changeDisplayColor(BLACK);
        break;
    case RX:
        changeDisplayColor(YELLOW);
        break;
    }

    macState = newState;
   }*/

void BMac::decapsulate(Packet *packet)
{
    const auto& bmacHeader = packet->popAtFront<BMacDataFrameHeader>();
    packet->addTagIfAbsent<MacAddressInd>()->setSrcAddress(bmacHeader->getSrcAddr());
    packet->addTagIfAbsent<InterfaceInd>()->setInterfaceId(interfaceEntry->getInterfaceId());
    auto payloadProtocol = ProtocolGroup::ethertype.getProtocol(bmacHeader->getNetworkProtocol());
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(payloadProtocol);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(payloadProtocol);
    EV_DETAIL << " message decapsulated " << endl;
}

void BMac::encapsulate(Packet *packet)
{
    auto pkt = makeShared<BMacDataFrameHeader>();
    pkt->setChunkLength(headerLength);

    pkt->setType(BMAC_DATA);
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

    //encapsulate the network packet
    packet->insertAtFront(pkt);
    EV_DETAIL << "pkt encapsulated\n";
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::bmac);
}

} // namespace inet


/*
 *  LMac.cc
 *
 *
 *  Created by Anna Foerster on 10/10/08.
 *  Copyright 2008 Universita della Svizzera Italiana. All rights reserved.
 *
 *  Converted to OMNeT++ 4 by Rudolf Hornig
 *  Converted to MiXiM by Kapourniotis Theodoros
 */

#include "inet/common/INETUtils.h"
#include "inet/common/INETMath.h"
#include "inet/common/FindModule.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/lmac/LMac.h"
#include "inet/linklayer/lmac/LMacHeader_m.h"
#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {

using namespace physicallayer;

Define_Module(LMac)

const MacAddress LMac::LMAC_NO_RECEIVER = MacAddress(-2);
const MacAddress LMac::LMAC_FREE_SLOT = MacAddress::BROADCAST_ADDRESS;

void LMac::initialize(int stage)
{
    MacProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        slotDuration = par("slotDuration");
        bitrate = par("bitrate");
        headerLength = b(par("headerLength"));
        EV << "headerLength is: " << headerLength << endl;
        ctrlFrameLength = b(par("ctrlFrameLength"));
        numSlots = par("numSlots");
        // the first N slots are reserved for mobile nodes to be able to function normally
        reservedMobileSlots = par("reservedMobileSlots");

        auto node = getContainingNode(this);
        myId = node->isVector() ? node->getIndex() : getId() % numSlots;

        macState = INIT;

        slotChange = new cOutVector("slotChange");

        // how long does it take to send/receive a control packet
        controlDuration = (double)(b(headerLength).get() + numSlots + 16) / (double)bitrate;     //FIXME replace 16 to a constant
        EV << "Control packets take : " << controlDuration << " seconds to transmit\n";

        txQueue = check_and_cast<queueing::IPacketQueue *>(getSubmodule("queue"));
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        cModule *radioModule = getModuleFromPar<cModule>(par("radioModule"), this);
        radioModule->subscribe(IRadio::radioModeChangedSignal, this);
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);
        radio = check_and_cast<IRadio *>(radioModule);

        WATCH(macState);

        EV_DETAIL << " slotDuration = " << slotDuration
                  << " controlDuration = " << controlDuration
                  << " numSlots = " << numSlots
                  << " bitrate = " << bitrate << endl;

        timeout = new cMessage("timeout", LMAC_TIMEOUT);
        sendData = new cMessage("sendData", LMAC_SEND_DATA);
        wakeup = new cMessage("wakeup", LMAC_WAKEUP);
        initChecker = new cMessage("setup phase", LMAC_SETUP_PHASE_END);
        checkChannel = new cMessage("checkchannel", LMAC_CHECK_CHANNEL);
        start_lmac = new cMessage("start_lmac", LMAC_START_LMAC);
        send_control = new cMessage("send_control", LMAC_SEND_CONTROL);

        scheduleAt(simTime(), start_lmac);
        EV_DETAIL << "My Mac address is" << interfaceEntry->getMacAddress() << " and my Id is " << myId << endl;
    }
}

LMac::~LMac()
{
    delete slotChange;
    cancelAndDelete(timeout);
    cancelAndDelete(wakeup);
    cancelAndDelete(checkChannel);
    cancelAndDelete(sendData);
    cancelAndDelete(initChecker);
    cancelAndDelete(start_lmac);
    cancelAndDelete(send_control);
}

void LMac::configureInterfaceEntry()
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
 * Check whether the queue is not full: if yes, print a warning and drop the packet.
 * Sending of messages is automatic.
 */
void LMac::handleUpperPacket(Packet *packet)
{
    encapsulate(packet);
    txQueue->enqueuePacket(packet);
    EV_DETAIL << "packet put in queue\n  queue size: " << txQueue->getNumPackets() << " macState: " << macState
              << "; mySlot is " << mySlot << "; current slot is " << currSlot << endl;
}

/**
 * Handle self messages:
 * LMAC_SETUP_PHASE_END: end of setup phase. Change slot duration to normal and start sending data packets. The slots of the nodes should be stable now.
 * LMAC_SEND_DATA: send the data packet.
 * LMAC_CHECK_CHANNEL: check the channel in own slot. If busy, change the slot. If not, send a control packet.
 * LMAC_WAKEUP: wake up the node and either check the channel before sending a control packet or wait for control packets.
 * LMAC_TIMEOUT: go back to sleep after nothing happened.
 */
void LMac::handleSelfMessage(cMessage *msg)
{
    MacAddress address = interfaceEntry->getMacAddress();

    switch (macState) {
        case INIT:
            if (msg->getKind() == LMAC_START_LMAC) {
                // the first 5 full slots we will be waking up every controlDuration to setup the network first
                // normal packets will be queued, but will be send only after the setup phase
                scheduleAt(slotDuration * 5 * numSlots, initChecker);
                EV << "Startup time =" << slotDuration * 5 * numSlots << endl;

                EV_DETAIL << "Scheduling the first wakeup at : " << slotDuration << endl;

                scheduleAt(slotDuration, wakeup);

                for (int i = 0; i < numSlots; i++) {
                    occSlotsDirect[i] = LMAC_FREE_SLOT;
                    occSlotsAway[i] = LMAC_FREE_SLOT;
                }

                if (myId >= reservedMobileSlots)
                    mySlot = ((int)FindModule<>::findHost(this)->getId()) % (numSlots - reservedMobileSlots);
                else
                    mySlot = myId;
                //occSlotsDirect[mySlot] = address;
                //occSlotsAway[mySlot] = address;
                currSlot = 0;

                EV_DETAIL << "ID: " << FindModule<>::findHost(this)->getId() << ". Picked random slot: " << mySlot << endl;

                macState = SLEEP;
                EV_DETAIL << "Old state: INIT, New state: SLEEP" << endl;
                SETUP_PHASE = true;
            }
            else {
                EV << "Unknown packet" << msg->getKind() << "in state" << macState << endl;
            }
            break;

        case SLEEP:
            if (msg->getKind() == LMAC_WAKEUP) {
                currSlot++;
                currSlot %= numSlots;
                EV_DETAIL << "New slot starting - No. " << currSlot << ", my slot is " << mySlot << endl;

                if (mySlot == currSlot) {
                    EV_DETAIL << "Waking up in my slot. Switch to RECV first to check the channel.\n";
                    macState = CCA;
                    radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
                    EV_DETAIL << "Old state: SLEEP, New state: CCA" << endl;

                    double small_delay = controlDuration * dblrand();
                    scheduleAt(simTime() + small_delay, checkChannel);
                    EV_DETAIL << "Checking for channel for " << small_delay << " time.\n";
                }
                else {
                    EV_DETAIL << "Waking up in a foreign slot. Ready to receive control packet.\n";
                    macState = WAIT_CONTROL;
                    radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
                    EV_DETAIL << "Old state: SLEEP, New state: WAIT_CONTROL" << endl;
                    if (!SETUP_PHASE) //in setup phase do not sleep
                        scheduleAt(simTime() + 2.f * controlDuration, timeout);
                }
                if (SETUP_PHASE) {
                    scheduleAt(simTime() + 2.f * controlDuration, wakeup);
                    EV_DETAIL << "setup phase slot duration:" << 2.f * controlDuration << "while controlduration is" << controlDuration << endl;
                }
                else
                    scheduleAt(simTime() + slotDuration, wakeup);
            }
            else if (msg->getKind() == LMAC_SETUP_PHASE_END) {
                EV_DETAIL << "Setup phase end. Start normal work at the next slot.\n";
                if (wakeup->isScheduled())
                    cancelEvent(wakeup);

                scheduleAt(simTime() + slotDuration, wakeup);

                SETUP_PHASE = false;
            }
            else {
                EV << "Unknown packet" << msg->getKind() << "in state" << macState << endl;
            }
            break;

        case CCA:
            if (msg->getKind() == LMAC_CHECK_CHANNEL) {
                // if the channel is clear, get ready for sending the control packet
                EV << "Channel is free, so let's prepare for sending.\n";

                macState = SEND_CONTROL;
                radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
                EV_DETAIL << "Old state: CCA, New state: SEND_CONTROL" << endl;
            }
            else if (msg->getKind() == LMAC_CONTROL) {
                auto packet = check_and_cast<Packet *>(msg);
                const auto& lmacHeader = packet->peekAtFront<LMacControlFrame>();
                const MacAddress& dest = lmacHeader->getDestAddr();
                EV_DETAIL << " I have received a control packet from src " << lmacHeader->getSrcAddr() << " and dest " << dest << ".\n";
                bool collision = false;
                // if we are listening to the channel and receive anything, there is a collision in the slot.
                if (checkChannel->isScheduled()) {
                    cancelEvent(checkChannel);
                    collision = true;
                }

                for (int s = 0; s < numSlots; s++) {
                    occSlotsAway[s] = lmacHeader->getOccupiedSlots(s);
                    EV_DETAIL << "Occupied slot " << s << ": " << occSlotsAway[s] << endl;
                    EV_DETAIL << "Occupied direct slot " << s << ": " << occSlotsDirect[s] << endl;
                }

                if (lmacHeader->getMySlot() > -1) {
                    // check first whether this address didn't have another occupied slot and free it again
                    for (int i = 0; i < numSlots; i++) {
                        if (occSlotsDirect[i] == lmacHeader->getSrcAddr())
                            occSlotsDirect[i] = LMAC_FREE_SLOT;
                        if (occSlotsAway[i] == lmacHeader->getSrcAddr())
                            occSlotsAway[i] = LMAC_FREE_SLOT;
                    }
                    occSlotsAway[lmacHeader->getMySlot()] = lmacHeader->getSrcAddr();
                    occSlotsDirect[lmacHeader->getMySlot()] = lmacHeader->getSrcAddr();
                }
                collision = collision || (lmacHeader->getMySlot() == mySlot);
                if (((mySlot > -1) && (lmacHeader->getOccupiedSlots(mySlot) > LMAC_FREE_SLOT) && (lmacHeader->getOccupiedSlots(mySlot) != address)) || collision) {
                    EV_DETAIL << "My slot is taken by " << lmacHeader->getOccupiedSlots(mySlot) << ". I need to change it.\n";
                    findNewSlot();
                    EV_DETAIL << "My new slot is " << mySlot << endl;
                }
                if (mySlot < 0) {
                    EV_DETAIL << "I don;t have a slot - try to find one.\n";
                    findNewSlot();
                }

                if (dest == address || dest.isBroadcast()) {
                    EV_DETAIL << "I need to stay awake.\n";
                    if (timeout->isScheduled())
                        cancelEvent(timeout);
                    macState = WAIT_DATA;
                    EV_DETAIL << "Old state: CCA, New state: WAIT_DATA" << endl;
                }
                else {
                    EV_DETAIL << "Incoming data packet not for me. Going back to sleep.\n";
                    macState = SLEEP;
                    EV_DETAIL << "Old state: CCA, New state: SLEEP" << endl;
                    radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                    if (timeout->isScheduled())
                        cancelEvent(timeout);
                }
                delete packet;
            }
            //probably it never happens
            else if (msg->getKind() == LMAC_DATA) {
                auto packet = check_and_cast<Packet *>(msg);
                const MacAddress& dest = packet->peekAtFront<LMacDataFrameHeader>()->getDestAddr();
                //bool collision = false;
                // if we are listening to the channel and receive anything, there is a collision in the slot.
                if (checkChannel->isScheduled()) {
                    cancelEvent(checkChannel);
                    //collision = true;
                }
                EV_DETAIL << " I have received a data packet.\n";
                if (dest == address || dest.isBroadcast()) {
                    EV_DETAIL << "sending pkt to upper...\n";
                    decapsulate(packet);
                    sendUp(packet);
                }
                else {
                    EV_DETAIL << "packet not for me, deleting...\n";
                    PacketDropDetails details;
                    details.setReason(NOT_ADDRESSED_TO_US);
                    emit(packetDroppedSignal, packet, &details);
                    delete packet;
                }
                // in any case, go back to sleep
                macState = SLEEP;
                EV_DETAIL << "Old state: CCA, New state: SLEEP" << endl;
                radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
            }
            else if (msg->getKind() == LMAC_SETUP_PHASE_END) {
                EV_DETAIL << "Setup phase end. Start normal work at the next slot.\n";
                if (wakeup->isScheduled())
                    cancelEvent(wakeup);

                scheduleAt(simTime() + slotDuration, wakeup);

                SETUP_PHASE = false;
            }
            else {
                EV << "Unknown packet" << msg->getKind() << "in state" << macState << endl;
            }
            break;

        case WAIT_CONTROL:
            if (msg->getKind() == LMAC_TIMEOUT) {
                EV_DETAIL << "Control timeout. Go back to sleep.\n";
                macState = SLEEP;
                EV_DETAIL << "Old state: WAIT_CONTROL, New state: SLEEP" << endl;
                radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
            }
            else if (msg->getKind() == LMAC_CONTROL) {
                auto packet = check_and_cast<Packet *>(msg);
                const auto& lmacHeader = packet->peekAtFront<LMacControlFrame>();
                const MacAddress& dest = lmacHeader->getDestAddr();
                EV_DETAIL << " I have received a control packet from src " << lmacHeader->getSrcAddr() << " and dest " << dest << ".\n";

                bool collision = false;

                // check first the slot assignment
                // copy the current slot assignment

                for (int s = 0; s < numSlots; s++) {
                    occSlotsAway[s] = lmacHeader->getOccupiedSlots(s);
                    EV_DETAIL << "Occupied slot " << s << ": " << occSlotsAway[s] << endl;
                    EV_DETAIL << "Occupied direct slot " << s << ": " << occSlotsDirect[s] << endl;
                }

                if (lmacHeader->getMySlot() > -1) {
                    // check first whether this address didn't have another occupied slot and free it again
                    for (int i = 0; i < numSlots; i++) {
                        if (occSlotsDirect[i] == lmacHeader->getSrcAddr())
                            occSlotsDirect[i] = LMAC_FREE_SLOT;
                        if (occSlotsAway[i] == lmacHeader->getSrcAddr())
                            occSlotsAway[i] = LMAC_FREE_SLOT;
                    }
                    occSlotsAway[lmacHeader->getMySlot()] = lmacHeader->getSrcAddr();
                    occSlotsDirect[lmacHeader->getMySlot()] = lmacHeader->getSrcAddr();
                }

                collision = collision || (lmacHeader->getMySlot() == mySlot);
                if (((mySlot > -1) && (lmacHeader->getOccupiedSlots(mySlot) > LMAC_FREE_SLOT) && (lmacHeader->getOccupiedSlots(mySlot) != address)) || collision) {
                    EV_DETAIL << "My slot is taken by " << lmacHeader->getOccupiedSlots(mySlot) << ". I need to change it.\n";
                    findNewSlot();
                    EV_DETAIL << "My new slot is " << mySlot << endl;
                }
                if (mySlot < 0) {
                    EV_DETAIL << "I don;t have a slot - try to find one.\n";
                    findNewSlot();
                }

                if (dest == address || dest.isBroadcast()) {
                    EV_DETAIL << "I need to stay awake.\n";
                    macState = WAIT_DATA;
                    EV_DETAIL << "Old state: WAIT_CONTROL, New state: WAIT_DATA" << endl;
                    if (timeout->isScheduled())
                        cancelEvent(timeout);
                }
                else {
                    EV_DETAIL << "Incoming data packet not for me. Going back to sleep.\n";
                    macState = SLEEP;
                    EV_DETAIL << "Old state: WAIT_CONTROL, New state: SLEEP" << endl;
                    radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                    if (timeout->isScheduled())
                        cancelEvent(timeout);
                }
                delete packet;
            }
            else if ((msg->getKind() == LMAC_WAKEUP)) {
                if (SETUP_PHASE == true)
                    EV_DETAIL << "End of setup-phase slot" << endl;
                else
                    EV_DETAIL << "Very unlikely transition";

                macState = SLEEP;
                EV_DETAIL << "Old state: WAIT_DATA, New state: SLEEP" << endl;
                scheduleAt(simTime(), wakeup);
            }
            else if (msg->getKind() == LMAC_SETUP_PHASE_END) {
                EV_DETAIL << "Setup phase end. Start normal work at the next slot.\n";
                if (wakeup->isScheduled())
                    cancelEvent(wakeup);

                scheduleAt(simTime() + slotDuration, wakeup);

                SETUP_PHASE = false;
            }
            else if (msg->getKind() == LMAC_DATA) {
                // TODO: review this delete, it's added to avoid a memory leak detected by Valgrind
                delete msg;
            }
            else {
                EV << "Unknown packet" << msg->getKind() << "in state" << macState << endl;
            }

            break;

        case SEND_CONTROL:
            if (msg->getKind() == LMAC_SEND_CONTROL) {
                if (!SETUP_PHASE && currentTxFrame == nullptr && !txQueue->isEmpty())
                    currentTxFrame = txQueue->dequeuePacket();
                // send first a control message, so that non-receiving nodes can switch off.
                EV << "Sending a control packet.\n";
                auto control = makeShared<LMacControlFrame>();
                if (currentTxFrame != nullptr && !SETUP_PHASE)
                    control->setDestAddr(currentTxFrame->peekAtFront<LMacHeaderBase>()->getDestAddr());
                else
                    control->setDestAddr(LMAC_NO_RECEIVER);

                control->setSrcAddr(address);
                control->setMySlot(mySlot);
                control->setChunkLength(ctrlFrameLength + b(numSlots));    //FIXME check it: add only 1 bit / slot?
                control->setOccupiedSlotsArraySize(numSlots);
                for (int i = 0; i < numSlots; i++)
                    control->setOccupiedSlots(i, occSlotsDirect[i]);

                Packet *packet = new Packet("Control");
                control->setType(LMAC_CONTROL);
                packet->insertAtFront(control);
                packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::lmac);
                sendDown(packet);
                if ((currentTxFrame != nullptr) && (!SETUP_PHASE))
                    scheduleAt(simTime() + controlDuration, sendData);
            }
            else if (msg->getKind() == LMAC_SEND_DATA) {
                // we should be in our own slot and the control packet should be already sent. receiving neighbors should wait for the data now.
                if (currSlot != mySlot) {
                    EV_DETAIL << "ERROR: Send data message received, but we are not in our slot!!! Repair.\n";
                    radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                    if (timeout->isScheduled())
                        cancelEvent(timeout);
                    return;
                }
                Packet *data = new Packet("Data");
                const auto& lmacHeader = currentTxFrame->removeAtFront<LMacDataFrameHeader>();
                data->insertAtBack(currentTxFrame->peekData());
                lmacHeader->setType(LMAC_DATA);
                lmacHeader->setMySlot(mySlot);
                lmacHeader->setOccupiedSlotsArraySize(numSlots);
                for (int i = 0; i < numSlots; i++)
                    lmacHeader->setOccupiedSlots(i, occSlotsDirect[i]);

                data->insertAtFront(lmacHeader);
                data->addTag<PacketProtocolTag>()->setProtocol(&Protocol::lmac);
                EV << "Sending down data packet\n";
                sendDown(data);
                deleteCurrentTxFrame();
                macState = SEND_DATA;
                EV_DETAIL << "Old state: SEND_CONTROL, New state: SEND_DATA" << endl;
            }
            else if (msg->getKind() == LMAC_SETUP_PHASE_END) {
                EV_DETAIL << "Setup phase end. Start normal work at the next slot.\n";
                if (wakeup->isScheduled())
                    cancelEvent(wakeup);

                scheduleAt(simTime() + slotDuration, wakeup);

                SETUP_PHASE = false;
            }
            else {
                EV << "Unknown packet" << msg->getKind() << "in state" << macState << endl;
            }
            break;

        case SEND_DATA:
            if (msg->getKind() == LMAC_WAKEUP) {
                throw cRuntimeError("I am still sending a message, while a new slot is starting!\n");
            }
            else {
                EV << "Unknown packet" << msg->getKind() << "in state" << macState << endl;
            }
            break;

        case WAIT_DATA:
            if (msg->getKind() == LMAC_DATA) {
                auto packet = check_and_cast<Packet *>(msg);
                const MacAddress& dest = packet->peekAtFront<LMacDataFrameHeader>()->getDestAddr();

                EV_DETAIL << " I have received a data packet.\n";
                if (dest == address || dest.isBroadcast()) {
                    EV_DETAIL << "sending pkt to upper...\n";
                    decapsulate(packet);
                    sendUp(packet);
                }
                else {
                    EV_DETAIL << "packet not for me, deleting...\n";
                    PacketDropDetails details;
                    details.setReason(NOT_ADDRESSED_TO_US);
                    emit(packetDroppedSignal, packet, &details);
                    delete packet;
                }
                // in any case, go back to sleep
                macState = SLEEP;
                EV_DETAIL << "Old state: WAIT_DATA, New state: SLEEP" << endl;
                radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                if (timeout->isScheduled())
                    cancelEvent(timeout);
            }
            else if (msg->getKind() == LMAC_WAKEUP) {
                macState = SLEEP;
                EV_DETAIL << "Unlikely transition. Old state: WAIT_DATA, New state: SLEEP" << endl;
                scheduleAt(simTime(), wakeup);
            }
            else {
                EV << "Unknown packet" << msg->getKind() << "in state" << macState << endl;
            }
            break;

        default:
            throw cRuntimeError("Unknown mac state: %d", macState);
            break;
    }
}

/**
 * Handle LMAC control packets and data packets. Recognize collisions, change own slot if necessary and remember who is using which slot.
 */
void LMac::handleLowerPacket(Packet *packet)
{
    if (packet->hasBitError()) {
        EV << "Received " << packet << " contains bit errors or collision, dropping it\n";
        PacketDropDetails details;
        details.setReason(INCORRECTLY_RECEIVED);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
        return;
    }
    // simply pass the massage as self message, to be processed by the FSM.
    const auto& hdr = packet->peekAtFront<LMacHeaderBase>();
    packet->setKind(hdr->getType());
    handleSelfMessage(packet);
}

/**
 * Handle transmission over messages: send the data packet or don;t do anyhting.
 */
void LMac::receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details)
{
    if (signalID == IRadio::transmissionStateChangedSignal) {
        IRadio::TransmissionState newRadioTransmissionState = static_cast<IRadio::TransmissionState>(value);
        if (transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING && newRadioTransmissionState == IRadio::TRANSMISSION_STATE_IDLE) {
            // if data is scheduled for transfer, don;t do anything.
            if (sendData->isScheduled()) {
                EV_DETAIL << " transmission of control packet over. data transfer will start soon." << endl;
            }
            else {
                EV_DETAIL << " transmission over. nothing else is scheduled, get back to sleep." << endl;
                macState = SLEEP;
                EV_DETAIL << "Old state: ?, New state: SLEEP" << endl;
                radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                if (timeout->isScheduled())
                    cancelEvent(timeout);
            }
        }
        transmissionState = newRadioTransmissionState;
    }
    else if (signalID == IRadio::radioModeChangedSignal) {
        IRadio::RadioMode radioMode = (IRadio::RadioMode)value;
        if (macState == SEND_CONTROL && radioMode == IRadio::RADIO_MODE_TRANSMITTER) {
            // we just switched to TX after CCA, so simply send the first sendPremable self message
            scheduleAt(simTime(), send_control);
        }
    }
}

/**
 * Try to find a new slot after collision. If not possible, set own slot to -1 (not able to send anything)
 */
void LMac::findNewSlot()
{
    // pick a random slot at the beginning and schedule the next wakeup
    // free the old one first
    int counter = 0;

    mySlot = intrand((numSlots - reservedMobileSlots));
    while ((occSlotsAway[mySlot] != LMAC_FREE_SLOT) && (counter < (numSlots - reservedMobileSlots))) {
        counter++;
        mySlot--;
        if (mySlot < 0)
            mySlot = (numSlots - reservedMobileSlots) - 1;
    }
    if (occSlotsAway[mySlot] != LMAC_FREE_SLOT) {
        EV << "ERROR: I cannot find a free slot. Cannot send data.\n";
        mySlot = -1;
    }
    else {
        EV << "ERROR: My new slot is : " << mySlot << endl;
    }
    EV << "ERROR: I needed to find new slot\n";
    slotChange->recordWithTimestamp(simTime(), FindModule<>::findHost(this)->getId() - 4);
}

void LMac::decapsulate(Packet *packet)
{
    const auto& lmacHeader = packet->popAtFront<LMacDataFrameHeader>();
    packet->addTagIfAbsent<MacAddressInd>()->setSrcAddress(lmacHeader->getSrcAddr());
    packet->addTagIfAbsent<InterfaceInd>()->setInterfaceId(interfaceEntry->getInterfaceId());
    auto payloadProtocol = ProtocolGroup::ethertype.getProtocol(lmacHeader->getNetworkProtocol());
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(payloadProtocol);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(payloadProtocol);
    EV_DETAIL << " message decapsulated " << endl;
}

/**
 * Encapsulates the received network-layer packet into a MacPkt and set all needed
 * header fields.
 */

void LMac::encapsulate(Packet *netwPkt)
{
    auto pkt = makeShared<LMacDataFrameHeader>();
    pkt->setChunkLength(headerLength);

    // copy dest address from the Control Info attached to the network
    // message by the network layer
    auto dest = netwPkt->getTag<MacAddressReq>()->getDestAddress();
    EV_DETAIL << "CInfo removed, mac addr=" << dest << endl;
    pkt->setDestAddr(dest);
    pkt->setNetworkProtocol(ProtocolGroup::ethertype.getProtocolNumber(netwPkt->getTag<PacketProtocolTag>()->getProtocol()));

    //delete the control info
    delete netwPkt->removeControlInfo();

    //set the src address to own mac address (nic module getId())
    pkt->setSrcAddr(interfaceEntry->getMacAddress());

    //encapsulate the network packet
    netwPkt->insertAtFront(pkt);
    EV_DETAIL << "pkt encapsulated\n";
}

void LMac::attachSignal(Packet *macPkt)
{
    //calc signal duration
    simtime_t duration = macPkt->getBitLength() / bitrate;
    //create and initialize control info with new signal
    macPkt->setDuration(duration);
}

} // namespace inet


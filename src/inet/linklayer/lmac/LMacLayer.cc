/*
 *  LMACLayer.cc
 *
 *
 *  Created by Anna Foerster on 10/10/08.
 *  Copyright 2008 Universita della Svizzera Italiana. All rights reserved.
 *
 *  Converted to OMNeT++ 4 by Rudolf Hornig
 *  Converted to MiXiM by Kapourniotis Theodoros
 */

#include "inet/linklayer/lmac/LMacLayer.h"

#include "inet/common/INETUtils.h"
#include "inet/common/INETMath.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/lmac/LMacFrame_m.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/linklayer/common/SimpleLinkLayerControlInfo.h"
#include "inet/common/FindModule.h"

namespace inet {

Define_Module(LMacLayer)

#define myId    (getParentModule()->getParentModule()->getIndex())

const MACAddress LMacLayer::LMAC_NO_RECEIVER = MACAddress(-2);
const MACAddress LMacLayer::LMAC_FREE_SLOT = MACAddress::BROADCAST_ADDRESS;

void LMacLayer::initialize(int stage)
{
    MACProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        queueLength = par("queueLength");
        slotDuration = par("slotDuration");
        bitrate = par("bitrate");
        headerLength = par("headerLength");
        EV << "headerLength is: " << headerLength << endl;
        numSlots = par("numSlots");
        // the first N slots are reserved for mobile nodes to be able to function normally
        reservedMobileSlots = par("reservedMobileSlots");

        EV_DETAIL << "My Mac address is" << address << " and my Id is " << myId << endl;

        macState = INIT;

        slotChange = new cOutVector("slotChange");

        // how long does it take to send/receive a control packet
        controlDuration = ((double)headerLength + (double)numSlots + 16) / (double)bitrate;
        EV << "Control packets take : " << controlDuration << " seconds to transmit\n";

        initializeMACAddress();
        registerInterface();

        cModule *radioModule = getModuleFromPar<cModule>(par("radioModule"), this);
        radioModule->subscribe(IRadio::radioModeChangedSignal, this);
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);
        radio = check_and_cast<IRadio *>(radioModule);

        WATCH(macState);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        //int channel;
        //channel = hasPar("defaultChannel") ? par("defaultChannel") : 0;

        EV_DETAIL << "queueLength = " << queueLength
                  << " slotDuration = " << slotDuration
                  << " controlDuration = " << controlDuration
                  << " numSlots = " << numSlots
                  << " bitrate = " << bitrate << endl;

        timeout = new cMessage("timeout");
        timeout->setKind(LMAC_TIMEOUT);

        sendData = new cMessage("sendData");
        sendData->setKind(LMAC_SEND_DATA);

        wakeup = new cMessage("wakeup");
        wakeup->setKind(LMAC_WAKEUP);

        initChecker = new cMessage("setup phase");
        initChecker->setKind(LMAC_SETUP_PHASE_END);

        checkChannel = new cMessage("checkchannel");
        checkChannel->setKind(LMAC_CHECK_CHANNEL);

        start_lmac = new cMessage("start_lmac");
        start_lmac->setKind(LMAC_START_LMAC);

        send_control = new cMessage("send_control");
        send_control->setKind(LMAC_SEND_CONTROL);

        scheduleAt(0.0, start_lmac);
    }
}

LMacLayer::~LMacLayer()
{
    delete slotChange;
    cancelAndDelete(timeout);
    cancelAndDelete(wakeup);
    cancelAndDelete(checkChannel);
    cancelAndDelete(sendData);
    cancelAndDelete(initChecker);
    cancelAndDelete(start_lmac);
    cancelAndDelete(send_control);

    for (auto & elem : macQueue) {
        delete (elem);
    }
    macQueue.clear();
}

void LMacLayer::initializeMACAddress()
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

InterfaceEntry *LMacLayer::createInterfaceEntry()
{
    InterfaceEntry *e = new InterfaceEntry(this);

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
 * Check whether the queue is not full: if yes, print a warning and drop the packet.
 * Sending of messages is automatic.
 */
void LMacLayer::handleUpperPacket(cPacket *msg)
{
    LMacFrame *mac = static_cast<LMacFrame *>(encapsMsg(static_cast<cPacket *>(msg)));

    // message has to be queued if another message is waiting to be send
    // or if we are already trying to send another message

    if (macQueue.size() <= queueLength) {
        macQueue.push_back(mac);
        EV_DETAIL << "packet put in queue\n  queue size: " << macQueue.size() << " macState: " << macState
                  << "; mySlot is " << mySlot << "; current slot is " << currSlot << endl;
        ;
    }
    else {
        // queue is full, message has to be deleted
        EV_DETAIL << "New packet arrived, but queue is FULL, so new packet is deleted\n";
        delete mac;
        EV_DETAIL << "ERROR: Queue is full, forced to delete.\n";
    }
}

/**
 * Handle self messages:
 * LMAC_SETUP_PHASE_END: end of setup phase. Change slot duration to normal and start sending data packets. The slots of the nodes should be stable now.
 * LMAC_SEND_DATA: send the data packet.
 * LMAC_CHECK_CHANNEL: check the channel in own slot. If busy, change the slot. If not, send a control packet.
 * LMAC_WAKEUP: wake up the node and either check the channel before sending a control packet or wait for control packets.
 * LMAC_TIMEOUT: go back to sleep after nothing happened.
 */
void LMacLayer::handleSelfMessage(cMessage *msg)
{
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
                LMacFrame *const mac = static_cast<LMacFrame *>(msg);
                const MACAddress& dest = mac->getDestAddr();
                EV_DETAIL << " I have received a control packet from src " << mac->getSrcAddr() << " and dest " << dest << ".\n";
                bool collision = false;
                // if we are listening to the channel and receive anything, there is a collision in the slot.
                if (checkChannel->isScheduled()) {
                    cancelEvent(checkChannel);
                    collision = true;
                }

                for (int s = 0; s < numSlots; s++) {
                    occSlotsAway[s] = mac->getOccupiedSlots(s);
                    EV_DETAIL << "Occupied slot " << s << ": " << occSlotsAway[s] << endl;
                    EV_DETAIL << "Occupied direct slot " << s << ": " << occSlotsDirect[s] << endl;
                }

                if (mac->getMySlot() > -1) {
                    // check first whether this address didn't have another occupied slot and free it again
                    for (int i = 0; i < numSlots; i++) {
                        if (occSlotsDirect[i] == mac->getSrcAddr())
                            occSlotsDirect[i] = LMAC_FREE_SLOT;
                        if (occSlotsAway[i] == mac->getSrcAddr())
                            occSlotsAway[i] = LMAC_FREE_SLOT;
                    }
                    occSlotsAway[mac->getMySlot()] = mac->getSrcAddr();
                    occSlotsDirect[mac->getMySlot()] = mac->getSrcAddr();
                }
                collision = collision || (mac->getMySlot() == mySlot);
                if (((mySlot > -1) && (mac->getOccupiedSlots(mySlot) > LMAC_FREE_SLOT) && (mac->getOccupiedSlots(mySlot) != address)) || collision) {
                    EV_DETAIL << "My slot is taken by " << mac->getOccupiedSlots(mySlot) << ". I need to change it.\n";
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
                delete mac;
            }
            //probably it never happens
            else if (msg->getKind() == LMAC_DATA) {
                LMacFrame *const mac = static_cast<LMacFrame *>(msg);
                const MACAddress& dest = mac->getDestAddr();
                //bool collision = false;
                // if we are listening to the channel and receive anything, there is a collision in the slot.
                if (checkChannel->isScheduled()) {
                    cancelEvent(checkChannel);
                    //collision = true;
                }
                EV_DETAIL << " I have received a data packet.\n";
                if (dest == address || dest.isBroadcast()) {
                    EV_DETAIL << "sending pkt to upper...\n";
                    sendUp(decapsMsg(mac));
                }
                else {
                    EV_DETAIL << "packet not for me, deleting...\n";
                    delete mac;
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
                LMacFrame *const mac = static_cast<LMacFrame *>(msg);
                const MACAddress& dest = mac->getDestAddr();
                EV_DETAIL << " I have received a control packet from src " << mac->getSrcAddr() << " and dest " << dest << ".\n";

                bool collision = false;

                // check first the slot assignment
                // copy the current slot assignment

                for (int s = 0; s < numSlots; s++) {
                    occSlotsAway[s] = mac->getOccupiedSlots(s);
                    EV_DETAIL << "Occupied slot " << s << ": " << occSlotsAway[s] << endl;
                    EV_DETAIL << "Occupied direct slot " << s << ": " << occSlotsDirect[s] << endl;
                }

                if (mac->getMySlot() > -1) {
                    // check first whether this address didn't have another occupied slot and free it again
                    for (int i = 0; i < numSlots; i++) {
                        if (occSlotsDirect[i] == mac->getSrcAddr())
                            occSlotsDirect[i] = LMAC_FREE_SLOT;
                        if (occSlotsAway[i] == mac->getSrcAddr())
                            occSlotsAway[i] = LMAC_FREE_SLOT;
                    }
                    occSlotsAway[mac->getMySlot()] = mac->getSrcAddr();
                    occSlotsDirect[mac->getMySlot()] = mac->getSrcAddr();
                }

                collision = collision || (mac->getMySlot() == mySlot);
                if (((mySlot > -1) && (mac->getOccupiedSlots(mySlot) > LMAC_FREE_SLOT) && (mac->getOccupiedSlots(mySlot) != address)) || collision) {
                    EV_DETAIL << "My slot is taken by " << mac->getOccupiedSlots(mySlot) << ". I need to change it.\n";
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
                delete mac;
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
            else {
                EV << "Unknown packet" << msg->getKind() << "in state" << macState << endl;
            }

            break;

        case SEND_CONTROL:

            if (msg->getKind() == LMAC_SEND_CONTROL) {
                // send first a control message, so that non-receiving nodes can switch off.
                EV << "Sending a control packet.\n";
                LMacFrame *control = new LMacFrame();
                control->setKind(LMAC_CONTROL);
                if ((macQueue.size() > 0) && !SETUP_PHASE)
                    control->setDestAddr((macQueue.front())->getDestAddr());
                else
                    control->setDestAddr(LMAC_NO_RECEIVER);

                control->setSrcAddr(address);
                control->setMySlot(mySlot);
                control->setBitLength(headerLength + numSlots);
                control->setOccupiedSlotsArraySize(numSlots);
                for (int i = 0; i < numSlots; i++)
                    control->setOccupiedSlots(i, occSlotsDirect[i]);

                sendDown(control);
                if ((macQueue.size() > 0) && (!SETUP_PHASE))
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
                LMacFrame *data = macQueue.front()->dup();
                data->setKind(LMAC_DATA);
                data->setMySlot(mySlot);
                data->setOccupiedSlotsArraySize(numSlots);
                for (int i = 0; i < numSlots; i++)
                    data->setOccupiedSlots(i, occSlotsDirect[i]);

                EV << "Sending down data packet\n";
                sendDown(data);
                delete macQueue.front();
                macQueue.pop_front();
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
                LMacFrame *const mac = static_cast<LMacFrame *>(msg);
                const MACAddress& dest = mac->getDestAddr();

                EV_DETAIL << " I have received a data packet.\n";
                if (dest == address || dest.isBroadcast()) {
                    EV_DETAIL << "sending pkt to upper...\n";
                    sendUp(decapsMsg(mac));
                }
                else {
                    EV_DETAIL << "packet not for me, deleting...\n";
                    delete mac;
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
void LMacLayer::handleLowerPacket(cPacket *msg)
{
    // simply pass the massage as self message, to be processed by the FSM.
    handleSelfMessage(msg);
}

/**
 * Handle transmission over messages: send the data packet or don;t do anyhting.
 */
void LMacLayer::receiveSignal(cComponent *source, simsignal_t signalID, long value DETAILS_ARG)
{
    if (signalID == IRadio::transmissionStateChangedSignal) {
        IRadio::TransmissionState newRadioTransmissionState = (IRadio::TransmissionState)value;
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
void LMacLayer::findNewSlot()
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

cPacket *LMacLayer::decapsMsg(LMacFrame *msg)
{
    cPacket *m = msg->decapsulate();
    setUpControlInfo(m, msg->getSrcAddr());
    // delete the macPkt
    delete msg;
    EV_DETAIL << " message decapsulated " << endl;
    return m;
}

/**
 * Encapsulates the received network-layer packet into a MacPkt and set all needed
 * header fields.
 */

LMacFrame *LMacLayer::encapsMsg(cPacket *netwPkt)
{
    LMacFrame *pkt = new LMacFrame(netwPkt->getName(), netwPkt->getKind());
    pkt->setBitLength(headerLength);

    // copy dest address from the Control Info attached to the network
    // message by the network layer
    IMACProtocolControlInfo *cInfo = check_and_cast<IMACProtocolControlInfo *>(netwPkt->removeControlInfo());
    EV_DETAIL << "CInfo removed, mac addr=" << cInfo->getDestinationAddress() << endl;
    pkt->setDestAddr(cInfo->getDestinationAddress());

    //delete the control info
    delete cInfo;

    //set the src address to own mac address (nic module getId())
    pkt->setSrcAddr(address);

    //encapsulate the network packet
    pkt->encapsulate(netwPkt);
    EV_DETAIL << "pkt encapsulated\n";

    return pkt;
}

void LMacLayer::flushQueue()
{
    // TODO:
    macQueue.clear();
}

void LMacLayer::clearQueue()
{
    macQueue.clear();
}

void LMacLayer::attachSignal(LMacFrame *macPkt)
{
    //calc signal duration
    simtime_t duration = macPkt->getBitLength() / bitrate;
    //create and initialize control info with new signal
    macPkt->setDuration(duration);
}

/**
 * Attaches a "control info" (MacToNetw) structure (object) to the message pMsg.
 */
cObject *LMacLayer::setUpControlInfo(cMessage *const pMsg, const MACAddress& pSrcAddr)
{
    SimpleLinkLayerControlInfo *const cCtrlInfo = new SimpleLinkLayerControlInfo();
    cCtrlInfo->setSrc(pSrcAddr);
    cCtrlInfo->setInterfaceId(interfaceEntry->getInterfaceId());
    pMsg->setControlInfo(cCtrlInfo);
    return cCtrlInfo;
}

} // namespace inet


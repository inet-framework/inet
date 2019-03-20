//
// Copyright (C) 2016 OpenSim Ltd.
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

#include "inet/lora/lorabase/LoRaMac.h"

#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/common/UserPriority.h"
#include "inet/linklayer/csmaca/CsmaCaMac.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/lora/lorabase/LoRaTagInfo_m.h"

namespace inet {
namespace lora {

Define_Module(LoRaMac);

LoRaMac::~LoRaMac()
{
    cancelAndDelete(endTransmission);
    cancelAndDelete(endReception);
    cancelAndDelete(droppedPacket);
    cancelAndDelete(endDelay_1);
    cancelAndDelete(endListening_1);
    cancelAndDelete(endDelay_2);
    cancelAndDelete(endListening_2);
    cancelAndDelete(mediumStateChange);
}

/****************************************************************
 * Initialization functions.
 */
void LoRaMac::initialize(int stage)
{
    MacProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        EV << "Initializing stage 0\n";

        maxQueueSize = par("maxQueueSize");
        headerLength = par("headerLength");
        ackLength = par("ackLength");
        ackTimeout = par("ackTimeout");
        retryLimit = par("retryLimit");

        waitDelay1Time = 1;
        listening1Time = 1;
        waitDelay2Time = 1;
        listening2Time = 1;

        const char *addressString = par("address");
        if (!strcmp(addressString, "auto")) {
            // assign automatic address
            address = MacAddress::generateAutoAddress();
            // change module parameter from "auto" to concrete address
            par("address").setStringValue(address.str().c_str());
        }
        else
            address.setAddress(addressString);

        // subscribe for the information of the carrier sense
        cModule *radioModule = getModuleFromPar<cModule>(par("radioModule"), this);
        radioModule->subscribe(IRadio::receptionStateChangedSignal, this);
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);
        radioModule->subscribe(LoRaRadio::droppedPacket, this);
        radio = check_and_cast<IRadio *>(radioModule);

        // initialize self messages
        endTransmission = new cMessage("Transmission");
        endReception = new cMessage("Reception");
        droppedPacket = new cMessage("Dropped Packet");
        endDelay_1 = new cMessage("Delay_1");
        endListening_1 = new cMessage("Listening_1");
        endDelay_2 = new cMessage("Delay_2");
        endListening_2 = new cMessage("Listening_2");
        mediumStateChange = new cMessage("MediumStateChange");

        // set up internal queue
        transmissionQueue.setName("transmissionQueue");

        // state variables
        fsm.setName("LoRaMac State Machine");
        backoffPeriod = -1;
        retryCounter = 0;

        // sequence number for messages
        sequenceNumber = 0;

        // statistics
        numRetry = 0;
        numSentWithoutRetry = 0;
        numGivenUp = 0;
        numCollision = 0;
        numSent = 0;
        numReceived = 0;
        numSentBroadcast = 0;
        numReceivedBroadcast = 0;

        // initialize watches
        WATCH(fsm);
        WATCH(backoffPeriod);
        WATCH(retryCounter);
        WATCH(numRetry);
        WATCH(numSentWithoutRetry);
        WATCH(numGivenUp);
        WATCH(numCollision);
        WATCH(numSent);
        WATCH(numReceived);
        WATCH(numSentBroadcast);
        WATCH(numReceivedBroadcast);
    }
    else if (stage == INITSTAGE_LINK_LAYER)
        radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
}

void LoRaMac::finish()
{
    recordScalar("numRetry", numRetry);
    recordScalar("numSentWithoutRetry", numSentWithoutRetry);
    recordScalar("numGivenUp", numGivenUp);
    //recordScalar("numCollision", numCollision);
    recordScalar("numSent", numSent);
    recordScalar("numReceived", numReceived);
    recordScalar("numSentBroadcast", numSentBroadcast);
    recordScalar("numReceivedBroadcast", numReceivedBroadcast);
}

void LoRaMac::configureInterfaceEntry()
{
    //InterfaceEntry *e = new InterfaceEntry(this);

    // data rate
    interfaceEntry->setDatarate(bitrate);

    // capabilities
    interfaceEntry->setMtu(par("mtu"));
    interfaceEntry->setMulticast(true);
    interfaceEntry->setBroadcast(true);
    interfaceEntry->setPointToPoint(false);
}

/****************************************************************
 * Message handling functions.
 */
void LoRaMac::handleSelfMessage(cMessage *msg)
{
    EV << "received self message: " << msg << endl;
    handleWithFsm(msg);
}

void LoRaMac::handleUpperMessage(cMessage *msg)
{
    if(fsm.getState() != IDLE) {
            error(fsm.getStateName());
            error("Wrong, it should not happen");
    }
    auto pkt = check_and_cast<Packet *>(msg);

    pkt->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::lora);
//    LoRaMacControlInfo *cInfo = check_and_cast<LoRaMacControlInfo *>(msg->getControlInfo());
    auto pktEncap = encapsulate(pkt);

    const auto &frame = pktEncap->peekAtFront<LoRaMacFrame>();


    EV << "frame " << pktEncap << " received from higher layer, receiver = " << frame->getReceiverAddress() << endl;
    transmissionQueue.insert(pktEncap);
    handleWithFsm(pktEncap);
}

void LoRaMac::handleLowerMessage(cMessage *msg)
{
    if( (fsm.getState() == RECEIVING_1) || (fsm.getState() == RECEIVING_2)) handleWithFsm(msg);
    else delete msg;
}

void LoRaMac::handleWithFsm(cMessage *msg)
{
    Ptr<LoRaMacFrame>frame = nullptr;

    auto pkt = dynamic_cast<Packet *>(msg);
    if (pkt) {
        const auto &chunk = pkt->peekAtFront<Chunk>();
        frame = dynamicPtrCast<LoRaMacFrame>(constPtrCast<Chunk>(chunk));
    }
    FSMA_Switch(fsm)
    {
        FSMA_State(IDLE)
        {
            FSMA_Enter(turnOffReceiver());
            FSMA_Event_Transition(Idle-Transmit,
                                  isUpperMessage(msg),
                                  TRANSMIT,
            );
        }
        FSMA_State(TRANSMIT)
        {
            FSMA_Enter(sendDataFrame(getCurrentTransmission()));
            FSMA_Event_Transition(Transmit-Wait_Delay_1,
                                  msg == endTransmission,
                                  WAIT_DELAY_1,
                finishCurrentTransmission();
                numSent++;
            );
        }
        FSMA_State(WAIT_DELAY_1)
        {
            FSMA_Enter(turnOffReceiver());
            FSMA_Event_Transition(Wait_Delay_1-Listening_1,
                                  msg == endDelay_1 || endDelay_1->isScheduled() == false,
                                  LISTENING_1,
            );
        }
        FSMA_State(LISTENING_1)
        {
            FSMA_Enter(turnOnReceiver());
            FSMA_Event_Transition(Listening_1-Wait_Delay_2,
                                  msg == endListening_1 || endListening_1->isScheduled() == false,
                                  WAIT_DELAY_2,
            );
            FSMA_Event_Transition(Listening_1-Receiving1,
                                  msg == mediumStateChange && isReceiving(),
                                  RECEIVING_1,
            );
        }
        FSMA_State(RECEIVING_1)
        {
            FSMA_Event_Transition(Receive-Unicast-Not-For,
                                  isLowerMessage(msg) && !isForUs(frame),
                                  LISTENING_1,
                delete pkt;
            );
            FSMA_Event_Transition(Receive-Unicast,
                                  isLowerMessage(msg) && isForUs(frame),
                                  IDLE,
                sendUp(decapsulate(pkt));
                numReceived++;
                cancelEvent(endListening_1);
                cancelEvent(endDelay_2);
                cancelEvent(endListening_2);
            );
            FSMA_Event_Transition(Receive-BelowSensitivity,
                                  msg == droppedPacket,
                                  LISTENING_1,
            );

        }
        FSMA_State(WAIT_DELAY_2)
        {
            FSMA_Enter(turnOffReceiver());
            FSMA_Event_Transition(Wait_Delay_2-Listening_2,
                                  msg == endDelay_2 || endDelay_2->isScheduled() == false,
                                  LISTENING_2,
            );
        }
        FSMA_State(LISTENING_2)
        {
            FSMA_Enter(turnOnReceiver());
            FSMA_Event_Transition(Listening_2-idle,
                                  msg == endListening_2 || endListening_2->isScheduled() == false,
                                  IDLE,
            );
            FSMA_Event_Transition(Listening_2-Receiving2,
                                  msg == mediumStateChange && isReceiving(),
                                  RECEIVING_2,
            );
        }
        FSMA_State(RECEIVING_2)
        {
            FSMA_Event_Transition(Receive2-Unicast-Not-For,
                                  isLowerMessage(msg) && !isForUs(frame),
                                  LISTENING_2,
                delete pkt;
            );
            FSMA_Event_Transition(Receive2-Unicast,
                                  isLowerMessage(msg) && isForUs(frame),
                                  IDLE,
                sendUp(pkt);
                numReceived++;
                cancelEvent(endListening_2);
            );
            FSMA_Event_Transition(Receive2-BelowSensitivity,
                                  msg == droppedPacket,
                                  LISTENING_2,
            );

        }
    }
}

void LoRaMac::receiveSignal(cComponent *source, simsignal_t signalID, long value, cObject *details)
{
    Enter_Method_Silent();
    if (signalID == IRadio::receptionStateChangedSignal) {
        IRadio::ReceptionState newRadioReceptionState = (IRadio::ReceptionState)value;
        if (receptionState == IRadio::RECEPTION_STATE_RECEIVING) {
            radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
        }
        receptionState = newRadioReceptionState;
        handleWithFsm(mediumStateChange);
    }
    else if (signalID == LoRaRadio::droppedPacket) {
        radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
        handleWithFsm(droppedPacket);
    }
    else if (signalID == IRadio::transmissionStateChangedSignal) {
        IRadio::TransmissionState newRadioTransmissionState = (IRadio::TransmissionState)value;
        if (transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING && newRadioTransmissionState == IRadio::TRANSMISSION_STATE_IDLE) {
            handleWithFsm(endTransmission);
            radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
        }
        transmissionState = newRadioTransmissionState;
    }
}

Packet *LoRaMac::encapsulate(Packet *msg)
{
    auto frame = makeShared<LoRaMacFrame>();
    frame->setChunkLength(B(headerLength));
    msg->setArrival(msg->getArrivalModuleId(), msg->getArrivalGateId());
    auto tag = msg->getTag<LoRaTag>();

    frame->setTransmitterAddress(address);
    frame->setLoRaTP(tag->getPower().get());
    frame->setLoRaCF(tag->getCarrierFrequency());
    frame->setLoRaSF(tag->getSpreadFactor());
    frame->setLoRaBW(tag->getBandwidth());
    frame->setLoRaCR(tag->getCodeRendundance());
    frame->setSequenceNumber(sequenceNumber);
    frame->setReceiverAddress(MacAddress::BROADCAST_ADDRESS);

    ++sequenceNumber;
    //frame->setLoRaUseHeader(cInfo->getLoRaUseHeader());
    frame->setLoRaUseHeader(tag->getUseHeader());

    msg->insertAtFront(frame);

    return msg;
}

Packet *LoRaMac::decapsulate(Packet *frame)
{

    auto loraHeader = frame->popAtFront<LoRaMacFrame>();
    frame->addTagIfAbsent<MacAddressInd>()->setSrcAddress(loraHeader->getTransmitterAddress());
    frame->addTagIfAbsent<MacAddressInd>()->setDestAddress(loraHeader->getReceiverAddress());
    frame->addTagIfAbsent<InterfaceInd>()->setInterfaceId(interfaceEntry->getInterfaceId());
    auto payloadProtocol = ProtocolGroup::ethertype.getProtocol(loraHeader->getNetworkProtocol());
    frame->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(payloadProtocol);
    frame->addTagIfAbsent<PacketProtocolTag>()->setProtocol(payloadProtocol);

    return frame;
}

/****************************************************************
 * Frame sender functions.
 */
void LoRaMac::sendDataFrame(Packet *frameToSend)
{
    EV << "sending Data frame\n";
    radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);

    auto frameCopy = frameToSend->dup();

    //LoRaMacControlInfo *ctrl = new LoRaMacControlInfo();
    //ctrl->setSrc(frameCopy->getTransmitterAddress());
    //ctrl->setDest(frameCopy->getReceiverAddress());
//    frameCopy->setControlInfo(ctrl);
    auto macHeader = frameCopy->peekAtFront<LoRaMacFrame>();

    auto macAddressInd = frameCopy->addTagIfAbsent<MacAddressInd>();
    macAddressInd->setSrcAddress(macHeader->getTransmitterAddress());
    macAddressInd->setDestAddress(macHeader->getReceiverAddress());

    //frameCopy->addTag<PacketProtocolTag>()->setProtocol(&Protocol::lora);

    sendDown(frameCopy);
}

void LoRaMac::sendAckFrame()
{


    auto frameToAck = static_cast<Packet *>(endSifs->getContextPointer());
    endSifs->setContextPointer(nullptr);
    auto macHeader = makeShared<CsmaCaMacAckHeader>();
    macHeader->setReceiverAddress(MacAddress(frameToAck->peekAtFront<LoRaMacFrame>()->getTransmitterAddress().getInt()));

    EV << "sending Ack frame\n";
    //auto macHeader = makeShared<CsmaCaMacAckHeader>();
    macHeader->setChunkLength(B(ackLength));
    auto frame = new Packet("CsmaAck");
    frame->insertAtFront(macHeader);
    frame->addTag<PacketProtocolTag>()->setProtocol(&Protocol::lora);
    radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);

    auto macAddressInd = frame->addTagIfAbsent<MacAddressInd>();
    macAddressInd->setSrcAddress(macHeader->getTransmitterAddress());
    macAddressInd->setDestAddress(macHeader->getReceiverAddress());

    sendDown(frame);
}

/****************************************************************
 * Helper functions.
 */
void LoRaMac::finishCurrentTransmission()
{
    scheduleAt(simTime() + waitDelay1Time, endDelay_1);
    scheduleAt(simTime() + waitDelay1Time + listening1Time, endListening_1);
    scheduleAt(simTime() + waitDelay1Time + listening1Time + waitDelay2Time, endDelay_2);
    scheduleAt(simTime() + waitDelay1Time + listening1Time + waitDelay2Time + listening2Time, endListening_2);
    popTransmissionQueue();
}

Packet *LoRaMac::getCurrentTransmission()
{
    return static_cast<Packet*>(transmissionQueue.front());
}

void LoRaMac::popTransmissionQueue()
{
    EV << "dropping frame from transmission queue\n";
    delete transmissionQueue.pop();
    if (queueModule) {
        // tell queue module that we've become idle
        EV << "requesting another frame from queue module\n";
        queueModule->requestPacket();
    }
}

bool LoRaMac::isReceiving()
{
    return radio->getReceptionState() == IRadio::RECEPTION_STATE_RECEIVING;
}

bool LoRaMac::isAck(const Ptr<const LoRaMacFrame> &frame)
{
    return false;//dynamic_cast<LoRaMacFrame *>(frame);
}

bool LoRaMac::isBroadcast(const Ptr<const LoRaMacFrame> &frame)
{
    return frame->getReceiverAddress().isBroadcast();
}

bool LoRaMac::isForUs(const Ptr<const LoRaMacFrame> &frame)
{
    return frame->getReceiverAddress() == address;
}

void LoRaMac::turnOnReceiver()
{
    LoRaRadio *loraRadio;
    loraRadio = check_and_cast<LoRaRadio *>(radio);
    loraRadio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
}

void LoRaMac::turnOffReceiver()
{
    LoRaRadio *loraRadio;
    loraRadio = check_and_cast<LoRaRadio *>(radio);
    loraRadio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
}

MacAddress LoRaMac::getAddress()
{
    return address;
}

}
} // namespace inet

//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/base/EthernetMacBase.h"

#include <stdlib.h>

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/StringFormat.h"
#include "inet/common/checksum/EthernetCRC.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/linklayer/common/EtherType_m.h"
#include "inet/linklayer/ethernet/common/Ethernet.h"
#include "inet/linklayer/ethernet/common/EthernetControlFrame_m.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/physicallayer/wired/ethernet/EthernetPhyHeader_m.h"
#include "inet/physicallayer/wired/ethernet/EthernetSignal_m.h"
#include "inet/queueing/function/PacketComparatorFunction.h"

namespace inet {

const double EthernetMacBase::SPEED_OF_LIGHT_IN_CABLE = 200000000.0;

const EthernetMacBase::EtherDescr EthernetMacBase::nullEtherDescr = {
    0.0,
    0.0,
    0,
    B(0),
    B(0),
    B(0),
    0.0,
    0.0
};

const EthernetMacBase::EtherDescr EthernetMacBase::etherDescrs[NUM_OF_ETHERDESCRS] = {
    {
        ETHERNET_TXRATE,
        0.5 / ETHERNET_TXRATE,
        0,
        B(0),
        MIN_ETHERNET_FRAME_BYTES,
        MIN_ETHERNET_FRAME_BYTES,
        512 / ETHERNET_TXRATE,
        2500 /*m*/ / SPEED_OF_LIGHT_IN_CABLE
    },
    {
        FAST_ETHERNET_TXRATE,
        0.5 / FAST_ETHERNET_TXRATE,
        0,
        B(0),
        MIN_ETHERNET_FRAME_BYTES,
        MIN_ETHERNET_FRAME_BYTES,
        512 / FAST_ETHERNET_TXRATE,
        250 /*m*/ / SPEED_OF_LIGHT_IN_CABLE
    },
    {
        GIGABIT_ETHERNET_TXRATE,
        0.5 / GIGABIT_ETHERNET_TXRATE,
        MAX_PACKETBURST,
        GIGABIT_MAX_BURST_BYTES,
        GIGABIT_MIN_FRAME_BYTES_WITH_EXT,
        MIN_ETHERNET_FRAME_BYTES,
        4096 / GIGABIT_ETHERNET_TXRATE,
        250 /*m*/ / SPEED_OF_LIGHT_IN_CABLE
    },
    {
        TWOANDHALFGIGABIT_ETHERNET_TXRATE,
        0.5 / TWOANDHALFGIGABIT_ETHERNET_TXRATE,
        0,
        B(0),
        B(-1), // half-duplex is not supported
        B(0),
        0.0,
        0.0
    },
    {
        FIVEGIGABIT_ETHERNET_TXRATE,
        0.5 / FIVEGIGABIT_ETHERNET_TXRATE,
        0,
        B(0),
        B(-1), // half-duplex is not supported
        B(0),
        0.0,
        0.0
    },
    {
        FAST_GIGABIT_ETHERNET_TXRATE,
        0.5 / FAST_GIGABIT_ETHERNET_TXRATE,
        0,
        B(0),
        B(-1), // half-duplex is not supported
        B(0),
        0.0,
        0.0
    },
    {
        TWENTYFIVE_GIGABIT_ETHERNET_TXRATE,
        0.5 / TWENTYFIVE_GIGABIT_ETHERNET_TXRATE,
        0,
        B(0),
        B(-1), // half-duplex is not supported
        B(0),
        0.0,
        0.0
    },
    {
        FOURTY_GIGABIT_ETHERNET_TXRATE,
        0.5 / FOURTY_GIGABIT_ETHERNET_TXRATE,
        0,
        B(0),
        B(-1), // half-duplex is not supported
        B(0),
        0.0,
        0.0
    },
    {
        HUNDRED_GIGABIT_ETHERNET_TXRATE,
        0.5 / HUNDRED_GIGABIT_ETHERNET_TXRATE,
        0,
        B(0),
        B(-1), // half-duplex is not supported
        B(0),
        0.0,
        0.0
    },
    {
        TWOHUNDRED_GIGABIT_ETHERNET_TXRATE,
        0.5 / TWOHUNDRED_GIGABIT_ETHERNET_TXRATE,
        0,
        B(0),
        B(-1), // half-duplex is not supported
        B(0),
        0.0,
        0.0
    },
    {
        FOURHUNDRED_GIGABIT_ETHERNET_TXRATE,
        0.5 / FOURHUNDRED_GIGABIT_ETHERNET_TXRATE,
        0,
        B(0),
        B(-1), // half-duplex is not supported
        B(0),
        0.0,
        0.0
    }
};

static int compareEthernetFrameType(Packet *a, Packet *b)
{
    const auto& ah = a->peekAtFront<EthernetMacHeader>();
    const auto& bh = b->peekAtFront<EthernetMacHeader>();
    int ac = (ah->getTypeOrLength() == ETHERTYPE_FLOW_CONTROL) ? 0 : 1;
    int bc = (bh->getTypeOrLength() == ETHERTYPE_FLOW_CONTROL) ? 0 : 1;
    return ac - bc;
}

Register_Packet_Comparator_Function(EthernetFrameTypeComparator, compareEthernetFrameType);

simsignal_t EthernetMacBase::rxPkOkSignal = registerSignal("rxPkOk");
simsignal_t EthernetMacBase::txPausePkUnitsSignal = registerSignal("txPausePkUnits");
simsignal_t EthernetMacBase::rxPausePkUnitsSignal = registerSignal("rxPausePkUnits");

simsignal_t EthernetMacBase::transmissionStateChangedSignal = registerSignal("transmissionStateChanged");
simsignal_t EthernetMacBase::receptionStateChangedSignal = registerSignal("receptionStateChanged");

EthernetMacBase::EthernetMacBase()
{
    lastTxFinishTime = -1.0; // never equals to current simtime
    curEtherDescr = &nullEtherDescr;
}

EthernetMacBase::~EthernetMacBase()
{
    delete curTxSignal;
    cancelAndDelete(endTxTimer);
    cancelAndDelete(endIfgTimer);
    cancelAndDelete(endPauseTimer);
}

void EthernetMacBase::initialize(int stage)
{
    MacProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        fcsMode = parseFcsMode(par("fcsMode"));
        physInGate = gate("phys$i");
        physOutGate = gate("phys$o");
        lowerLayerInGateId = physInGate->getId();
        lowerLayerOutGateId = physOutGate->getId();
        transmissionChannel = nullptr;
        txQueue = getQueue(gate(upperLayerInGateId));

        initializeFlags();

        initializeStatistics();

        lastTxFinishTime = -1.0; // not equals with current simtime.

        // initialize self messages
        endTxTimer = new cMessage("EndTransmission", ENDTRANSMISSION);
        endIfgTimer = new cMessage("EndIFG", ENDIFG);
        endPauseTimer = new cMessage("EndPause", ENDPAUSE);

        // initialize states
        transmitState = TX_IDLE_STATE;
        receiveState = RX_IDLE_STATE;

        // initialize pause
        pauseUnitsRequested = 0;

        subscribe(POST_MODEL_CHANGE, this);

        WATCH(transmitState);
        WATCH(receiveState);
        WATCH(connected);
        WATCH(frameBursting);
        WATCH(promiscuous);
        WATCH(pauseUnitsRequested);
    }
}

void EthernetMacBase::initializeFlags()
{
    displayStringTextFormat = par("displayStringTextFormat");
    sendRawBytes = par("sendRawBytes");
    duplexMode = true;

    // initialize connected flag
    connected = physOutGate->getPathEndGate()->isConnected() && physInGate->getPathStartGate()->isConnected();

    if (!connected)
        EV_WARN << "MAC not connected to a network.\n";

    // initialize promiscuous flag
    promiscuous = par("promiscuous");

    frameBursting = false;
}

void EthernetMacBase::initializeStatistics()
{
    numFramesSent = numFramesReceivedOK = numBytesSent = numBytesReceivedOK = 0;
    numFramesPassedToHL = numDroppedBitError = numDroppedNotForUs = 0;
    numFramesFromHL = numDroppedIfaceDown = numDroppedPkFromHLIfaceDown = 0;
    numPauseFramesRcvd = numPauseFramesSent = 0;

    WATCH(numFramesSent);
    WATCH(numFramesReceivedOK);
    WATCH(numBytesSent);
    WATCH(numBytesReceivedOK);
    WATCH(numFramesFromHL);
    WATCH(numDroppedPkFromHLIfaceDown);
    WATCH(numDroppedIfaceDown);
    WATCH(numDroppedBitError);
    WATCH(numDroppedNotForUs);
    WATCH(numFramesPassedToHL);
    WATCH(numPauseFramesRcvd);
    WATCH(numPauseFramesSent);
}

void EthernetMacBase::configureNetworkInterface()
{

    // MTU: typical values are 576 (Internet de facto), 1500 (Ethernet-friendly),
    // 4000 (on some point-to-point links), 4470 (Cisco routers default, FDDI compatible)
    networkInterface->setMtu(par("mtu"));

    // capabilities
    networkInterface->setMulticast(true);
    networkInterface->setBroadcast(true);
}

void EthernetMacBase::handleStartOperation(LifecycleOperation *operation)
{
    networkInterface->setState(NetworkInterface::State::UP);
    initializeFlags();
    readChannelParameters(true);
}

void EthernetMacBase::handleStopOperation(LifecycleOperation *operation)
{
    if (currentTxFrame != nullptr || !txQueue->isEmpty()) {
        networkInterface->setState(NetworkInterface::State::GOING_DOWN);
        delayActiveOperationFinish(par("stopOperationTimeout"));
    }
    else {
        networkInterface->setCarrier(false);
        networkInterface->setState(NetworkInterface::State::DOWN);
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
    }
}

void EthernetMacBase::handleCrashOperation(LifecycleOperation *operation)
{
//    clearQueue();
    connected = false;
    networkInterface->setCarrier(false);
    processConnectDisconnect();
    networkInterface->setState(NetworkInterface::State::DOWN);
}

// TODO this method should be renamed and called where processing is finished on the current frame (i.e. curTxFrame becomes nullptr)
void EthernetMacBase::processAtHandleMessageFinished()
{
    if (operationalState == State::STOPPING_OPERATION) {
        if (currentTxFrame == nullptr && txQueue->isEmpty()) {
            EV << "Ethernet Queue is empty, MAC stopped\n";
            connected = false;
            networkInterface->setCarrier(false);
            processConnectDisconnect();
            networkInterface->setState(NetworkInterface::State::DOWN);
            startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
        }
    }
}

void EthernetMacBase::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    MacProtocolBase::receiveSignal(source, signalID, obj, details);

    if (signalID == POST_MODEL_CHANGE) {
        if (auto gcobj = dynamic_cast<cPostPathCreateNotification *>(obj)) {
            if ((physOutGate == gcobj->pathStartGate) || (physInGate == gcobj->pathEndGate))
                refreshConnection();
        }
        else if (auto gcobj = dynamic_cast<cPostPathCutNotification *>(obj)) {
            if ((physOutGate == gcobj->pathStartGate) || (physInGate == gcobj->pathEndGate))
                refreshConnection();
        }
        else if (transmissionChannel && dynamic_cast<cPostParameterChangeNotification *>(obj)) { // note: we are subscribed to the channel object too
            cPostParameterChangeNotification *gcobj = static_cast<cPostParameterChangeNotification *>(obj);
            if (transmissionChannel == gcobj->par->getOwner())
                refreshConnection();
        }
    }
}

void EthernetMacBase::processConnectDisconnect()
{
    if (!connected) {
        cancelEvent(endIfgTimer);
        cancelEvent(endPauseTimer);

        if (curTxSignal) {
#if OMNETPP_BUILDNUM < 2001
            if (getSimulation()->getSimulationStage() == STAGE(EVENT) && physOutGate->getPathEndGate()->isConnected()) {
#else
            if (getSimulation()->getStage() == STAGE(EVENT) && physOutGate->getPathEndGate()->isConnected()) {
#endif
                ASSERT(endTxTimer->isScheduled());
                curTxSignal->setDuration(endTxTimer->getArrivalTime() - curTxSignal->getCreationTime());
                simtime_t duration = simTime() - curTxSignal->getCreationTime(); // TODO save and use start tx time
                cutEthernetSignalEnd(curTxSignal, duration);
                emit(transmissionEndedSignal, curTxSignal);
                send(curTxSignal, SendOptions().finishTx(curTxSignal->getId()), physOutGate);
            }
            else
                delete curTxSignal;
            curTxSignal = nullptr;
            cancelEvent(endTxTimer);
        }
        ASSERT(!endTxTimer->isScheduled());

        if (currentTxFrame) {
            EV_DETAIL << "Interface is not connected, dropping packet " << currentTxFrame << endl;
            numDroppedPkFromHLIfaceDown++;
            PacketDropDetails details;
            details.setReason(INTERFACE_DOWN);
            dropCurrentTxFrame(details);
            lastTxFinishTime = -1.0; // so that it never equals to the current simtime, used for Burst mode detection.
        }

        // Clear queue
        while (canDequeuePacket()) {
            Packet *msg = dequeuePacket();
            EV_DETAIL << "Interface is not connected, dropping packet " << msg << endl;
            numDroppedPkFromHLIfaceDown++;
            PacketDropDetails details;
            details.setReason(INTERFACE_DOWN);
            emit(packetDroppedSignal, msg, &details);
            delete msg;
        }

        changeTransmissionState(TX_IDLE_STATE); // FIXME replace status to OFF
        changeReceptionState(RX_IDLE_STATE);
    }
    // FIXME when connect, set statuses to RECONNECT or IDLE
}

void EthernetMacBase::encapsulate(Packet *frame)
{
    auto phyHeader = makeShared<EthernetPhyHeader>();
    frame->insertAtFront(phyHeader);
    frame->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetPhy);
}

void EthernetMacBase::decapsulate(Packet *packet)
{
    auto phyHeader = packet->popAtFront<EthernetPhyHeader>();
    ASSERT(packet->getDataLength() >= MIN_ETHERNET_FRAME_BYTES);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);
}

// FIXME should use it in EthernetCsmaMac, EthernetMac, etc. modules. But should not use it in EtherBus, EthernetHub.
bool EthernetMacBase::verifyCrcAndLength(Packet *packet)
{
    EV_STATICCONTEXT;

    auto ethHeader = packet->peekAtFront<EthernetMacHeader>(); // FIXME can I use any flags?
    const auto& ethTrailer = packet->peekAtBack<EthernetFcs>(ETHER_FCS_BYTES); // FIXME can I use any flags?

    switch (ethTrailer->getFcsMode()) {
        case FCS_DECLARED_CORRECT:
            break;
        case FCS_DECLARED_INCORRECT:
            EV_ERROR << "incorrect FCS in ethernet frame\n";
            return false;
        case FCS_COMPUTED: {
            bool isFcsBad = false;
            // check the FCS
            auto ethBytes = packet->peekDataAt<BytesChunk>(B(0), packet->getDataLength() - ethTrailer->getChunkLength());
            auto bufferLength = B(ethBytes->getChunkLength()).get();
            auto buffer = new uint8_t[bufferLength];
            // 1. fill in the data
            ethBytes->copyToBuffer(buffer, bufferLength);
            // 2. compute the FCS
            auto computedFcs = ethernetCRC(buffer, bufferLength);
            delete[] buffer;
            isFcsBad = (computedFcs != ethTrailer->getFcs()); // FIXME how to check fcs?
            if (isFcsBad)
                return false;
            break;
        }
        default:
            throw cRuntimeError("invalid FCS mode in ethernet frame");
    }
    if (isIeee8023Header(*ethHeader)) {
        b payloadLength = B(ethHeader->getTypeOrLength());

        return payloadLength <= packet->getDataLength() - (ethHeader->getChunkLength() + ethTrailer->getChunkLength());
    }
    return true;
}

void EthernetMacBase::refreshConnection()
{
    Enter_Method("refreshConnection");

    bool oldConn = connected;
    readChannelParameters(false);

    if (oldConn != connected)
        processConnectDisconnect();
}

bool EthernetMacBase::dropFrameNotForUs(Packet *packet, const Ptr<const EthernetMacHeader>& frame)
{
    // Current ethernet mac implementation does not support the configuration of multicast
    // ethernet address groups. We rather accept all multicast frames (just like they were
    // broadcasts) and pass it up to the higher layer where they will be dropped
    // if not needed.
    //
    // PAUSE frames must be handled a bit differently because they are processed at
    // this level. Multicast PAUSE frames should not be processed unless they have a
    // destination of MULTICAST_PAUSE_ADDRESS. We drop all PAUSE frames that have a
    // different muticast destination. (Note: Would the multicast ethernet addressing
    // implemented, we could also process the PAUSE frames destined to any of our
    // multicast adresses)
    // All NON-PAUSE frames must be passed to the upper layer if the interface is
    // in promiscuous mode.

    if (frame->getDest().equals(getMacAddress()))
        return false;

    if (frame->getDest().isBroadcast())
        return false;

    bool isPause = (frame->getTypeOrLength() == ETHERTYPE_FLOW_CONTROL);

    if (!isPause && (promiscuous || frame->getDest().isMulticast()))
        return false;

    if (isPause && frame->getDest().equals(MacAddress::MULTICAST_PAUSE_ADDRESS))
        return false;

    EV_WARN << "Frame `" << packet->getName() << "' not destined to us, discarding\n";
    numDroppedNotForUs++;
    PacketDropDetails details;
    details.setReason(NOT_ADDRESSED_TO_US);
    emit(packetDroppedSignal, packet, &details);
    delete packet;
    return true;
}

void EthernetMacBase::readChannelParameters(bool errorWhenAsymmetric)
{
    // When the connected channels change at runtime, we'll receive
    // two separate notifications (one for the rx channel and one for the tx one),
    // so we cannot immediately raise an error when they differ. Rather, we'll need
    // to verify at the next opportunity (event) that the two channels have eventually
    // been set to the same value.

    cDatarateChannel *outTrChannel = check_and_cast_nullable<cDatarateChannel *>(physOutGate->findTransmissionChannel());
    cDatarateChannel *inTrChannel = check_and_cast_nullable<cDatarateChannel *>(physInGate->findIncomingTransmissionChannel());

    connected = physOutGate->getPathEndGate()->isConnected() && physInGate->getPathStartGate()->isConnected();

    if (connected && ((!outTrChannel) || (!inTrChannel)))
        throw cRuntimeError("Ethernet phys gate must be connected using a transmission channel");

    double txRate = outTrChannel ? outTrChannel->getNominalDatarate() : 0.0;
    double rxRate = inTrChannel ? inTrChannel->getNominalDatarate() : 0.0;

    bool rxDisabled = !inTrChannel || inTrChannel->isDisabled();
    bool txDisabled = !outTrChannel || outTrChannel->isDisabled();

    if (errorWhenAsymmetric && (rxDisabled != txDisabled))
        throw cRuntimeError("The enablements of the input/output channels differ (rx=%s vs tx=%s)", rxDisabled ? "off" : "on", txDisabled ? "off" : "on");

    if (txDisabled)
        connected = false;

    bool dataratesDiffer;
    if (!connected) {
        curEtherDescr = &nullEtherDescr;
        dataratesDiffer = false;
        if (!outTrChannel)
            transmissionChannel = nullptr;
        if (networkInterface) {
            networkInterface->setCarrier(false);
            networkInterface->setDatarate(0);
        }
    }
    else {
        if (outTrChannel && !transmissionChannel)
            outTrChannel->subscribe(POST_MODEL_CHANGE, this);
        transmissionChannel = outTrChannel;
        dataratesDiffer = (txRate != rxRate);
    }

    channelsDiffer = dataratesDiffer || (rxDisabled != txDisabled);

    if (errorWhenAsymmetric && dataratesDiffer)
        throw cRuntimeError("The input/output datarates differ (rx=%g bps vs tx=%g bps)", rxRate, txRate);

    if (connected) {
        // Check valid speeds
        for (auto& etherDescr : etherDescrs) {
            if (txRate == etherDescr.txrate) {
                curEtherDescr = &(etherDescr);
                if (networkInterface) {
                    networkInterface->setCarrier(true);
                    networkInterface->setDatarate(txRate);
                }
                return;
            }
        }
        throw cRuntimeError("Invalid transmission rate %g bps on channel %s at module %s",
                txRate, transmissionChannel->getFullPath().c_str(), getFullPath().c_str());
    }
}

void EthernetMacBase::printParameters()
{
    // Dump parameters
    EV_DETAIL << "MAC address: " << getMacAddress() << (promiscuous ? ", promiscuous mode" : "") << endl
              << "txrate: " << curEtherDescr->txrate << " bps, "
              << (duplexMode ? "full-duplex" : "half-duplex") << endl
              << "bitTime: " << 1e9 / curEtherDescr->txrate << " ns" << endl
              << "frameBursting: " << (frameBursting ? "on" : "off") << endl
              << "slotTime: " << curEtherDescr->slotTime << endl
              << "interFrameGap: " << INTERFRAME_GAP_BITS / curEtherDescr->txrate << endl
              << endl;
}

void EthernetMacBase::finish()
{
    {
        simtime_t t = simTime();
        recordScalar("simulated time", t);
        recordScalar("full-duplex", duplexMode);

        if (t > 0) {
            recordScalar("frames/sec sent", numFramesSent / t);
            recordScalar("frames/sec rcvd", numFramesReceivedOK / t);
            recordScalar("bits/sec sent", (8.0 * numBytesSent) / t);
            recordScalar("bits/sec rcvd", (8.0 * numBytesReceivedOK) / t);
        }
    }
}

void EthernetMacBase::refreshDisplay() const
{
    MacProtocolBase::refreshDisplay();

    // icon coloring
    const char *color;

    if (receiveState == RX_COLLISION_STATE)
        color = "red";
    else if (transmitState == TRANSMITTING_STATE)
        color = "yellow";
    else if (transmitState == JAMMING_STATE)
        color = "red";
    else if (receiveState == RECEIVING_STATE)
        color = "#4040ff";
    else if (transmitState == BACKOFF_STATE)
        color = "white";
    else if (transmitState == PAUSE_STATE)
        color = "gray";
    else
        color = "";

    getDisplayString().setTagArg("i", 1, color);

    if (!strcmp(getParentModule()->getNedTypeName(), "inet.linklayer.ethernet.EthernetInterface"))
        getParentModule()->getDisplayString().setTagArg("i", 1, color);

    auto text = StringFormat::formatString(displayStringTextFormat, [&] (char directive) -> std::string {
         switch (directive) {
            case 's':
                return std::to_string(numFramesSent);
            case 'r':
                return std::to_string(numFramesReceivedOK);
            case 'd':
                return std::to_string(numDroppedPkFromHLIfaceDown + numDroppedIfaceDown + numDroppedBitError + numDroppedNotForUs);
            case 'q':
                return txQueue != nullptr ? std::to_string(txQueue->getNumPackets()) : "";
            case 'b':
                if (transmissionChannel == nullptr)
                    return "not connected";
                else {
                    char datarateText[40];
                    double datarate = transmissionChannel->getNominalDatarate();
                    if (datarate >= 1e9)
                        sprintf(datarateText, "%gGbps", datarate / 1e9);
                    else if (datarate >= 1e6)
                        sprintf(datarateText, "%gMbps", datarate / 1e6);
                    else if (datarate >= 1e3)
                        sprintf(datarateText, "%gkbps", datarate / 1e3);
                    else
                        sprintf(datarateText, "%gbps", datarate);
                    return datarateText;
                }
            default:
                throw cRuntimeError("Unknown directive: %c", directive);
        }
        });
    getDisplayString().setTagArg("t", 0, text.c_str());
}

void EthernetMacBase::changeTransmissionState(MacTransmitState newState)
{
    transmitState = newState;
    emit(transmissionStateChangedSignal, newState);
}

void EthernetMacBase::changeReceptionState(MacReceiveState newState)
{
    receiveState = newState;
    emit(receptionStateChangedSignal, newState);
}

void EthernetMacBase::addPaddingAndSetFcs(Packet *packet, B requiredMinBytes) const
{
    auto ethFcs = packet->removeAtBack<EthernetFcs>(ETHER_FCS_BYTES);
    ethFcs->setFcsMode(fcsMode);

    B paddingLength = requiredMinBytes - ETHER_FCS_BYTES - B(packet->getByteLength());
    if (paddingLength > B(0)) {
        const auto& ethPadding = makeShared<EthernetPadding>();
        ethPadding->setChunkLength(paddingLength);
        packet->insertAtBack(ethPadding);
    }

    switch (ethFcs->getFcsMode()) {
        case FCS_DECLARED_CORRECT:
            ethFcs->setFcs(0xC00DC00DL);
            break;
        case FCS_DECLARED_INCORRECT:
            ethFcs->setFcs(0xBAADBAADL);
            break;
        case FCS_COMPUTED: { // calculate FCS
            auto ethBytes = packet->peekDataAsBytes();
            auto bufferLength = B(ethBytes->getChunkLength()).get();
            auto buffer = new uint8_t[bufferLength];
            // 1. fill in the data
            ethBytes->copyToBuffer(buffer, bufferLength);
            // 2. compute the FCS
            auto computedFcs = ethernetCRC(buffer, bufferLength);
            delete[] buffer;
            ethFcs->setFcs(computedFcs);
            break;
        }
        default:
            throw cRuntimeError("Unknown FCS mode: %d", (int)(ethFcs->getFcsMode()));
    }

    packet->insertAtBack(ethFcs);
}

void EthernetMacBase::cutEthernetSignalEnd(EthernetSignalBase *signal, simtime_t duration)
{
    ASSERT(duration <= signal->getDuration());
    if (duration == signal->getDuration())
        return;
    int64_t newBitLength = duration.dbl() * signal->getBitrate();
    if (auto packet = check_and_cast_nullable<Packet *>(signal->decapsulate())) {
        // TODO removed length calculation based on the PHY layer (parallel bits, bit order, etc.)
        if (newBitLength < packet->getBitLength()) {
            packet->trimFront();
            packet->setBackOffset(B(newBitLength / 8)); //TODO rounded down to byte align instead of b(newBitLength)
            packet->trimBack();
            packet->setBitError(true);
        }
        signal->encapsulate(packet);
    }
    signal->setBitError(true);
    signal->setBitLength(newBitLength);
    signal->setDuration(duration);
}

void EthernetMacBase::txFinished()
{
    ASSERT(curTxSignal != nullptr);
    delete curTxSignal;
    curTxSignal = nullptr;
}

queueing::IPassivePacketSource *EthernetMacBase::getProvider(cGate *gate)
{
    return (gate->getId() == upperLayerInGateId) ? txQueue.get() : nullptr;
}

void EthernetMacBase::handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful)
{
    Enter_Method("handlePullPacketProcessed");
    throw cRuntimeError("Not supported callback");
}

} // namespace inet


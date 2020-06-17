//
// Copyright (C) 2006 Levente Meszaros
// Copyright (C) 2004 Andras Varga
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

#include <stdlib.h>

#include "inet/common/checksum/EthernetCRC.h"
#include "inet/common/INETUtils.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/StringFormat.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/ethernet/EtherMacBase.h"
#include "inet/linklayer/ethernet/Ethernet.h"
#include "inet/linklayer/ethernet/EtherPhyFrame_m.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/queueing/function/PacketComparatorFunction.h"

namespace inet {

const double EtherMacBase::SPEED_OF_LIGHT_IN_CABLE = 200000000.0;

const EtherMacBase::EtherDescr EtherMacBase::nullEtherDescr = {
    0.0,
    0.0,
    0,
    B(0),
    B(0),
    B(0),
    0.0,
    0.0
};

const EtherMacBase::EtherDescr EtherMacBase::etherDescrs[NUM_OF_ETHERDESCRS] = {
    {
        ETHERNET_TXRATE,
        0.5 / ETHERNET_TXRATE,
        0,
        B(0),
        MIN_ETHERNET_FRAME_BYTES,
        MIN_ETHERNET_FRAME_BYTES,
        512 / ETHERNET_TXRATE,
        2500    /*m*/ / SPEED_OF_LIGHT_IN_CABLE
    },
    {
        FAST_ETHERNET_TXRATE,
        0.5 / FAST_ETHERNET_TXRATE,
        0,
        B(0),
        MIN_ETHERNET_FRAME_BYTES,
        MIN_ETHERNET_FRAME_BYTES,
        512 / FAST_ETHERNET_TXRATE,
        250    /*m*/ / SPEED_OF_LIGHT_IN_CABLE
    },
    {
        GIGABIT_ETHERNET_TXRATE,
        0.5 / GIGABIT_ETHERNET_TXRATE,
        MAX_PACKETBURST,
        GIGABIT_MAX_BURST_BYTES,
        GIGABIT_MIN_FRAME_BYTES_WITH_EXT,
        MIN_ETHERNET_FRAME_BYTES,
        4096 / GIGABIT_ETHERNET_TXRATE,
        250    /*m*/ / SPEED_OF_LIGHT_IN_CABLE
    },
    {
        FAST_GIGABIT_ETHERNET_TXRATE,
        0.5 / FAST_GIGABIT_ETHERNET_TXRATE,
        0,
        B(0),
        B(-1),    // half-duplex is not supported
        B(0),
        0.0,
        0.0
    },
    {
        TWENTYFIVE_GIGABIT_ETHERNET_TXRATE,
        0.5 / TWENTYFIVE_GIGABIT_ETHERNET_TXRATE,
        0,
        B(0),
        B(-1),    // half-duplex is not supported
        B(0),
        0.0,
        0.0
    },
    {
        FOURTY_GIGABIT_ETHERNET_TXRATE,
        0.5 / FOURTY_GIGABIT_ETHERNET_TXRATE,
        0,
        B(0),
        B(-1),    // half-duplex is not supported
        B(0),
        0.0,
        0.0
    },
    {
        HUNDRED_GIGABIT_ETHERNET_TXRATE,
        0.5 / HUNDRED_GIGABIT_ETHERNET_TXRATE,
        0,
        B(0),
        B(-1),    // half-duplex is not supported
        B(0),
        0.0,
        0.0
    },
    {
        TWOHUNDRED_GIGABIT_ETHERNET_TXRATE,
        0.5 / TWOHUNDRED_GIGABIT_ETHERNET_TXRATE,
        0,
        B(0),
        B(-1),    // half-duplex is not supported
        B(0),
        0.0,
        0.0
    },
    {
        FOURHUNDRED_GIGABIT_ETHERNET_TXRATE,
        0.5 / FOURHUNDRED_GIGABIT_ETHERNET_TXRATE,
        0,
        B(0),
        B(-1),    // half-duplex is not supported
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

simsignal_t EtherMacBase::rxPkOkSignal = registerSignal("rxPkOk");
simsignal_t EtherMacBase::txPausePkUnitsSignal = registerSignal("txPausePkUnits");
simsignal_t EtherMacBase::rxPausePkUnitsSignal = registerSignal("rxPausePkUnits");

simsignal_t EtherMacBase::transmissionStateChangedSignal = registerSignal("transmissionStateChanged");
simsignal_t EtherMacBase::receptionStateChangedSignal = registerSignal("receptionStateChanged");

EtherMacBase::EtherMacBase()
{
    lastTxFinishTime = -1.0;    // never equals to current simtime
    curEtherDescr = &nullEtherDescr;
}

EtherMacBase::~EtherMacBase()
{
    cancelAndDelete(endTxMsg);
    cancelAndDelete(endIFGMsg);
    cancelAndDelete(endPauseMsg);
}

void EtherMacBase::initialize(int stage)
{
    MacProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        physInGate = gate("phys$i");
        physOutGate = gate("phys$o");
        lowerLayerInGateId = physInGate->getId();
        lowerLayerOutGateId = physOutGate->getId();
        rxTransmissionChannel = nullptr;
        txTransmissionChannel = nullptr;
        currentTxFrame = nullptr;

        initializeFlags();

        initializeStatistics();

        lastTxFinishTime = -1.0;    // not equals with current simtime.

        // initialize self messages
        endTxMsg = new cMessage("EndTransmission", ENDTRANSMISSION);
        endIFGMsg = new cMessage("EndIFG", ENDIFG);
        endPauseMsg = new cMessage("EndPause", ENDPAUSE);

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

void EtherMacBase::initializeQueue()
{
    txQueue = check_and_cast<queueing::IPacketQueue *>(getSubmodule("queue"));
}

void EtherMacBase::initializeFlags()
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

void EtherMacBase::initializeStatistics()
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

void EtherMacBase::configureInterfaceEntry()
{

    // MTU: typical values are 576 (Internet de facto), 1500 (Ethernet-friendly),
    // 4000 (on some point-to-point links), 4470 (Cisco routers default, FDDI compatible)
    interfaceEntry->setMtu(par("mtu"));

    // capabilities
    interfaceEntry->setMulticast(true);
    interfaceEntry->setBroadcast(true);
}

void EtherMacBase::handleStartOperation(LifecycleOperation *operation)
{
    interfaceEntry->setState(InterfaceEntry::State::UP);
    initializeFlags();
    initializeQueue();
    readChannelParameters(true);
}

void EtherMacBase::handleStopOperation(LifecycleOperation *operation)
{
    if (currentTxFrame != nullptr || !txQueue->isEmpty()) {
        interfaceEntry->setState(InterfaceEntry::State::GOING_DOWN);
        delayActiveOperationFinish(par("stopOperationTimeout"));
    }
    else {
        interfaceEntry->setCarrier(false);
        interfaceEntry->setState(InterfaceEntry::State::DOWN);
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
    }
}

void EtherMacBase::handleCrashOperation(LifecycleOperation *operation)
{
//    clearQueue();
    connected = false;
    interfaceEntry->setCarrier(false);
    processConnectDisconnect();
    interfaceEntry->setState(InterfaceEntry::State::DOWN);
}

// TODO: this method should be renamed and called where processing is finished on the current frame (i.e. curTxFrame becomes nullptr)
void EtherMacBase::processAtHandleMessageFinished()
{
    if (operationalState == State::STOPPING_OPERATION) {
        if (currentTxFrame == nullptr && txQueue->isEmpty()) {
            EV << "Ethernet Queue is empty, MAC stopped\n";
            connected = false;
            interfaceEntry->setCarrier(false);
            processConnectDisconnect();
            interfaceEntry->setState(InterfaceEntry::State::DOWN);
            startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
        }
    }
}

void EtherMacBase::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method_Silent();

    MacProtocolBase::receiveSignal(source, signalID, obj, details);

    if (signalID != POST_MODEL_CHANGE)
        return;

    if (auto gcobj = dynamic_cast<cPostPathCreateNotification *>(obj)) {
        if ((physOutGate == gcobj->pathStartGate) || (physInGate == gcobj->pathEndGate))
            refreshConnection();
    }
    else if (auto gcobj = dynamic_cast<cPostPathCutNotification *>(obj)) {
        if ((physOutGate == gcobj->pathStartGate) || (physInGate == gcobj->pathEndGate))
            refreshConnection();
    }
    else if (auto gcobj = dynamic_cast<cPostParameterChangeNotification *>(obj)) {    // note: we are subscribed to the channel object too
        if (gcobj->par->getOwner() == txTransmissionChannel || gcobj->par->getOwner() == rxTransmissionChannel)
            refreshConnection();
    }
}

void EtherMacBase::processConnectDisconnect()
{
    if (!connected) {
        cancelEvent(endTxMsg);
        cancelEvent(endIFGMsg);
        cancelEvent(endPauseMsg);

        if (currentTxFrame) {
            EV_DETAIL << "Interface is not connected, dropping packet " << currentTxFrame << endl;
            numDroppedPkFromHLIfaceDown++;
            PacketDropDetails details;
            details.setReason(INTERFACE_DOWN);
            dropCurrentTxFrame(details);
            lastTxFinishTime = -1.0;    // so that it never equals to the current simtime, used for Burst mode detection.
        }

        // Clear queue
        while (!txQueue->isEmpty()) {
            Packet *msg = txQueue->dequeuePacket();
            EV_DETAIL << "Interface is not connected, dropping packet " << msg << endl;
            numDroppedPkFromHLIfaceDown++;
            PacketDropDetails details;
            details.setReason(INTERFACE_DOWN);
            emit(packetDroppedSignal, msg, &details);
            delete msg;
        }

        changeTransmissionState(TX_IDLE_STATE);         //FIXME replace status to OFF
        changeReceptionState(RX_IDLE_STATE);
    }
    //FIXME when connect, set statuses to RECONNECT or IDLE
}

void EtherMacBase::encapsulate(Packet *frame)
{
    auto phyHeader = makeShared<EthernetPhyHeader>();
    frame->insertAtFront(phyHeader);
    frame->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetPhy);
}

void EtherMacBase::decapsulate(Packet *packet)
{
    auto phyHeader = packet->popAtFront<EthernetPhyHeader>();
    ASSERT(packet->getDataLength() >= MIN_ETHERNET_FRAME_BYTES);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);
}

//FIXME should use it in EtherMac, EtherMacFullDuplex, etc. modules. But should not use it in EtherBus, EtherHub.
bool EtherMacBase::verifyCrcAndLength(Packet *packet)
{
    EV_STATICCONTEXT;

    auto ethHeader = packet->peekAtFront<EthernetMacHeader>();          //FIXME can I use any flags?
    const auto& ethTrailer = packet->peekAtBack<EthernetFcs>(ETHER_FCS_BYTES);          //FIXME can I use any flags?

    switch(ethTrailer->getFcsMode()) {
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
            delete [] buffer;
            isFcsBad = (computedFcs != ethTrailer->getFcs());      //FIXME how to check fcs?
            if (isFcsBad)
                return false;
            break;
        }
        default:
            throw cRuntimeError("invalid FCS mode in ethernet frame");
    }
    if (isIeee8023Header(*ethHeader)) {
        b payloadLength = B(ethHeader->getTypeOrLength());

        return (payloadLength <= packet->getDataLength() - (ethHeader->getChunkLength() + ethTrailer->getChunkLength()));
    }
    return true;
}

void EtherMacBase::refreshConnection()
{
    Enter_Method_Silent();

    bool oldConn = connected;
    readChannelParameters(false);

    if (oldConn != connected)
        processConnectDisconnect();
}

bool EtherMacBase::dropFrameNotForUs(Packet *packet, const Ptr<const EthernetMacHeader>& frame)
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

void EtherMacBase::readChannelParameters(bool errorWhenAsymmetric)
{
    // When the connected channels change at runtime, we'll receive
    // two separate notifications (one for the rx channel and one for the tx one),
    // so we cannot immediately raise an error when they differ. Rather, we'll need
    // to verify at the next opportunity (event) that the two channels have eventually
    // been set to the same value.

    auto *outTrChannel = check_and_cast_nullable<cTransmissionChannel *>(physOutGate->findTransmissionChannel());
    auto *inTrChannel = check_and_cast_nullable<cTransmissionChannel *>(physInGate->findIncomingTransmissionChannel());

    connected = physOutGate->getPathEndGate()->isConnected() && physInGate->getPathStartGate()->isConnected();

    if (connected && ((!outTrChannel) || (!inTrChannel)))
        throw cRuntimeError("Ethernet phys gate must be connected using a transmission channel");

    bool rxDisabled = !inTrChannel || inTrChannel->isDisabled();
    bool txDisabled = !outTrChannel || outTrChannel->isDisabled();
    if (errorWhenAsymmetric && (rxDisabled != txDisabled))
        throw cRuntimeError("The enablements of the input/output channels differ (rx=%s vs tx=%s)", rxDisabled ? "off" : "on", txDisabled ? "off" : "on");

    auto rxDelay = inTrChannel ? inTrChannel->getDelay() : NaN;
    auto txDelay = outTrChannel ? outTrChannel->getDelay() : NaN;
    if (errorWhenAsymmetric && (rxDelay != txDelay))
        throw cRuntimeError("The delays on the input/output channels differ (rx=%s vs tx=%s)", rxDelay.str().c_str(), txDelay.str().c_str());

    if (rxDisabled || txDisabled)
        connected = false;

    if (!interfaceEntry)
        interfaceEntry = getContainingNicModule(this);

    channelsDiffer = (rxDisabled != txDisabled) || (rxDelay != txDelay);

    if (connected) {
        rxTransmissionChannel = inTrChannel;
        txTransmissionChannel = outTrChannel;
        if (!rxTransmissionChannel->isSubscribed(POST_MODEL_CHANGE, this))
            rxTransmissionChannel->subscribe(POST_MODEL_CHANGE, this);
        if (!txTransmissionChannel->isSubscribed(POST_MODEL_CHANGE, this))
            txTransmissionChannel->subscribe(POST_MODEL_CHANGE, this);

        // Check valid speeds
        if (rxTransmissionChannel->hasPar("datarate") && txTransmissionChannel->hasPar("datarate")) {
            double bitrate = txTransmissionChannel->par("datarate");
            double rxBitrate = rxTransmissionChannel->par("datarate");
            if (bitrate != rxBitrate)
                throw cRuntimeError("The datarate parameters on tx and rx transmission channels are differ %g vs %g", bitrate, rxBitrate);
            interfaceEntry->par("bitrate").setDoubleValue(bitrate);
        }
        else if (!rxTransmissionChannel->hasPar("datarate") && !txTransmissionChannel->hasPar("datarate")) {
            // channels doesn't have datarate parameters
        }
        else
            throw cRuntimeError("asymmetric settings: only one channel has datarate parameter on tx/rx transmission channels");

        double txRate = interfaceEntry->par("bitrate");
        for (auto & etherDescr : etherDescrs) {
            if (txRate == etherDescr.txrate) {
                curEtherDescr = &(etherDescr);
                interfaceEntry->setCarrier(true);
                interfaceEntry->setDatarate(txRate);
                return;
            }
        }
        throw cRuntimeError("Invalid transmission rate %g bps on channel %s at module %s",
                txRate, txTransmissionChannel->getFullPath().c_str(), getFullPath().c_str());
    }
    else {
        curEtherDescr = &nullEtherDescr;
        if (!inTrChannel)
            rxTransmissionChannel = nullptr;
        if (!outTrChannel)
            txTransmissionChannel = nullptr;
        interfaceEntry->setCarrier(false);
        interfaceEntry->setDatarate(0);
    }
}

void EtherMacBase::printParameters()
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

void EtherMacBase::finish()
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

void EtherMacBase::refreshDisplay() const
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

    auto text = StringFormat::formatString(displayStringTextFormat, [&] (char directive) {
        static std::string result;
        switch (directive) {
            case 's':
                result = std::to_string(numFramesSent);
                break;
            case 'r':
                result = std::to_string(numFramesReceivedOK);
                break;
            case 'd':
                result = std::to_string(numDroppedPkFromHLIfaceDown + numDroppedIfaceDown + numDroppedBitError + numDroppedNotForUs);
                break;
            case 'q':
                result = std::to_string(txQueue->getNumPackets());
                break;
            case 'b':
                if (txTransmissionChannel == nullptr)
                    result = "not connected";
                else {
                    char datarateText[40];
                    double datarate = txTransmissionChannel->getNominalDatarate();
                    if (datarate >= 1e9)
                        sprintf(datarateText, "%gGbps", datarate / 1e9);
                    else if (datarate >= 1e6)
                        sprintf(datarateText, "%gMbps", datarate / 1e6);
                    else if (datarate >= 1e3)
                        sprintf(datarateText, "%gkbps", datarate / 1e3);
                    else
                        sprintf(datarateText, "%gbps", datarate);
                    result = datarateText;
                }
                break;
            default:
                throw cRuntimeError("Unknown directive: %c", directive);
        }
        return result.c_str();
    });
    getDisplayString().setTagArg("t", 0, text);
}

void EtherMacBase::changeTransmissionState(MacTransmitState newState)
{
    transmitState = newState;
    emit(transmissionStateChangedSignal, newState);
}

void EtherMacBase::changeReceptionState(MacReceiveState newState)
{
    receiveState = newState;
    emit(receptionStateChangedSignal, newState);
}

void EtherMacBase::addPaddingAndSetFcs(Packet *packet, B requiredMinBytes) const
{
    auto ethFcs = packet->removeAtBack<EthernetFcs>(ETHER_FCS_BYTES);

    B paddingLength = requiredMinBytes - ETHER_FCS_BYTES - B(packet->getByteLength());
    if (paddingLength > B(0)) {
        const auto& ethPadding = makeShared<EthernetPadding>();
        ethPadding->setChunkLength(paddingLength);
        packet->insertAtBack(ethPadding);
    }

    switch(ethFcs->getFcsMode()) {
        case FCS_DECLARED_CORRECT:
            ethFcs->setFcs(0xC00DC00DL);
            break;
        case FCS_DECLARED_INCORRECT:
            ethFcs->setFcs(0xBAADBAADL);
            break;
        case FCS_COMPUTED:
            { // calculate FCS
                auto ethBytes = packet->peekDataAsBytes();
                auto bufferLength = B(ethBytes->getChunkLength()).get();
                auto buffer = new uint8_t[bufferLength];
                // 1. fill in the data
                ethBytes->copyToBuffer(buffer, bufferLength);
                // 2. compute the FCS
                auto computedFcs = ethernetCRC(buffer, bufferLength);
                delete [] buffer;
                ethFcs->setFcs(computedFcs);
            }
            break;
        default:
            throw cRuntimeError("Unknown FCS mode: %d", (int)(ethFcs->getFcsMode()));
    }

    packet->insertAtBack(ethFcs);
}

simtime_t EtherMacBase::calculateDuration(EthernetSignalBase *signal)
{
    return signal->getBitLength() / signal->getBitrate();
}

} // namespace inet


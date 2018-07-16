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
#include "inet/linklayer/ethernet/EtherMacBase.h"
#include "inet/common/ProtocolTag_m.h"
//#include "inet/common/ProtocolGroup.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/common/checksum/EthernetCRC.h"
#include "inet/linklayer/ethernet/EtherPhyFrame_m.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/ethernet/Ethernet.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/common/queue/IPassiveQueue.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/INETUtils.h"

namespace inet {

const double EtherMacBase::SPEED_OF_LIGHT_IN_CABLE = 200000000.0;

const EtherMacBase::EtherDescr EtherMacBase::nullEtherDescr = {
    0.0,
    0.0,
    B(0),
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
        MIN_ETHERNET_FRAME_BYTES,
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
        MIN_ETHERNET_FRAME_BYTES,
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
        MIN_ETHERNET_FRAME_BYTES,
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
        MIN_ETHERNET_FRAME_BYTES,
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
        MIN_ETHERNET_FRAME_BYTES,
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
        MIN_ETHERNET_FRAME_BYTES,
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
        MIN_ETHERNET_FRAME_BYTES,
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
        MIN_ETHERNET_FRAME_BYTES,
        0,
        B(0),
        B(-1),    // half-duplex is not supported
        B(0),
        0.0,
        0.0
    }
};

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
    delete curTxFrame;

    cancelAndDelete(endTxMsg);
    cancelAndDelete(endIFGMsg);
    cancelAndDelete(endPauseMsg);
}

void EtherMacBase::initialize(int stage)
{
    MacBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        physInGate = gate("phys$i");
        physOutGate = gate("phys$o");
        upperLayerInGate = gate("upperLayerIn");
        transmissionChannel = nullptr;
        curTxFrame = nullptr;

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
        WATCH(transmitState);
        WATCH(receiveState);

        // initialize pause
        pauseUnitsRequested = 0;
        WATCH(pauseUnitsRequested);

        subscribe(POST_MODEL_CHANGE, this);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        initializeMacAddress();
        registerInterface();    // needs MAC address    //FIXME why not called in MacBase::initialize()?
        initializeQueueModule();
        readChannelParameters(true);
    }
}

void EtherMacBase::initializeQueueModule()
{
    if (par("queueModule").stringValue()[0]) {
        cModule *module = getModuleFromPar<cModule>(par("queueModule"), this);
        IPassiveQueue *queueModule;
        if (module->isSimple())
            queueModule = check_and_cast<IPassiveQueue *>(module);
        else {
            cGate *queueOut = module->gate("out")->getPathStartGate();
            queueModule = check_and_cast<IPassiveQueue *>(queueOut->getOwnerModule());
        }

        EV_DETAIL << "Requesting first frame from queue module\n";
        txQueue.setExternalQueue(queueModule);

        if (txQueue.extQueue->getNumPendingRequests() == 0)
            txQueue.extQueue->requestPacket();
    }
    else {
        txQueue.setInternalQueue("txQueue", par("txQueueLimit"));
    }
}

void EtherMacBase::initializeMacAddress()
{
    const char *addrstr = par("address");

    if (!strcmp(addrstr, "auto")) {
        // assign automatic address
        address = MacAddress::generateAutoAddress();

        // change module parameter from "auto" to concrete address
        par("address").setStringValue(address.str().c_str());
    }
    else {
        address.setAddress(addrstr);
    }
}

void EtherMacBase::initializeFlags()
{
    sendRawBytes = par("sendRawBytes");
    duplexMode = true;

    // initialize connected flag
    connected = physOutGate->getPathEndGate()->isConnected() && physInGate->getPathStartGate()->isConnected();

    if (!connected)
        EV_WARN << "MAC not connected to a network.\n";

    WATCH(connected);

    // TODO: this should be set from the GUI
    // initialize disabled flag
    // Note: it is currently not supported to enable a disabled MAC at runtime.
    // Difficulties: (1) autoconfig (2) how to pick up channel state (free, tx, collision etc)
    disabled = false;
    WATCH(disabled);

    // initialize promiscuous flag
    promiscuous = par("promiscuous");
    WATCH(promiscuous);

    frameBursting = false;
    WATCH(frameBursting);
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

InterfaceEntry *EtherMacBase::createInterfaceEntry()
{
    InterfaceEntry *interfaceEntry = getContainingNicModule(this);

    // generate a link-layer address to be used as interface token for IPv6
    interfaceEntry->setMacAddress(address);
    interfaceEntry->setInterfaceToken(address.formInterfaceIdentifier());
    //InterfaceToken token(0, getSimulation()->getUniqueNumber(), 64);
    //interfaceEntry->setInterfaceToken(token);

    // MTU: typical values are 576 (Internet de facto), 1500 (Ethernet-friendly),
    // 4000 (on some point-to-point links), 4470 (Cisco routers default, FDDI compatible)
    interfaceEntry->setMtu(par("mtu"));

    // capabilities
    interfaceEntry->setMulticast(true);
    interfaceEntry->setBroadcast(true);

    return interfaceEntry;
}

bool EtherMacBase::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (static_cast<NodeStartOperation::Stage>(stage) == NodeStartOperation::STAGE_LINK_LAYER) {
            initializeFlags();
            initializeMacAddress();
            initializeQueueModule();
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (static_cast<NodeShutdownOperation::Stage>(stage) == NodeShutdownOperation::STAGE_LINK_LAYER) {
            connected = false;
            processConnectDisconnect();
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (static_cast<NodeCrashOperation::Stage>(stage) == NodeCrashOperation::STAGE_CRASH) {
            connected = false;
            processConnectDisconnect();
        }
    }
    return MacBase::handleOperationStage(operation, stage, doneCallback);
}

void EtherMacBase::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method_Silent();

    MacBase::receiveSignal(source, signalID, obj, details);

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
    else if (transmissionChannel && dynamic_cast<cPostParameterChangeNotification *>(obj)) {    // note: we are subscribed to the channel object too
        cPostParameterChangeNotification *gcobj = static_cast<cPostParameterChangeNotification *>(obj);
        if (transmissionChannel == gcobj->par->getOwner())
            refreshConnection();
    }
}

void EtherMacBase::processConnectDisconnect()
{
    if (!connected) {
        cancelEvent(endTxMsg);
        cancelEvent(endIFGMsg);
        cancelEvent(endPauseMsg);

        if (curTxFrame) {
            EV_DETAIL << "Interface is not connected, dropping packet " << curTxFrame << endl;
            numDroppedPkFromHLIfaceDown++;
            PacketDropDetails details;
            details.setReason(INTERFACE_DOWN);
            emit(packetDroppedSignal, curTxFrame, &details);
            delete curTxFrame;
            curTxFrame = nullptr;
            lastTxFinishTime = -1.0;    // so that it never equals to the current simtime, used for Burst mode detection.
        }

        if (txQueue.extQueue) {
            // Clear external queue: send a request, and received packet will be deleted in handleMessage()
            if (txQueue.extQueue->getNumPendingRequests() == 0)
                txQueue.extQueue->requestPacket();
        }
        else {
            // Clear inner queue
            while (!txQueue.innerQueue->isEmpty()) {
                cMessage *msg = check_and_cast<cMessage *>(txQueue.innerQueue->pop());
                EV_DETAIL << "Interface is not connected, dropping packet " << msg << endl;
                numDroppedPkFromHLIfaceDown++;
                PacketDropDetails details;
                details.setReason(INTERFACE_DOWN);
                emit(packetDroppedSignal, msg, &details);
                delete msg;
            }
        }

        changeTransmissionState(TX_IDLE_STATE);         //FIXME replace status to OFF
        changeReceptionState(RX_IDLE_STATE);
    }
    //FIXME when connect, set statuses to RECONNECT or IDLE
}

void EtherMacBase::encapsulate(Packet *frame)
{
    auto phyHeader = makeShared<EthernetPhyHeader>();
    phyHeader->setSrcMacFullDuplex(duplexMode);
    frame->insertAtFront(phyHeader);
    frame->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetPhy);
}

void EtherMacBase::decapsulate(Packet *packet)
{
    auto phyHeader = packet->popAtFront<EthernetPhyHeader>();
    if (phyHeader->getSrcMacFullDuplex() != duplexMode)
        throw cRuntimeError("Ethernet misconfiguration: MACs on the same link must be all in full duplex mode, or all in half-duplex mode");
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ethernetMac);
}

//FIXME should use it in EtherMac, EtherMacFullDuplex, etc. modules. But should not use it in EtherBus, EtherHub.
bool EtherMacBase::verifyCrcAndLength(Packet *packet)
{
    EV_STATICCONTEXT;

    auto ethHeader = packet->peekAtFront<EthernetMacHeader>();          //FIXME can I use any flags?
    const auto& ethTrailer = packet->peekAtBack<EthernetFcs>(B(ETHER_FCS_BYTES));          //FIXME can I use any flags?

    switch(ethTrailer->getFcsMode()) {
        case FCS_DECLARED_CORRECT:
            break;
        case FCS_DECLARED_INCORRECT:
            EV_ERROR << "incorrect fcs in ethernet frame\n";
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

void EtherMacBase::flushQueue()
{
    // code would look slightly nicer with a pop() function that returns nullptr if empty
    if (txQueue.innerQueue) {
        while (!txQueue.innerQueue->isEmpty()) {
            cMessage *msg = static_cast<cMessage *>(txQueue.innerQueue->pop());
            PacketDropDetails details;
            details.setReason(INTERFACE_DOWN);
            emit(packetDroppedSignal, msg, &details);
            delete msg;
        }
    }
    else {
        while (!txQueue.extQueue->isEmpty()) {
            cMessage *msg = txQueue.extQueue->pop();
            PacketDropDetails details;
            details.setReason(INTERFACE_DOWN);
            emit(packetDroppedSignal, msg, &details);
            delete msg;
        }
        txQueue.extQueue->clear();    // clear request count
    }
}

void EtherMacBase::clearQueue()
{
    if (txQueue.innerQueue)
        txQueue.innerQueue->clear();
    else
        txQueue.extQueue->clear(); // clear request count
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

    if (frame->getDest().equals(address))
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
        if (interfaceEntry) {
            interfaceEntry->setCarrier(false);
            interfaceEntry->setDatarate(0);
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
        for (auto & etherDescr : etherDescrs) {
            if (txRate == etherDescr.txrate) {
                curEtherDescr = &(etherDescr);
                if (interfaceEntry) {
                    interfaceEntry->setCarrier(true);
                    interfaceEntry->setDatarate(txRate);
                }
                return;
            }
        }
        throw cRuntimeError("Invalid transmission rate %g bps on channel %s at module %s",
                txRate, transmissionChannel->getFullPath().c_str(), getFullPath().c_str());
    }
}

void EtherMacBase::printParameters()
{
    // Dump parameters
    EV_DETAIL << "MAC address: " << address << (promiscuous ? ", promiscuous mode" : "") << endl
              << "txrate: " << curEtherDescr->txrate << " bps, "
              << (duplexMode ? "full-duplex" : "half-duplex") << endl
              << "bitTime: " << 1e9 / curEtherDescr->txrate << " ns" << endl
              << "frameBursting: " << (frameBursting ? "on" : "off") << endl
              << "slotTime: " << curEtherDescr->slotTime << endl
              << "interFrameGap: " << INTERFRAME_GAP_BITS / curEtherDescr->txrate << endl
              << endl;
}

void EtherMacBase::getNextFrameFromQueue()
{
    ASSERT(nullptr == curTxFrame);
    if (txQueue.extQueue) {
        if (txQueue.extQueue->getNumPendingRequests() == 0)
            txQueue.extQueue->requestPacket();
    }
    else {
        if (!txQueue.innerQueue->isEmpty())
            curTxFrame = static_cast<Packet *>(txQueue.innerQueue->pop());
    }
}

void EtherMacBase::requestNextFrameFromExtQueue()
{
    ASSERT(nullptr == curTxFrame);
    if (txQueue.extQueue) {
        if (txQueue.extQueue->getNumPendingRequests() == 0)
            txQueue.extQueue->requestPacket();
    }
}

void EtherMacBase::finish()
{
    if (!disabled) {
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
}

int EtherMacBase::InnerQueue::packetCompare(cObject *a, cObject *b)
{
    Packet *ap = static_cast<Packet *>(a);
    Packet *bp = static_cast<Packet *>(b);
    const auto& ah = ap->peekAtFront<EthernetMacHeader>();
    const auto& bh = bp->peekAtFront<EthernetMacHeader>();
    int ac = (ah->getTypeOrLength() == ETHERTYPE_FLOW_CONTROL) ? 0 : 1;
    int bc = (bh->getTypeOrLength() == ETHERTYPE_FLOW_CONTROL) ? 0 : 1;
    return ac - bc;
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

} // namespace inet


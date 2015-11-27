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

#include "inet/linklayer/ethernet/EtherMACBase.h"

#include "inet/linklayer/ethernet/EtherFrame.h"
#include "inet/linklayer/ethernet/Ethernet.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/common/queue/IPassiveQueue.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/INETUtils.h"

namespace inet {

const double EtherMACBase::SPEED_OF_LIGHT_IN_CABLE = 200000000.0;

/*
   double      txrate;
   int         maxFramesInBurst;
   int64       maxBytesInBurst;
   int64       frameMinBytes;
   int64       otherFrameMinBytes;     // minimal frame length in burst mode, after first frame
 */

const EtherMACBase::EtherDescr EtherMACBase::nullEtherDescr = {
    0.0,
    0.0,
    0,
    0,
    0,
    0,
    0,
    0.0,
    0.0
};

const EtherMACBase::EtherDescr EtherMACBase::etherDescrs[NUM_OF_ETHERDESCRS] = {
    {
        ETHERNET_TXRATE,
        0.5 / ETHERNET_TXRATE,
        MIN_ETHERNET_FRAME_BYTES,
        0,
        0,
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
        0,
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
        0,
        -1,    // half-duplex is not supported
        0,
        0.0,
        0.0
    },
    {
        FOURTY_GIGABIT_ETHERNET_TXRATE,
        0.5 / FOURTY_GIGABIT_ETHERNET_TXRATE,
        MIN_ETHERNET_FRAME_BYTES,
        0,
        0,
        -1,    // half-duplex is not supported
        0,
        0.0,
        0.0
    },
    {
        HUNDRED_GIGABIT_ETHERNET_TXRATE,
        0.5 / HUNDRED_GIGABIT_ETHERNET_TXRATE,
        MIN_ETHERNET_FRAME_BYTES,
        0,
        0,
        -1,    // half-duplex is not supported
        0,
        0.0,
        0.0
    }
};

simsignal_t EtherMACBase::txPkSignal = registerSignal("txPk");
simsignal_t EtherMACBase::rxPkOkSignal = registerSignal("rxPkOk");
simsignal_t EtherMACBase::txPausePkUnitsSignal = registerSignal("txPausePkUnits");
simsignal_t EtherMACBase::rxPausePkUnitsSignal = registerSignal("rxPausePkUnits");
simsignal_t EtherMACBase::rxPkFromHLSignal = registerSignal("rxPkFromHL");
simsignal_t EtherMACBase::dropPkBitErrorSignal = registerSignal("dropPkBitError");
simsignal_t EtherMACBase::dropPkIfaceDownSignal = registerSignal("dropPkIfaceDown");
simsignal_t EtherMACBase::dropPkFromHLIfaceDownSignal = registerSignal("dropPkFromHLIfaceDown");
simsignal_t EtherMACBase::dropPkNotForUsSignal = registerSignal("dropPkNotForUs");

simsignal_t EtherMACBase::packetSentToLowerSignal = registerSignal("packetSentToLower");
simsignal_t EtherMACBase::packetReceivedFromLowerSignal = registerSignal("packetReceivedFromLower");
simsignal_t EtherMACBase::packetSentToUpperSignal = registerSignal("packetSentToUpper");
simsignal_t EtherMACBase::packetReceivedFromUpperSignal = registerSignal("packetReceivedFromUpper");

simsignal_t EtherMACBase::transmitStateSignal = registerSignal("transmitState");
simsignal_t EtherMACBase::receiveStateSignal = registerSignal("receiveState");

EtherMACBase::EtherMACBase()
{
    lastTxFinishTime = -1.0;    // never equals to current simtime
    curEtherDescr = &nullEtherDescr;
}

EtherMACBase::~EtherMACBase()
{
    delete curTxFrame;

    cancelAndDelete(endTxMsg);
    cancelAndDelete(endIFGMsg);
    cancelAndDelete(endPauseMsg);
}

void EtherMACBase::initialize(int stage)
{
    connectionColoring = par("connectionColoring");

    MACBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        physInGate = gate("phys$i");
        physOutGate = gate("phys$o");
        upperLayerInGate = gate("upperLayerIn");
        transmissionChannel = nullptr;
        curTxFrame = nullptr;

        initializeFlags();

        initializeMACAddress();
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
        registerInterface();    // needs MAC address
        initializeQueueModule();
        readChannelParameters(true);
    }
}

void EtherMACBase::initializeQueueModule()
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
        txQueue.setInternalQueue("txQueue", par("txQueueLimit").longValue());
    }
}

void EtherMACBase::initializeMACAddress()
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

void EtherMACBase::initializeFlags()
{
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

void EtherMACBase::initializeStatistics()
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

InterfaceEntry *EtherMACBase::createInterfaceEntry()
{
    InterfaceEntry *interfaceEntry = new InterfaceEntry(this);

    // generate a link-layer address to be used as interface token for IPv6
    interfaceEntry->setMACAddress(address);
    interfaceEntry->setInterfaceToken(address.formInterfaceIdentifier());
    //InterfaceToken token(0, getSimulation()->getUniqueNumber(), 64);
    //interfaceEntry->setInterfaceToken(token);

    // MTU: typical values are 576 (Internet de facto), 1500 (Ethernet-friendly),
    // 4000 (on some point-to-point links), 4470 (Cisco routers default, FDDI compatible)
    interfaceEntry->setMtu(par("mtu").longValue());

    // capabilities
    interfaceEntry->setMulticast(true);
    interfaceEntry->setBroadcast(true);

    return interfaceEntry;
}

bool EtherMACBase::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_LINK_LAYER) {
            initializeFlags();
            initializeMACAddress();
            initializeQueueModule();
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_LINK_LAYER) {
            connected = false;
            processConnectDisconnect();
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if ((NodeCrashOperation::Stage)stage == NodeCrashOperation::STAGE_CRASH) {
            connected = false;
            processConnectDisconnect();
        }
    }
    return MACBase::handleOperationStage(operation, stage, doneCallback);
}

void EtherMACBase::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG)
{
    Enter_Method_Silent();

    MACBase::receiveSignal(source, signalID, obj DETAILS_ARG_NAME);

    if (signalID != POST_MODEL_CHANGE)
        return;

    if (dynamic_cast<cPostPathCreateNotification *>(obj)) {
        cPostPathCreateNotification *gcobj = (cPostPathCreateNotification *)obj;

        if ((physOutGate == gcobj->pathStartGate) || (physInGate == gcobj->pathEndGate))
            refreshConnection();
    }
    else if (dynamic_cast<cPostPathCutNotification *>(obj)) {
        cPostPathCutNotification *gcobj = (cPostPathCutNotification *)obj;

        if ((physOutGate == gcobj->pathStartGate) || (physInGate == gcobj->pathEndGate))
            refreshConnection();
    }
    else if (transmissionChannel && dynamic_cast<cPostParameterChangeNotification *>(obj)) {    // note: we are subscribed to the channel object too
        cPostParameterChangeNotification *gcobj = (cPostParameterChangeNotification *)obj;
        if (transmissionChannel == gcobj->par->getOwner())
            refreshConnection();
    }
}

void EtherMACBase::processConnectDisconnect()
{
    if (!connected) {
        cancelEvent(endTxMsg);
        cancelEvent(endIFGMsg);
        cancelEvent(endPauseMsg);

        if (curTxFrame) {
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
                emit(dropPkIfaceDownSignal, msg);
                delete msg;
            }
        }

        transmitState = TX_IDLE_STATE;
        emit(transmitStateSignal, TX_IDLE_STATE);
        receiveState = RX_IDLE_STATE;
        emit(receiveStateSignal, RX_IDLE_STATE);
    }
}

void EtherMACBase::flushQueue()
{
    // code would look slightly nicer with a pop() function that returns nullptr if empty
    if (txQueue.innerQueue) {
        while (!txQueue.innerQueue->isEmpty()) {
            cMessage *msg = (cMessage *)txQueue.innerQueue->pop();
            emit(dropPkFromHLIfaceDownSignal, msg);
            delete msg;
        }
    }
    else {
        while (!txQueue.extQueue->isEmpty()) {
            cMessage *msg = txQueue.extQueue->pop();
            emit(dropPkFromHLIfaceDownSignal, msg);
            delete msg;
        }
        txQueue.extQueue->clear();    // clear request count
    }
}

void EtherMACBase::clearQueue()
{
    if (txQueue.innerQueue)
        txQueue.innerQueue->clear();
    else
        txQueue.extQueue->clear(); // clear request count
}

void EtherMACBase::refreshConnection()
{
    Enter_Method_Silent();

    bool oldConn = connected;
    readChannelParameters(false);

    if (oldConn != connected)
        processConnectDisconnect();
}

bool EtherMACBase::dropFrameNotForUs(EtherFrame *frame)
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

    bool isPause = (dynamic_cast<EtherPauseFrame *>(frame) != nullptr);

    if (!isPause && (promiscuous || frame->getDest().isMulticast()))
        return false;

    if (isPause && frame->getDest().equals(MACAddress::MULTICAST_PAUSE_ADDRESS))
        return false;

    EV_WARN << "Frame `" << frame->getName() << "' not destined to us, discarding\n";
    numDroppedNotForUs++;
    emit(dropPkNotForUsSignal, frame);
    delete frame;
    return true;
}

void EtherMACBase::readChannelParameters(bool errorWhenAsymmetric)
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

void EtherMACBase::printParameters()
{
    // Dump parameters
    EV_DETAIL << "MAC address: " << address << (promiscuous ? ", promiscuous mode" : "") << endl
              << "txrate: " << curEtherDescr->txrate << ", "
              << (duplexMode ? "full-duplex" : "half-duplex") << endl;
#if 1
    EV_DETAIL << "bitTime: " << 1.0 / curEtherDescr->txrate << endl;
    EV_DETAIL << "frameBursting: " << frameBursting << endl;
    EV_DETAIL << "slotTime: " << curEtherDescr->slotTime << endl;
    EV_DETAIL << "interFrameGap: " << INTERFRAME_GAP_BITS / curEtherDescr->txrate << endl;
    EV_DETAIL << endl;
#endif // if 1
}

void EtherMACBase::getNextFrameFromQueue()
{
    ASSERT(nullptr == curTxFrame);
    if (txQueue.extQueue) {
        if (txQueue.extQueue->getNumPendingRequests() == 0)
            txQueue.extQueue->requestPacket();
    }
    else {
        if (!txQueue.innerQueue->isEmpty())
            curTxFrame = (EtherFrame *)txQueue.innerQueue->pop();
    }
}

void EtherMACBase::requestNextFrameFromExtQueue()
{
    ASSERT(nullptr == curTxFrame);
    if (txQueue.extQueue) {
        if (txQueue.extQueue->getNumPendingRequests() == 0)
            txQueue.extQueue->requestPacket();
    }
}

void EtherMACBase::finish()
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

void EtherMACBase::updateDisplayString()
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

    if (!strcmp(getParentModule()->getClassName(), "EthernetInterface"))
        getParentModule()->getDisplayString().setTagArg("i", 1, color);

    // connection coloring
    updateConnectionColor(transmitState);

#if 0
    // this code works but didn't turn out to be very useful
    const char *txStateName;

    switch (transmitState) {
        case TX_IDLE_STATE:
            txStateName = "IDLE";
            break;

        case WAIT_IFG_STATE:
            txStateName = "WAIT_IFG";
            break;

        case SEND_IFG_STATE:
            txStateName = "SEND_IFG";
            break;

        case TRANSMITTING_STATE:
            txStateName = "TX";
            break;

        case JAMMING_STATE:
            txStateName = "JAM";
            break;

        case BACKOFF_STATE:
            txStateName = "BACKOFF";
            break;

        case PAUSE_STATE:
            txStateName = "PAUSE";
            break;

        default:
            throw cRuntimeError("wrong tx state");
    }

    const char *rxStateName;

    switch (receiveState) {
        case RX_IDLE_STATE:
            rxStateName = "IDLE";
            break;

        case RECEIVING_STATE:
            rxStateName = "RX";
            break;

        case RX_COLLISION_STATE:
            rxStateName = "COLL";
            break;

        default:
            throw cRuntimeError("wrong rx state");
    }

    char buf[80];
    sprintf(buf, "tx:%s rx: %s\n#boff:%d #cTx:%d",
            txStateName, rxStateName, backoffs, numConcurrentTransmissions);
    getDisplayString().setTagArg("t", 0, buf);
#endif // if 0
}

void EtherMACBase::updateConnectionColor(int txState)
{
    const char *color;

    if (txState == TRANSMITTING_STATE)
        color = "yellow";
    else if (txState == JAMMING_STATE || txState == BACKOFF_STATE)
        color = "red";
    else
        color = "";

    if (hasGUI() && connectionColoring) {
        if (connected) {
            transmissionChannel->getDisplayString().setTagArg("ls", 0, color);
            transmissionChannel->getDisplayString().setTagArg("ls", 1, color[0] ? "3" : "1");
        }
        else {
            // we are not connected: gray out our icon
            getDisplayString().setTagArg("i", 1, "#707070");
            getDisplayString().setTagArg("i", 2, "100");
        }
    }
}

int EtherMACBase::InnerQueue::packetCompare(cObject *a, cObject *b)
{
    int ap = (dynamic_cast<EtherPauseFrame *>(a) == nullptr) ? 1 : 0;
    int bp = (dynamic_cast<EtherPauseFrame *>(b) == nullptr) ? 1 : 0;
    return ap - bp;
}

} // namespace inet


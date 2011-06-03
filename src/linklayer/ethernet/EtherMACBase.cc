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

#include "EtherMACBase.h"

#include "EtherFrame_m.h"
#include "Ethernet.h"
#include "InterfaceEntry.h"
#include "InterfaceTableAccess.h"
#include "IPassiveQueue.h"
#include "NotificationBoard.h"
#include "opp_utils.h"


static const double SPEED_OF_LIGHT_IN_CABLE = 200000000.0;

/*
double      txrate;
int         maxFramesInBurst;
int64       maxBytesInBurst;
int64       frameMinBytes;
int64       otherFrameMinBytes;     // minimal frame length in burst mode, after first frame
simtime_t   halfBitTime;          // transmission time of a half bit
simtime_t   slotTime;             // slot time
simtime_t   shortestFrameDuration;// precalculated from MIN_ETHERNET_FRAME or GIGABIT_MIN_FRAME_WITH_EXT
*/

const EtherMACBase::EtherDescr EtherMACBase::nullEtherDescr =
{
    0,
    0,
    0,
    0,
    0,
    0.0,
    0.0,
    0.0
};

const EtherMACBase::EtherDescr EtherMACBase::etherDescrs[NUM_OF_ETHERDESCRS] =
{
    {
        ETHERNET_TXRATE,
        0,
        0,
        MIN_ETHERNET_FRAME,
        0,
        0.5 / ETHERNET_TXRATE,
        512.0 / ETHERNET_TXRATE,
        MIN_ETHERNET_FRAME / ETHERNET_TXRATE
    },
    {
        FAST_ETHERNET_TXRATE,
        0,
        0,
        MIN_ETHERNET_FRAME,
        0,
        0.5 / FAST_ETHERNET_TXRATE,
        512.0 / ETHERNET_TXRATE,
        MIN_ETHERNET_FRAME / FAST_ETHERNET_TXRATE
    },
    {
        GIGABIT_ETHERNET_TXRATE,
        MAX_PACKETBURST,
        GIGABIT_MAX_BURST_BYTES,
        GIGABIT_MIN_FRAME_WITH_EXT,
        MIN_ETHERNET_FRAME,
        0.5 / GIGABIT_ETHERNET_TXRATE,
        512.0 / GIGABIT_ETHERNET_TXRATE,
        GIGABIT_MIN_FRAME_WITH_EXT / GIGABIT_ETHERNET_TXRATE
    },
    {
        FAST_GIGABIT_ETHERNET_TXRATE,
        MAX_PACKETBURST,
        GIGABIT_MAX_BURST_BYTES,
        GIGABIT_MIN_FRAME_WITH_EXT,
        MIN_ETHERNET_FRAME,
        0.5 / FAST_GIGABIT_ETHERNET_TXRATE,
        512.0 / GIGABIT_ETHERNET_TXRATE,
        GIGABIT_MIN_FRAME_WITH_EXT / FAST_GIGABIT_ETHERNET_TXRATE
    }
};

simsignal_t EtherMACBase::txPkBytesSignal = SIMSIGNAL_NULL;
simsignal_t EtherMACBase::rxPkBytesOkSignal = SIMSIGNAL_NULL;
simsignal_t EtherMACBase::passedUpPkBytesSignal = SIMSIGNAL_NULL;
simsignal_t EtherMACBase::txPausePkUnitsSignal = SIMSIGNAL_NULL;
simsignal_t EtherMACBase::rxPausePkUnitsSignal = SIMSIGNAL_NULL;
simsignal_t EtherMACBase::rxPkBytesFromHLSignal = SIMSIGNAL_NULL;
simsignal_t EtherMACBase::droppedPkBytesNotForUsSignal = SIMSIGNAL_NULL;
simsignal_t EtherMACBase::droppedPkBytesBitErrorSignal = SIMSIGNAL_NULL;
simsignal_t EtherMACBase::droppedPkBytesIfaceDownSignal = SIMSIGNAL_NULL;

simsignal_t EtherMACBase::packetSentToLowerSignal = SIMSIGNAL_NULL;
simsignal_t EtherMACBase::packetReceivedFromLowerSignal = SIMSIGNAL_NULL;
simsignal_t EtherMACBase::packetSentToUpperSignal = SIMSIGNAL_NULL;
simsignal_t EtherMACBase::packetReceivedFromUpperSignal = SIMSIGNAL_NULL;

bool EtherMACBase::MacQueue::isEmpty()
{
    return innerQueue ? innerQueue->queue.empty() : extQueue->isEmpty();
}

EtherMACBase::EtherMACBase()
{
    lastTxFinishTime = -1.0; // never equals with current simtime.
    curEtherDescr = &nullEtherDescr;
    transmissionChannel = NULL;
    physInGate = NULL;
    physOutGate = NULL;
    interfaceEntry = NULL;
    nb = NULL;
    curTxFrame = NULL;
    endTxMsg = endIFGMsg = endPauseMsg = NULL;
}

EtherMACBase::~EtherMACBase()
{
    delete curTxFrame;

    cancelAndDelete(endTxMsg);
    cancelAndDelete(endIFGMsg);
    cancelAndDelete(endPauseMsg);
}

void EtherMACBase::initialize()
{
    physInGate = gate("phys$i");
    physOutGate = gate("phys$o");
    transmissionChannel = NULL;
    interfaceEntry = NULL;
    curTxFrame = NULL;

    initializeFlags();

    initializeMACAddress();
    initializeQueueModule();
    initializeNotificationBoard();
    initializeStatistics();

    registerInterface(); // needs MAC address

    calculateParameters();

    lastTxFinishTime = -1.0; // never equals with current simtime.

    // initialize self messages
    endTxMsg = new cMessage("EndTransmission", ENDTRANSMISSION);
    endIFGMsg = new cMessage("EndIFG", ENDIFG);
    endPauseMsg = new cMessage("EndPause", ENDPAUSE);

    // initialize states
    transmitState = TX_IDLE_STATE;
    receiveState = RX_IDLE_STATE;
    WATCH(transmitState);
    WATCH(receiveState);

    // initalize pause
    pauseUnitsRequested = 0;
    WATCH(pauseUnitsRequested);

    subscribe(POST_MODEL_CHANGE, this);
}

void EtherMACBase::initializeQueueModule()
{
    if (par("queueModule").stringValue()[0])
    {
        cModule *module = getParentModule()->getSubmodule(par("queueModule").stringValue());
        IPassiveQueue *queueModule = check_and_cast<IPassiveQueue *>(module);
        EV << "Requesting first frame from queue module\n";
        txQueue.setExternalQueue(queueModule);

        if (0 == txQueue.extQueue->getNumPendingRequests())
            txQueue.extQueue->requestPacket();
    }
    else
    {
        txQueue.setInternalQueue("txQueue", par("txQueueLimit"));
    }
}

void EtherMACBase::initializeMACAddress()
{
    const char *addrstr = par("address");

    if (!strcmp(addrstr, "auto"))
    {
        // assign automatic address
        address = MACAddress::generateAutoAddress();

        // change module parameter from "auto" to concrete address
        par("address").setStringValue(address.str().c_str());
    }
    else
    {
        address.setAddress(addrstr);
    }
}

void EtherMACBase::initializeNotificationBoard()
{
    hasSubscribers = false;

    if (interfaceEntry)
    {
        nb = NotificationBoardAccess().getIfExists();
        notifDetails.setInterfaceEntry(interfaceEntry);
        nb->subscribe(this, NF_SUBSCRIBERLIST_CHANGED);
        updateHasSubcribers();
    }
}

void EtherMACBase::initializeFlags()
{
    duplexMode = true;

    // initialize connected flag
    connected = physOutGate->getPathEndGate()->isConnected();

    if (!connected)
        EV << "MAC not connected to a network.\n";

    WATCH(connected);

    // TODO: this should be settable from the gui
    // initialize disabled flag
    // Note: it is currently not supported to enable a disabled MAC at runtime.
    // Difficulties: (1) autoconfig (2) how to pick up channel state (free, tx, collision etc)
    disabled = false;
    WATCH(disabled);

    // initialize promiscuous flag
    promiscuous = par("promiscuous");
    WATCH(promiscuous);

    frameBursting = par("frameBursting");
    WATCH(frameBursting);
}

void EtherMACBase::initializeStatistics()
{
    framesSentInBurst = 0;
    bytesSentInBurst = 0;

    numFramesSent = numFramesReceivedOK = numBytesSent = numBytesReceivedOK = 0;
    numFramesPassedToHL = numDroppedBitError = numDroppedNotForUs = 0;
    numFramesFromHL = numDroppedIfaceDown = 0;
    numPauseFramesRcvd = numPauseFramesSent = 0;

    WATCH(framesSentInBurst);
    WATCH(bytesSentInBurst);

    WATCH(numFramesSent);
    WATCH(numFramesReceivedOK);
    WATCH(numBytesSent);
    WATCH(numBytesReceivedOK);
    WATCH(numFramesFromHL);
    WATCH(numDroppedIfaceDown);
    WATCH(numDroppedBitError);
    WATCH(numDroppedNotForUs);
    WATCH(numFramesPassedToHL);
    WATCH(numPauseFramesRcvd);
    WATCH(numPauseFramesSent);

    txPkBytesSignal = registerSignal("txPkBytes");
    rxPkBytesOkSignal = registerSignal("rxPkBytesOk");
    txPausePkUnitsSignal = registerSignal("txPausePkUnits");
    rxPausePkUnitsSignal = registerSignal("rxPausePkUnits");
    rxPkBytesFromHLSignal = registerSignal("rxPkBytesFromHL");
    droppedPkBytesBitErrorSignal = registerSignal("droppedPkBytesBitError");
    droppedPkBytesIfaceDownSignal = registerSignal("droppedPkBytesIfaceDown");
    droppedPkBytesNotForUsSignal = registerSignal("droppedPkBytesNotForUs");
    passedUpPkBytesSignal = registerSignal("passedUpPkBytes");

    packetSentToLowerSignal = registerSignal("packetSentToLower");
    packetReceivedFromLowerSignal = registerSignal("packetReceivedFromLower");
    packetSentToUpperSignal = registerSignal("packetSentToUpper");
    packetReceivedFromUpperSignal = registerSignal("packetReceivedFromUpper");
}

void EtherMACBase::registerInterface()
{
    interfaceEntry = new InterfaceEntry();

    // interface name: NIC module's name without special characters ([])
    interfaceEntry->setName(OPP_Global::stripnonalnum(getParentModule()->getFullName()).c_str());

    // generate a link-layer address to be used as interface token for IPv6
    interfaceEntry->setMACAddress(address);
    interfaceEntry->setInterfaceToken(address.formInterfaceIdentifier());
    //InterfaceToken token(0, simulation.getUniqueNumber(), 64);
    //interfaceEntry->setInterfaceToken(token);

    // MTU: typical values are 576 (Internet de facto), 1500 (Ethernet-friendly),
    // 4000 (on some point-to-point links), 4470 (Cisco routers default, FDDI compatible)
    interfaceEntry->setMtu(par("mtu"));

    // capabilities
    interfaceEntry->setMulticast(true);
    interfaceEntry->setBroadcast(true);

    // add
    IInterfaceTable *ift = InterfaceTableAccess().getIfExists();

    if (ift)
        ift->addInterface(interfaceEntry, this);
}

void EtherMACBase::receiveSignal(cComponent *src, simsignal_t id, cObject *obj)
{
    if (dynamic_cast<cPostPathCreateNotification *>(obj))
    {
        cPostPathCreateNotification *gcobj = (cPostPathCreateNotification *)obj;

        if ((physOutGate == gcobj->pathStartGate) || (physInGate == gcobj->pathEndGate))
            refreshConnection(true);
    }
    else if (dynamic_cast<cPostPathCutNotification *>(obj))
    {
        cPostPathCutNotification *gcobj = (cPostPathCutNotification *)obj;

        if ((physOutGate == gcobj->pathStartGate) || (physInGate == gcobj->pathEndGate))
            refreshConnection(false);
    }
    else if (transmissionChannel && dynamic_cast<cPostParameterChangeNotification *>(obj))
    {
        cPostParameterChangeNotification *gcobj = (cPostParameterChangeNotification *)obj;
        if (transmissionChannel == gcobj->par->getOwner() && !strcmp("datarate", gcobj->par->getName()))
            refreshConnection(true);
    }
}

void EtherMACBase::refreshConnection(bool connected_par)
{
    Enter_Method_Silent();

    calculateParameters();

    if (!connected)
    {
        // cMessage *endTxMsg, *endIFGMsg, *endPauseMsg;
        cancelEvent(endTxMsg);
        cancelEvent(endIFGMsg);
        cancelEvent(endPauseMsg);

        if (curTxFrame)
        {
            delete curTxFrame;
            curTxFrame = NULL;
            lastTxFinishTime = simTime();
        }

        if (txQueue.extQueue)
        {
            // Clear external queue: send a request, and received packet will be deleted in handleMessage()
            if (0 == txQueue.extQueue->getNumPendingRequests())
                txQueue.extQueue->requestPacket();
        }
        else
        {
            //Clear inner queue
            while (!txQueue.innerQueue->queue.empty())
            {
                cMessage *msg = check_and_cast<cMessage *>(txQueue.innerQueue->queue.pop());
                EV << "Interface is not connected, dropping packet " << msg << endl;
                numDroppedIfaceDown++;
                emit(droppedPkBytesIfaceDownSignal, msg->isPacket() ? (long)(((cPacket*)msg)->getByteLength()) : 0L);
                delete msg;
            }
        }

        transmitState = TX_IDLE_STATE;
        receiveState = RX_IDLE_STATE;
    }
}

bool EtherMACBase::checkDestinationAddress(EtherFrame *frame)
{
    // If not set to promiscuous = on, then checks if received frame contains destination MAC address
    // matching port's MAC address, also checks if broadcast bit is set
    if (!promiscuous && !frame->getDest().isBroadcast() && !frame->getDest().equals(address))
    {
        EV << "Frame `" << frame->getName() <<"' not destined to us, discarding\n";
        numDroppedNotForUs++;
        emit(droppedPkBytesNotForUsSignal, (long)(frame->getByteLength()));
        delete frame;

        return false;
    }

    return true;
}

void EtherMACBase::calculateParameters()
{
    ASSERT(physInGate == gate("phys$i"));
    ASSERT(physOutGate == gate("phys$o"));

    cChannel *outTrChannel = physOutGate->findTransmissionChannel();
    cChannel *inTrChannel = physInGate->findIncomingTransmissionChannel();

    connected = (outTrChannel != NULL) && (inTrChannel != NULL);

    if (!connected)
    {
        curEtherDescr = &nullEtherDescr;
        carrierExtension = false;

        if (transmissionChannel)
            transmissionChannel->forceTransmissionFinishTime(SimTime());

        transmissionChannel = NULL;
        interfaceEntry->setDown(true);
        interfaceEntry->setDatarate(0);
        return;
    }

    if (outTrChannel && !transmissionChannel)
        outTrChannel->subscribe(POST_MODEL_CHANGE, this);

    transmissionChannel = outTrChannel;
    carrierExtension = false; // FIXME
    double txRate = transmissionChannel->getNominalDatarate();
    double rxRate = inTrChannel->getNominalDatarate();

    if (txRate != rxRate)
    {
        if (!initialized())
        {
            throw cRuntimeError(this, "The input/output datarates are differs (%g / %g bps)",
                                rxRate, txRate);
        }
        else
        {
            ev << "The input/output datarates are differs (" << rxRate << " / " << txRate
                    << " bps), input rate changed to " << txRate << "bps.\n";
            inTrChannel->par("datarate").setDoubleValue(txRate);
        }
    }

    // Check valid speeds
    for (int i = 0; i < NUM_OF_ETHERDESCRS; i++)
    {
        if (txRate == etherDescrs[i].txrate)
        {
            curEtherDescr = &etherDescrs[i];
            interfaceEntry->setDown(false);
            interfaceEntry->setDatarate(txRate);
            return;
        }
    }

    throw cRuntimeError(this, "Invalid transmission rate %g on channel %s at %s modul",
                        txRate, transmissionChannel->getFullPath().c_str(), getFullPath().c_str());
}

void EtherMACBase::printParameters()
{
    // Dump parameters
    EV << "MAC address: " << address << (promiscuous ? ", promiscuous mode" : "") << endl
       << "txrate: " << curEtherDescr->txrate << ", "
       << (duplexMode ? "full-duplex" : "half-duplex") << endl;
#if 0
    EV << "bitTime: " << bitTime << endl;
    EV << "carrierExtension: " << carrierExtension << endl;
    EV << "frameBursting: " << frameBursting << endl;
    EV << "slotTime: " << slotTime << endl;
    EV << "interFrameGap: " << interFrameGap << endl;
    EV << endl;
#endif
}

void EtherMACBase::processFrameFromUpperLayer(EtherFrame *frame)
{
    EV << "Received frame from upper layer: " << frame << endl;

    emit(packetReceivedFromUpperSignal, frame);

    if (frame->getDest().equals(address))
    {
        error("logic error: frame %s from higher layer has local MAC address as dest (%s)",
                frame->getFullName(), frame->getDest().str().c_str());
    }

    if (frame->getByteLength() > MAX_ETHERNET_FRAME)
    {
        error("packet from higher layer (%d bytes) exceeds maximum Ethernet frame size (%d)",
                (int)(frame->getByteLength()), MAX_ETHERNET_FRAME);
    }

    // fill in src address if not set
    if (frame->getSrc().isUnspecified())
        frame->setSrc(address);

    bool isPauseFrame = (dynamic_cast<EtherPauseFrame*>(frame) != NULL);

    if (!isPauseFrame)
    {
        numFramesFromHL++;
        emit(rxPkBytesFromHLSignal, (long)(frame->getByteLength()));
    }

    if (txQueue.extQueue)
    {
        ASSERT(curTxFrame == NULL);
        curTxFrame = frame;
    }
    else
    {
        if (!isPauseFrame)
        {
            if (txQueue.innerQueue->queueLimit
                    && txQueue.innerQueue->queue.length() > txQueue.innerQueue->queueLimit)
            {
                error("txQueue length exceeds %d -- this is probably due to "
                      "a bogus app model generating excessive traffic "
                      "(or if this is normal, increase txQueueLimit!)",
                      txQueue.innerQueue->queueLimit);
            }

            // store frame and possibly begin transmitting
            EV << "Packet " << frame << " arrived from higher layers, enqueueing\n";
            txQueue.innerQueue->queue.insert(frame);
        }
        else
        {
            EV << "PAUSE received from higher layer\n";

            // PAUSE frames enjoy priority -- they're transmitted before all other frames queued up
            if (!txQueue.innerQueue->queue.empty())
                // front() frame is probably being transmitted
                txQueue.innerQueue->queue.insertBefore(txQueue.innerQueue->queue.front(), frame);
            else
                txQueue.innerQueue->queue.insert(frame);
        }

        if (NULL == curTxFrame && !txQueue.innerQueue->queue.empty())
            curTxFrame = (EtherFrame*)txQueue.innerQueue->queue.pop();
    }
}

void EtherMACBase::processMsgFromNetwork(EtherTraffic *frame)
{
    EV << "Received frame from network: " << frame << endl;

    // detect cable length violation in half-duplex mode
    if (!duplexMode)
    {
        if (simTime() - frame->getSendingTime() >= curEtherDescr->shortestFrameDuration)
        {
            error("very long frame propagation time detected, "
                  "maybe cable exceeds maximum allowed length? "
                  "(%lgs corresponds to an approx. %lgm cable)",
                  SIMTIME_STR(simTime() - frame->getSendingTime()),
                  SIMTIME_STR((simTime() - frame->getSendingTime()) * SPEED_OF_LIGHT_IN_CABLE));
        }
    }
}

void EtherMACBase::frameReceptionComplete(EtherTraffic *frame)
{
    int pauseUnits;
    EtherPauseFrame *pauseFrame;

    if ((pauseFrame = dynamic_cast<EtherPauseFrame*>(frame)) != NULL)
    {
        pauseUnits = pauseFrame->getPauseTime();
        delete frame;
        numPauseFramesRcvd++;
        emit(rxPausePkUnitsSignal, pauseUnits);
        processPauseCommand(pauseUnits);
    }
    else if ((dynamic_cast<EtherPadding*>(frame)) != NULL)
    {
        delete frame;
    }
    else
    {
        processReceivedDataFrame((EtherFrame *)frame);
    }
}

void EtherMACBase::processReceivedDataFrame(EtherFrame *frame)
{
    emit(packetReceivedFromLowerSignal, frame);

    // bit errors
    if (frame->hasBitError())
    {
        numDroppedBitError++;
        emit(droppedPkBytesBitErrorSignal, (long)(frame->getByteLength()));
        delete frame;
        return;
    }

    // restore original byte length (strip preamble and SFD and external bytes)
    frame->setByteLength(frame->getOrigByteLength());

    // statistics
    unsigned long curBytes = frame->getByteLength();
    numFramesReceivedOK++;
    numBytesReceivedOK += curBytes;
    emit(rxPkBytesOkSignal, curBytes);

    if (!checkDestinationAddress(frame))
        return;

    numFramesPassedToHL++;
    emit(passedUpPkBytesSignal, curBytes);

    emit(packetSentToUpperSignal, frame);
    // pass up to upper layer
    send(frame, "upperLayerOut");
}

void EtherMACBase::processPauseCommand(int pauseUnits)
{
    if (transmitState == TX_IDLE_STATE)
    {
        EV << "PAUSE frame received, pausing for " << pauseUnitsRequested << " time units\n";
        if (pauseUnits > 0)
            scheduleEndPausePeriod(pauseUnits);
    }
    else if (transmitState == PAUSE_STATE)
    {
        EV << "PAUSE frame received, pausing for " << pauseUnitsRequested
           << " more time units from now\n";
        cancelEvent(endPauseMsg);

        if (pauseUnits > 0)
            scheduleEndPausePeriod(pauseUnits);
    }
    else
    {
        // transmitter busy -- wait until it finishes with current frame (endTx)
        // and then it'll go to PAUSE state
        EV << "PAUSE frame received, storing pause request\n";
        pauseUnitsRequested = pauseUnits;
    }
}

void EtherMACBase::handleEndIFGPeriod()
{
    if (transmitState != WAIT_IFG_STATE && transmitState != SEND_IFG_STATE)
        error("Not in WAIT_IFG_STATE at the end of IFG period");

    if (NULL == curTxFrame)
        error("End of IFG and no frame to transmit");

    // End of IFG period, okay to transmit, if Rx idle OR duplexMode
    EV << "IFG elapsed, now begin transmission of frame " << curTxFrame << endl;

    // Perform carrier extension if in Gigabit Ethernet
    if (carrierExtension && curTxFrame->getByteLength() < GIGABIT_MIN_FRAME_WITH_EXT)
    {
        EV << "Performing carrier extension of small frame\n";
        curTxFrame->setByteLength(GIGABIT_MIN_FRAME_WITH_EXT);
    }
}

void EtherMACBase::handleEndTxPeriod()
{
    // we only get here if transmission has finished successfully, without collision
    if (transmitState != TRANSMITTING_STATE || (!duplexMode && receiveState != RX_IDLE_STATE))
        error("End of transmission, and incorrect state detected");

    if (NULL == curTxFrame)
        error("Frame under transmission cannot be found");

    unsigned long curBytes = curTxFrame->getByteLength();
    numFramesSent++;
    numBytesSent += curBytes;
    emit(txPkBytesSignal, curBytes);

    if (dynamic_cast<EtherPauseFrame*>(curTxFrame) != NULL)
    {
        numPauseFramesSent++;
        emit(txPausePkUnitsSignal, ((EtherPauseFrame*)curTxFrame)->getPauseTime());
    }

    EV << "Transmission of " << curTxFrame << " successfully completed\n";
    delete curTxFrame;
    curTxFrame = NULL;
    lastTxFinishTime = simTime();
    getNextFrameFromQueue();
}

void EtherMACBase::getNextFrameFromQueue()
{
    if (txQueue.extQueue)
    {
        if (0 == txQueue.extQueue->getNumPendingRequests())
            txQueue.extQueue->requestPacket();
    }
    else
    {
        if (!txQueue.innerQueue->queue.empty())
            curTxFrame = (EtherFrame*)txQueue.innerQueue->queue.pop();
    }
}

void EtherMACBase::prepareTxFrame(EtherFrame *frame)
{
    if (frame->getSrc().isUnspecified())
        frame->setSrc(address);

    // add preamble and SFD (Starting Frame Delimiter), then send out
    frame->setOrigByteLength(frame->getByteLength());
    frame->addByteLength(PREAMBLE_BYTES+SFD_BYTES);
    bool inBurst = frameBursting && framesSentInBurst;
    int64 minFrameLength =
            inBurst ? curEtherDescr->frameInBurstMinBytes : curEtherDescr->frameMinBytes;

    if (frame->getByteLength() < minFrameLength)
    {
        frame->setByteLength(minFrameLength);
    }
}

void EtherMACBase::handleEndPausePeriod()
{
    if (transmitState != PAUSE_STATE)
        error("At end of PAUSE not in PAUSE_STATE!");

    EV << "Pause finished, resuming transmissions\n";
    beginSendFrames();
}

void EtherMACBase::processMessageWhenNotConnected(cMessage *msg)
{
    EV << "Interface is not connected -- dropping packet " << msg << endl;
    emit(droppedPkBytesIfaceDownSignal, (long)(PK(msg)->getByteLength()));
    numDroppedIfaceDown++;
    delete msg;

    if (txQueue.extQueue)
    {
        if (0 == txQueue.extQueue->getNumPendingRequests())
            txQueue.extQueue->requestPacket();
    }
}

void EtherMACBase::processMessageWhenDisabled(cMessage *msg)
{
    EV << "MAC is disabled -- dropping message " << msg << endl;
    delete msg;

    if (txQueue.extQueue)
    {
        if (0 == txQueue.extQueue->getNumPendingRequests())
            txQueue.extQueue->requestPacket();
    }
}

void EtherMACBase::scheduleEndIFGPeriod()
{
    ASSERT(curTxFrame);

    if (frameBursting
            && (simTime() == lastTxFinishTime)
            && (framesSentInBurst < curEtherDescr->maxFramesInBurst)
            && (bytesSentInBurst + (INTERFRAME_GAP_BITS / 8) + curTxFrame->getByteLength()
                    <= curEtherDescr->maxBytesInBurst)
       )
    {
        EtherPadding *gap = new EtherPadding("IFG");
        gap->setBitLength(INTERFRAME_GAP_BITS);
        bytesSentInBurst += gap->getByteLength();
        send(gap, physOutGate);
        transmitState = SEND_IFG_STATE;
        scheduleAt(transmissionChannel->getTransmissionFinishTime(), endIFGMsg);
        // FIXME Check collision?
    }
    else
    {
        EtherPadding gap;
        gap.setBitLength(INTERFRAME_GAP_BITS);
        bytesSentInBurst = 0;
        framesSentInBurst = 0;
        transmitState = WAIT_IFG_STATE;
        scheduleAt(simTime() + transmissionChannel->calculateDuration(&gap), endIFGMsg);
    }
}

void EtherMACBase::scheduleEndTxPeriod(cPacket *frame)
{
    // update burst variables
    if (frameBursting)
    {
        bytesSentInBurst += frame->getByteLength();
        framesSentInBurst++;
    }

    scheduleAt(transmissionChannel->getTransmissionFinishTime(), endTxMsg);
    transmitState = TRANSMITTING_STATE;
}

void EtherMACBase::scheduleEndPausePeriod(int pauseUnits)
{
    // length is interpreted as 512-bit-time units
    cPacket pause;
    pause.setBitLength(pauseUnits * PAUSE_BITTIME);
    simtime_t pausePeriod = transmissionChannel->calculateDuration(&pause);
    scheduleAt(simTime() + pausePeriod, endPauseMsg);
    transmitState = PAUSE_STATE;
}

bool EtherMACBase::checkAndScheduleEndPausePeriod()
{
    if (pauseUnitsRequested > 0)
    {
        // if we received a PAUSE frame recently, go into PAUSE state
        EV << "Going to PAUSE mode for " << pauseUnitsRequested << " time units\n";

        scheduleEndPausePeriod(pauseUnitsRequested);
        pauseUnitsRequested = 0;
        return true;
    }

    return false;
}

void EtherMACBase::beginSendFrames()
{
    if (curTxFrame)
    {
        // Other frames are queued, therefore wait IFG period and transmit next frame
        EV << "Transmit next frame in output queue, after IFG period\n";
        scheduleEndIFGPeriod();
    }
    else
    {
        transmitState = TX_IDLE_STATE;
        // No more frames set transmitter to idle
        EV << "No more frames to send, transmitter set to idle\n";
    }
}

void EtherMACBase::fireChangeNotification(int type, cPacket *msg)
{
    if (nb)
    {
        notifDetails.setPacket(msg);
        nb->fireChangeNotification(type, &notifDetails);
    }
}

void EtherMACBase::finish()
{
    if (!disabled)
    {
        simtime_t t = simTime();
        recordScalar("simulated time", t);
        recordScalar("full duplex", duplexMode);

        if (t > 0)
        {
            recordScalar("frames/sec sent", numFramesSent / t);
            recordScalar("frames/sec rcvd", numFramesReceivedOK / t);
            recordScalar("bits/sec sent",   (8.0 * numBytesSent) / t);
            recordScalar("bits/sec rcvd",   (8.0 * numBytesReceivedOK) / t);
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

    switch (transmitState)
    {
        case TX_IDLE_STATE:      txStateName = "IDLE"; break;
        case WAIT_IFG_STATE:     txStateName = "WAIT_IFG"; break;
        case SEND_IFG_STATE:     txStateName = "SEND_IFG"; break;
        case TRANSMITTING_STATE: txStateName = "TX"; break;
        case JAMMING_STATE:      txStateName = "JAM"; break;
        case BACKOFF_STATE:      txStateName = "BACKOFF"; break;
        case PAUSE_STATE:        txStateName = "PAUSE"; break;
        default: error("wrong tx state");
    }

    const char *rxStateName;

    switch (receiveState)
    {
        case RX_IDLE_STATE:      rxStateName = "IDLE"; break;
        case RECEIVING_STATE:    rxStateName = "RX"; break;
        case RX_COLLISION_STATE: rxStateName = "COLL"; break;
        default: error("wrong rx state");
    }

    char buf[80];
    sprintf(buf, "tx:%s rx: %s\n#boff:%d #cTx:%d",
                 txStateName, rxStateName, backoffs, numConcurrentTransmissions);
    getDisplayString().setTagArg("t", 0, buf);
#endif
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

    if (ev.isGUI())
    {
        if (connected)
        {
            transmissionChannel->getDisplayString().setTagArg("ls", 0, color);
            transmissionChannel->getDisplayString().setTagArg("ls", 1, color[0] ? "3" : "1");
        }
        else
        {
            // we are not connected: gray out our icon
            getDisplayString().setTagArg("i", 1, "#707070");
            getDisplayString().setTagArg("i", 2, "100");
        }
    }
}

void EtherMACBase::receiveChangeNotification(int category, const cPolymorphic *)
{
    if (category == NF_SUBSCRIBERLIST_CHANGED)
        updateHasSubcribers();
}

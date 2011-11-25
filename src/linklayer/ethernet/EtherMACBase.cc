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


const double EtherMACBase::SPEED_OF_LIGHT_IN_CABLE = 200000000.0;

/*
double      txrate;
int         maxFramesInBurst;
int64       maxBytesInBurst;
int64       frameMinBytes;
int64       otherFrameMinBytes;     // minimal frame length in burst mode, after first frame
*/

const EtherMACBase::EtherDescr EtherMACBase::nullEtherDescr =
{
    0,
    0,
    0,
    0,
    0
};

const EtherMACBase::EtherDescr EtherMACBase::etherDescrs[NUM_OF_ETHERDESCRS] =
{
    {
        ETHERNET_TXRATE,
        0,
        0,
        MIN_ETHERNET_FRAME,
        MIN_ETHERNET_FRAME
    },
    {
        FAST_ETHERNET_TXRATE,
        0,
        0,
        MIN_ETHERNET_FRAME,
        MIN_ETHERNET_FRAME
    },
    {
        GIGABIT_ETHERNET_TXRATE,
        MAX_PACKETBURST,
        GIGABIT_MAX_BURST_BYTES,
        MIN_ETHERNET_FRAME,
        MIN_ETHERNET_FRAME
    },
    {
        FAST_GIGABIT_ETHERNET_TXRATE,
        0,
        0,
        MIN_ETHERNET_FRAME,
        MIN_ETHERNET_FRAME
    },
    {
        FOURTY_GIGABIT_ETHERNET_TXRATE,
        0,
        0,
        MIN_ETHERNET_FRAME,
        MIN_ETHERNET_FRAME
    },
    {
        HUNDRED_GIGABIT_ETHERNET_TXRATE,
        0,
        0,
        MIN_ETHERNET_FRAME,
        MIN_ETHERNET_FRAME
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
    curEtherDescr = nullEtherDescr;
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

    calculateParameters(true);

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
    numFramesFromHL = numDroppedIfaceDown = 0;
    numPauseFramesRcvd = numPauseFramesSent = 0;

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
            refreshConnection();
    }
    else if (dynamic_cast<cPostPathCutNotification *>(obj))
    {
        cPostPathCutNotification *gcobj = (cPostPathCutNotification *)obj;

        if ((physOutGate == gcobj->pathStartGate) || (physInGate == gcobj->pathEndGate))
            refreshConnection();
    }
    else if (transmissionChannel && dynamic_cast<cPostParameterChangeNotification *>(obj))
    {
        cPostParameterChangeNotification *gcobj = (cPostParameterChangeNotification *)obj;
        if (transmissionChannel == gcobj->par->getOwner() && !strcmp("datarate", gcobj->par->getName()))
            refreshConnection();
    }
}

void EtherMACBase::ifDown()
{
    // cMessage *endTxMsg, *endIFGMsg, *endPauseMsg;
    cancelEvent(endTxMsg);
    cancelEvent(endIFGMsg);
    cancelEvent(endPauseMsg);

    if (curTxFrame)
    {
        delete curTxFrame;
        curTxFrame = NULL;
        lastTxFinishTime = simTime() - 1.0;  // never equals with current simtime. for Burst mode.
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

void EtherMACBase::refreshConnection()
{
    Enter_Method_Silent();

    calculateParameters(false);

    if (!connected)
        ifDown();
}

bool EtherMACBase::checkDestinationAddress(EtherFrame *frame)
{
    // If not set to promiscuous = on, then checks if received frame contains destination MAC address
    // matching port's MAC address, also checks if broadcast bit is set

    if (promiscuous || frame->getDest().isBroadcast() || frame->getDest().isMulticast() || frame->getDest().equals(address))
        return true;

    EV << "Frame `" << frame->getName() <<"' not destined to us, discarding\n";
    numDroppedNotForUs++;
    emit(droppedPkBytesNotForUsSignal, (long)(frame->getByteLength()));
    delete frame;
    return false;
}

void EtherMACBase::calculateParameters(bool errorWhenAsymmetric)
{
    ASSERT(physInGate == gate("phys$i"));
    ASSERT(physOutGate == gate("phys$o"));

    cChannel *outTrChannel = physOutGate->findTransmissionChannel();
    cChannel *inTrChannel = physInGate->findIncomingTransmissionChannel();
    connected = (outTrChannel != NULL) && (inTrChannel != NULL);
    double txRate = outTrChannel ? outTrChannel->getNominalDatarate() : 0.0;
    double rxRate = inTrChannel ? inTrChannel->getNominalDatarate() : 0.0;

    if (!connected)
    {
        curEtherDescr = nullEtherDescr;
        carrierExtension = false;
        dataratesDiffer = (outTrChannel != NULL) || (inTrChannel != NULL);
        if (transmissionChannel)
            transmissionChannel->forceTransmissionFinishTime(SimTime());
        transmissionChannel = NULL;
        interfaceEntry->setDown(true);
        interfaceEntry->setDatarate(0);
    }
    else
    {
        if (outTrChannel && !transmissionChannel)
            outTrChannel->subscribe(POST_MODEL_CHANGE, this);
        transmissionChannel = outTrChannel;
        dataratesDiffer = (txRate != rxRate);
    }

    if (dataratesDiffer)
    {
        if (errorWhenAsymmetric)
            throw cRuntimeError(this, "The input/output datarates differ (%g / %g bps)", rxRate, txRate);

        // When the datarate of the connected channels change at runtime, we'll receive
        // two separate notifications (one for the rx channel and one for the tx one),
        // so we cannot immediately raise an error when they differ. Rather, we'll need
        // to verify at the next opportunity (event) that the two datarates have eventually
        // been set to the same value.
        //
        EV << "The input/output datarates differ (" << rxRate << " / " << txRate << "bps).\n";
    }

    if (connected)
    {
        // Check valid speeds
        for (int i = 0; i < NUM_OF_ETHERDESCRS; i++)
        {
            if (txRate == etherDescrs[i].txrate)
            {
                curEtherDescr = etherDescrs[i];
                interfaceEntry->setDown(false);
                interfaceEntry->setDatarate(txRate);
                return;
            }
        }
        throw cRuntimeError(this, "Invalid transmission rate %g bps on channel %s at module %s",
                    txRate, transmissionChannel->getFullPath().c_str(), getFullPath().c_str());
    }
}

void EtherMACBase::printParameters()
{
    // Dump parameters
    EV << "MAC address: " << address << (promiscuous ? ", promiscuous mode" : "") << endl
       << "txrate: " << curEtherDescr.txrate << ", "
       << (duplexMode ? "full-duplex" : "half-duplex") << endl;
#if 1
    EV << "bitTime: " << 1.0 / curEtherDescr.txrate << endl;
    EV << "carrierExtension: " << carrierExtension << endl;
    EV << "frameBursting: " << frameBursting << endl;
    EV << "slotTime: " << 512.0 / curEtherDescr.txrate << endl;
    EV << "interFrameGap: " << INTERFRAME_GAP_BITS / curEtherDescr.txrate << endl;
    EV << endl;
#endif
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

void EtherMACBase::receiveChangeNotification(int category, const cObject *)
{
    if (category == NF_SUBSCRIBERLIST_CHANGED)
        updateHasSubcribers();
}


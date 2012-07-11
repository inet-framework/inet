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
    0.0,
    0,
    0,
    0,
    0,
    0,
    0,
    0.0,
    0.0
};

const EtherMACBase::EtherDescr EtherMACBase::etherDescrs[NUM_OF_ETHERDESCRS] =
{
    {
        ETHERNET_TXRATE,
        0.5 / ETHERNET_TXRATE,
        MIN_ETHERNET_FRAME_BYTES,
        0,
        0,
        MIN_ETHERNET_FRAME_BYTES,
        MIN_ETHERNET_FRAME_BYTES,
        512 / ETHERNET_TXRATE,
        2500 /*m*/ / SPEED_OF_LIGHT_IN_CABLE
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
        250 /*m*/ / SPEED_OF_LIGHT_IN_CABLE
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
        250 /*m*/ / SPEED_OF_LIGHT_IN_CABLE
    },
    {
        FAST_GIGABIT_ETHERNET_TXRATE,
        0.5 / FAST_GIGABIT_ETHERNET_TXRATE,
        MIN_ETHERNET_FRAME_BYTES,
        0,
        0,
        -1,  // half-duplex is not supported
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
        -1,  // half-duplex is not supported
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
        -1,  // half-duplex is not supported
        0,
        0.0,
        0.0
    }
};

simsignal_t EtherMACBase::txPkSignal = SIMSIGNAL_NULL;
simsignal_t EtherMACBase::rxPkOkSignal = SIMSIGNAL_NULL;
simsignal_t EtherMACBase::txPausePkUnitsSignal = SIMSIGNAL_NULL;
simsignal_t EtherMACBase::rxPausePkUnitsSignal = SIMSIGNAL_NULL;
simsignal_t EtherMACBase::rxPkFromHLSignal = SIMSIGNAL_NULL;
simsignal_t EtherMACBase::dropPkNotForUsSignal = SIMSIGNAL_NULL;
simsignal_t EtherMACBase::dropPkBitErrorSignal = SIMSIGNAL_NULL;
simsignal_t EtherMACBase::dropPkIfaceDownSignal = SIMSIGNAL_NULL;
simsignal_t EtherMACBase::dropPkFromHLIfaceDownSignal = SIMSIGNAL_NULL;

simsignal_t EtherMACBase::packetSentToLowerSignal = SIMSIGNAL_NULL;
simsignal_t EtherMACBase::packetReceivedFromLowerSignal = SIMSIGNAL_NULL;
simsignal_t EtherMACBase::packetSentToUpperSignal = SIMSIGNAL_NULL;
simsignal_t EtherMACBase::packetReceivedFromUpperSignal = SIMSIGNAL_NULL;


EtherMACBase::EtherMACBase()
{
    lastTxFinishTime = -1.0; // never equals to current simtime
    curEtherDescr = &nullEtherDescr;
    transmissionChannel = NULL;
    physInGate = NULL;
    physOutGate = NULL;
    upperLayerInGate = NULL;
    interfaceEntry = NULL;
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
    upperLayerInGate = gate("upperLayerIn");
    transmissionChannel = NULL;
    interfaceEntry = NULL;
    curTxFrame = NULL;

    initializeFlags();

    initializeMACAddress();
    initializeQueueModule();
    initializeStatistics();

    registerInterface(); // needs MAC address

    readChannelParameters(true);

    lastTxFinishTime = -1.0; // not equals with current simtime.

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

void EtherMACBase::initializeQueueModule()
{
    if (par("queueModule").stringValue()[0])
    {
        cModule *module = getParentModule()->getSubmodule(par("queueModule").stringValue());
        IPassiveQueue *queueModule;
        if (module->isSimple())
            queueModule = check_and_cast<IPassiveQueue *>(module);
        else
        {
            cGate *queueOut = module->gate("out")->getPathStartGate();
            queueModule = check_and_cast<IPassiveQueue *>(queueOut->getOwnerModule());
        }

        EV << "Requesting first frame from queue module\n";
        txQueue.setExternalQueue(queueModule);

        if (txQueue.extQueue->getNumPendingRequests() == 0)
            txQueue.extQueue->requestPacket();
    }
    else
    {
        txQueue.setInternalQueue("txQueue", par("txQueueLimit").longValue());
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

void EtherMACBase::initializeFlags()
{
    duplexMode = true;

    // initialize connected flag
    connected = physOutGate->getPathEndGate()->isConnected() && physInGate->getPathStartGate()->isConnected();

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

    txPkSignal = registerSignal("txPk");
    rxPkOkSignal = registerSignal("rxPkOk");
    txPausePkUnitsSignal = registerSignal("txPausePkUnits");
    rxPausePkUnitsSignal = registerSignal("rxPausePkUnits");
    rxPkFromHLSignal = registerSignal("rxPkFromHL");
    dropPkBitErrorSignal = registerSignal("dropPkBitError");
    dropPkIfaceDownSignal = registerSignal("dropPkIfaceDown");
    dropPkFromHLIfaceDownSignal = registerSignal("dropPkFromHLIfaceDown");
    dropPkNotForUsSignal = registerSignal("dropPkNotForUs");

    packetSentToLowerSignal = registerSignal("packetSentToLower");
    packetReceivedFromLowerSignal = registerSignal("packetReceivedFromLower");
    packetSentToUpperSignal = registerSignal("packetSentToUpper");
    packetReceivedFromUpperSignal = registerSignal("packetReceivedFromUpper");
}

void EtherMACBase::registerInterface()
{
    interfaceEntry = new InterfaceEntry(this);

    // interface name: NIC module's name without special characters ([])
    interfaceEntry->setName(OPP_Global::stripnonalnum(getParentModule()->getFullName()).c_str());

    // generate a link-layer address to be used as interface token for IPv6
    interfaceEntry->setMACAddress(address);
    interfaceEntry->setInterfaceToken(address.formInterfaceIdentifier());
    //InterfaceToken token(0, simulation.getUniqueNumber(), 64);
    //interfaceEntry->setInterfaceToken(token);

    // MTU: typical values are 576 (Internet de facto), 1500 (Ethernet-friendly),
    // 4000 (on some point-to-point links), 4470 (Cisco routers default, FDDI compatible)
    interfaceEntry->setMtu(par("mtu").longValue());

    // capabilities
    interfaceEntry->setMulticast(true);
    interfaceEntry->setBroadcast(true);

    // add
    IInterfaceTable *ift = InterfaceTableAccess().getIfExists();

    if (ift)
        ift->addInterface(interfaceEntry);
}

void EtherMACBase::receiveSignal(cComponent *src, simsignal_t signalId, cObject *obj)
{
    Enter_Method_Silent();

    ASSERT(signalId == POST_MODEL_CHANGE);

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
    else if (transmissionChannel && dynamic_cast<cPostParameterChangeNotification *>(obj)) // note: we are subscribed to the channel object too
    {
        cPostParameterChangeNotification *gcobj = (cPostParameterChangeNotification *)obj;
        if (transmissionChannel == gcobj->par->getOwner() && !strcmp("datarate", gcobj->par->getName()))
            refreshConnection();
    }
}

void EtherMACBase::processConnectDisconnect()
{
    if (!connected)
    {
        cancelEvent(endTxMsg);
        cancelEvent(endIFGMsg);
        cancelEvent(endPauseMsg);

        if (curTxFrame)
        {
            delete curTxFrame;
            curTxFrame = NULL;
            lastTxFinishTime = -1.0;  // so that it never equals to the current simtime, used for Burst mode detection.
        }

        if (txQueue.extQueue)
        {
            // Clear external queue: send a request, and received packet will be deleted in handleMessage()
            if (txQueue.extQueue->getNumPendingRequests() == 0)
                txQueue.extQueue->requestPacket();
        }
        else
        {
            // Clear inner queue
            while (!txQueue.innerQueue->empty())
            {
                cMessage *msg = check_and_cast<cMessage *>(txQueue.innerQueue->pop());
                EV << "Interface is not connected, dropping packet " << msg << endl;
                numDroppedPkFromHLIfaceDown++;
                emit(dropPkIfaceDownSignal, msg);
                delete msg;
            }
        }

        transmitState = TX_IDLE_STATE;
        receiveState = RX_IDLE_STATE;
    }
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

    bool isPause = (dynamic_cast<EtherPauseFrame *>(frame) != NULL);

    if (!isPause && (promiscuous || frame->getDest().isMulticast()))
        return false;

    if (isPause && frame->getDest().equals(MACAddress::MULTICAST_PAUSE_ADDRESS))
        return false;

    EV << "Frame `" << frame->getName() <<"' not destined to us, discarding\n";
    numDroppedNotForUs++;
    emit(dropPkNotForUsSignal, frame);
    delete frame;
    return true;
}

void EtherMACBase::readChannelParameters(bool errorWhenAsymmetric)
{
    cChannel *outTrChannel = physOutGate->findTransmissionChannel();
    cChannel *inTrChannel = physInGate->findIncomingTransmissionChannel();

    connected = physOutGate->getPathEndGate()->isConnected() && physInGate->getPathStartGate()->isConnected();

    if (connected && ((!outTrChannel) || (!inTrChannel)))
        throw cRuntimeError("Ethernet phys gate must be connected using a transmission channel");

    double txRate = outTrChannel ? outTrChannel->getNominalDatarate() : 0.0;
    double rxRate = inTrChannel ? inTrChannel->getNominalDatarate() : 0.0;

    if (!connected)
    {
        curEtherDescr = &nullEtherDescr;
        dataratesDiffer = (outTrChannel != NULL) || (inTrChannel != NULL);
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
            throw cRuntimeError("The input/output datarates differ (%g / %g bps)", rxRate, txRate);

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
                curEtherDescr = &(etherDescrs[i]);
                interfaceEntry->setDown(false);
                interfaceEntry->setDatarate(txRate);
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
    EV << "MAC address: " << address << (promiscuous ? ", promiscuous mode" : "") << endl
       << "txrate: " << curEtherDescr->txrate << ", "
       << (duplexMode ? "full-duplex" : "half-duplex") << endl;
#if 1
    EV << "bitTime: " << 1.0 / curEtherDescr->txrate << endl;
    EV << "frameBursting: " << frameBursting << endl;
    EV << "slotTime: " << curEtherDescr->slotTime << endl;
    EV << "interFrameGap: " << INTERFRAME_GAP_BITS / curEtherDescr->txrate << endl;
    EV << endl;
#endif
}

void EtherMACBase::getNextFrameFromQueue()
{
    if (txQueue.extQueue)
    {
        if (txQueue.extQueue->getNumPendingRequests() == 0)
            txQueue.extQueue->requestPacket();
    }
    else
    {
        if (!txQueue.innerQueue->empty())
            curTxFrame = (EtherFrame*)txQueue.innerQueue->pop();
    }
}

void EtherMACBase::requestNextFrameFromExtQueue()
{
    if (txQueue.extQueue)
    {
        if (txQueue.extQueue->getNumPendingRequests() == 0)
            txQueue.extQueue->requestPacket();
    }
}

void EtherMACBase::finish()
{
    if (!disabled)
    {
        simtime_t t = simTime();
        recordScalar("simulated time", t);
        recordScalar("full-duplex", duplexMode);

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


int EtherMACBase::InnerQueue::packetCompare(cObject *a, cObject *b)
{
    int ap = (dynamic_cast<EtherPauseFrame*>(a) == NULL) ? 1 : 0;
    int bp = (dynamic_cast<EtherPauseFrame*>(b) == NULL) ? 1 : 0;
    return ap - bp;
}


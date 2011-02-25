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

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "EtherMACBase.h"
#include "IPassiveQueue.h"
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"

static const double SPEED_OF_LIGHT = 200000000.0;

EtherMACBase::EtherMACBase()
{
    nb = NULL;
    queueModule = NULL;
    interfaceEntry = NULL;
    endTxMsg = endIFGMsg = endPauseMsg = NULL;
}

EtherMACBase::~EtherMACBase()
{
    cancelAndDelete(endTxMsg);
    cancelAndDelete(endIFGMsg);
    cancelAndDelete(endPauseMsg);
}

void EtherMACBase::initialize()
{
    physOutGate = gate("phys$o");

    initializeFlags();

    initializeTxrate();
    WATCH(txrate);

    initializeMACAddress();
    initializeQueueModule();
    initializeNotificationBoard();
    initializeStatistics();

    registerInterface(txrate); // needs MAC address

    // initialize queue
    txQueue.setName("txQueue");

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

    // initialize queue limit
    txQueueLimit = par("txQueueLimit");
    WATCH(txQueueLimit);
}

void EtherMACBase::initializeQueueModule()
{
    if (par("queueModule").stringValue()[0])
    {
        cModule *module = getParentModule()->getSubmodule(par("queueModule").stringValue());
        queueModule = check_and_cast<IPassiveQueue *>(module);
        EV << "Requesting first frame from queue module\n";
        queueModule->requestPacket();
    }
}

void EtherMACBase::initializeMACAddress()
{
    const char *addrstr = par("address");

    if (!strcmp(addrstr,"auto"))
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
    if (interfaceEntry) {
        nb = NotificationBoardAccess().getIfExists();
        notifDetails.setInterfaceEntry(interfaceEntry);
        nb->subscribe(this, NF_SUBSCRIBERLIST_CHANGED);
        updateHasSubcribers();
    }
}

void EtherMACBase::initializeFlags()
{
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

    numFramesSentVector.setName("framesSent");
    numFramesReceivedOKVector.setName("framesReceivedOK");
    numBytesSentVector.setName("bytesSent");
    numBytesReceivedOKVector.setName("bytesReceivedOK");
    numDroppedIfaceDownVector.setName("framesDroppedIfaceDown");
    numDroppedBitErrorVector.setName("framesDroppedBitError");
    numDroppedNotForUsVector.setName("framesDroppedNotForUs");
    numFramesPassedToHLVector.setName("framesPassedToHL");
    numPauseFramesRcvdVector.setName("pauseFramesRcvd");
    numPauseFramesSentVector.setName("pauseFramesSent");
}

void EtherMACBase::registerInterface(double txrate)
{
    IInterfaceTable *ift = InterfaceTableAccess().getIfExists();
    if (!ift)
        return;

    interfaceEntry = new InterfaceEntry();

    // interface name: our module name without special characters ([])
    char *interfaceName = new char[strlen(getParentModule()->getFullName())+1];
    char *d=interfaceName;
    for (const char *s=getParentModule()->getFullName(); *s; s++)
        if (isalnum(*s))
            *d++ = *s;
    *d = '\0';

    interfaceEntry->setName(interfaceName);
    delete [] interfaceName;

    // data rate
    interfaceEntry->setDatarate(txrate);

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
    ift->addInterface(interfaceEntry, this);
}


bool EtherMACBase::checkDestinationAddress(EtherFrame *frame)
{
    // If not set to promiscuous = on, then checks if received frame contains destination MAC address
    // matching port's MAC address, also checks if broadcast bit is set
    if (!promiscuous && !frame->getDest().isBroadcast() && !frame->getDest().equals(address))
    {
        EV << "Frame `" << frame->getName() <<"' not destined to us, discarding\n";
        numDroppedNotForUs++;
        numDroppedNotForUsVector.record(numDroppedNotForUs);
        delete frame;

        return false;
    }

    return true;
}

void EtherMACBase::calculateParameters()
{
    if (disabled || !connected)
    {
        bitTime = slotTime = interFrameGap = jamDuration = shortestFrameDuration = 0;
        carrierExtension = frameBursting = false;
        return;
    }

    if (txrate != ETHERNET_TXRATE && txrate != FAST_ETHERNET_TXRATE &&
        txrate != GIGABIT_ETHERNET_TXRATE && txrate != FAST_GIGABIT_ETHERNET_TXRATE)
    {
        error("nonstandard transmission rate %g, must be %g, %g, %g or %g bit/sec",
            txrate, ETHERNET_TXRATE, FAST_ETHERNET_TXRATE, GIGABIT_ETHERNET_TXRATE, FAST_GIGABIT_ETHERNET_TXRATE);
    }

    bitTime = 1/(double)txrate;

    // set slot time
    if (txrate==ETHERNET_TXRATE || txrate==FAST_ETHERNET_TXRATE)
        slotTime = SLOT_TIME;
    else
        slotTime = GIGABIT_SLOT_TIME;

    // only if Gigabit Ethernet
    frameBursting = (txrate==GIGABIT_ETHERNET_TXRATE || txrate==FAST_GIGABIT_ETHERNET_TXRATE);
    carrierExtension = (slotTime == GIGABIT_SLOT_TIME && !duplexMode);

    interFrameGap = INTERFRAME_GAP_BITS/(double)txrate;
    jamDuration = 8*JAM_SIGNAL_BYTES*bitTime;
    shortestFrameDuration = carrierExtension ? GIGABIT_MIN_FRAME_WITH_EXT : MIN_ETHERNET_FRAME;
}

void EtherMACBase::printParameters()
{
    // Dump parameters
    EV << "MAC address: " << address << (promiscuous ? ", promiscuous mode" : "") << endl;
    EV << "txrate: " << txrate << ", " << (duplexMode ? "duplex" : "half-duplex") << endl;
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

    if (frame->getDest().equals(address))
    {
        error("logic error: frame %s from higher layer has local MAC address as dest (%s)",
              frame->getFullName(), frame->getDest().str().c_str());
    }

    if (frame->getByteLength() > MAX_ETHERNET_FRAME)
        error("packet from higher layer (%d bytes) exceeds maximum Ethernet frame size (%d)", (int)(frame->getByteLength()), MAX_ETHERNET_FRAME);

    // must be EtherFrame (or EtherPauseFrame) from upper layer
    bool isPauseFrame = (dynamic_cast<EtherPauseFrame*>(frame)!=NULL);
    if (!isPauseFrame)
    {
        numFramesFromHL++;

        if (txQueueLimit && txQueue.length()>txQueueLimit)
            error("txQueue length exceeds %d -- this is probably due to "
                  "a bogus app model generating excessive traffic "
                  "(or if this is normal, increase txQueueLimit!)",
                  txQueueLimit);

        // fill in src address if not set
        if (frame->getSrc().isUnspecified())
            frame->setSrc(address);

        // store frame and possibly begin transmitting
        EV << "Packet " << frame << " arrived from higher layers, enqueueing\n";
        txQueue.insert(frame);
    }
    else
    {
        EV << "PAUSE received from higher layer\n";

        // PAUSE frames enjoy priority -- they're transmitted before all other frames queued up
        if (!txQueue.empty())
            txQueue.insertBefore(txQueue.front(), frame);  // front() frame is probably being transmitted
        else
            txQueue.insert(frame);
    }

}

void EtherMACBase::processMsgFromNetwork(cPacket *frame)
{
    EV << "Received frame from network: " << frame << endl;

    // frame must be EtherFrame or EtherJam
    if (dynamic_cast<EtherFrame*>(frame)==NULL && dynamic_cast<EtherJam*>(frame)==NULL)
        error("message with unexpected message class '%s' arrived from network (name='%s')",
                frame->getClassName(), frame->getFullName());

    // detect cable length violation in half-duplex mode
    if (!duplexMode && simTime()-frame->getSendingTime()>=shortestFrameDuration)
        error("very long frame propagation time detected, maybe cable exceeds maximum allowed length? "
              "(%lgs corresponds to an approx. %lgm cable)",
              SIMTIME_STR(simTime() - frame->getSendingTime()),
              SIMTIME_STR((simTime() - frame->getSendingTime())*SPEED_OF_LIGHT));
}

void EtherMACBase::frameReceptionComplete(EtherFrame *frame)
{
    int pauseUnits;
    EtherPauseFrame *pauseFrame;

    if ((pauseFrame=dynamic_cast<EtherPauseFrame*>(frame))!=NULL)
    {
        pauseUnits = pauseFrame->getPauseTime();
        delete frame;
        numPauseFramesRcvd++;
        numPauseFramesRcvdVector.record(numPauseFramesRcvd);
        processPauseCommand(pauseUnits);
    }
    else
    {
        processReceivedDataFrame((EtherFrame *)frame);
    }
}

void EtherMACBase::processReceivedDataFrame(EtherFrame *frame)
{
    // bit errors
    if (frame->hasBitError())
    {
        numDroppedBitError++;
        numDroppedBitErrorVector.record(numDroppedBitError);
        delete frame;
        return;
    }

    // strip preamble and SFD
    frame->addByteLength(-PREAMBLE_BYTES-SFD_BYTES);

    // statistics
    numFramesReceivedOK++;
    numBytesReceivedOK += frame->getByteLength();
    numFramesReceivedOKVector.record(numFramesReceivedOK);
    numBytesReceivedOKVector.record(numBytesReceivedOK);

    if (!checkDestinationAddress(frame))
        return;

    numFramesPassedToHL++;
    numFramesPassedToHLVector.record(numFramesPassedToHL);

    // pass up to upper layer
    send(frame, "upperLayerOut");
}

void EtherMACBase::processPauseCommand(int pauseUnits)
{
    if (transmitState==TX_IDLE_STATE)
    {
        EV << "PAUSE frame received, pausing for " << pauseUnitsRequested << " time units\n";
        if (pauseUnits>0)
            scheduleEndPausePeriod(pauseUnits);
    }
    else if (transmitState==PAUSE_STATE)
    {
        EV << "PAUSE frame received, pausing for " << pauseUnitsRequested << " more time units from now\n";
        cancelEvent(endPauseMsg);
        if (pauseUnits>0)
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
    if (transmitState!=WAIT_IFG_STATE)
        error("Not in WAIT_IFG_STATE at the end of IFG period");

    if (txQueue.empty())
        error("End of IFG and no frame to transmit");

    // End of IFG period, okay to transmit, if Rx idle OR duplexMode
    cPacket *frame = (cPacket *)txQueue.front();
    EV << "IFG elapsed, now begin transmission of frame " << frame << endl;

    // Perform carrier extension if in Gigabit Ethernet
    if (carrierExtension && frame->getByteLength() < GIGABIT_MIN_FRAME_WITH_EXT)
    {
        EV << "Performing carrier extension of small frame\n";
        frame->setByteLength(GIGABIT_MIN_FRAME_WITH_EXT);
    }

    // start frame burst, if enabled
    if (frameBursting)
    {
        EV << "Starting frame burst\n";
        framesSentInBurst = 0;
        bytesSentInBurst = 0;
    }
}

void EtherMACBase::handleEndTxPeriod()
{
    // we only get here if transmission has finished successfully, without collision
    if (transmitState!=TRANSMITTING_STATE || (!duplexMode && receiveState!=RX_IDLE_STATE))
        error("End of transmission, and incorrect state detected");

    if (txQueue.empty())
        error("Frame under transmission cannot be found");

    // get frame from buffer
    cPacket *frame = (cPacket *)txQueue.pop();

    numFramesSent++;
    numBytesSent += frame->getByteLength();
    numFramesSentVector.record(numFramesSent);
    numBytesSentVector.record(numBytesSent);

    if (dynamic_cast<EtherPauseFrame*>(frame)!=NULL)
    {
        numPauseFramesSent++;
        numPauseFramesSentVector.record(numPauseFramesSent);
    }

    EV << "Transmission of " << frame << " successfully completed\n";
    delete frame;
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
    delete msg;
    numDroppedIfaceDown++;
}

void EtherMACBase::processMessageWhenDisabled(cMessage *msg)
{
    EV << "MAC is disabled -- dropping message " << msg << endl;
    delete msg;
}

void EtherMACBase::scheduleEndIFGPeriod()
{
    scheduleAt(simTime()+interFrameGap, endIFGMsg);
    transmitState = WAIT_IFG_STATE;
}

void EtherMACBase::scheduleEndTxPeriod(cPacket *frame)
{
    scheduleAt(simTime()+frame->getBitLength()*bitTime, endTxMsg);
    transmitState = TRANSMITTING_STATE;
}

void EtherMACBase::scheduleEndPausePeriod(int pauseUnits)
{
    // length is interpreted as 512-bit-time units
    simtime_t pausePeriod = pauseUnits*PAUSE_BITTIME*bitTime;
    scheduleAt(simTime()+pausePeriod, endPauseMsg);
    transmitState = PAUSE_STATE;
}

bool EtherMACBase::checkAndScheduleEndPausePeriod()
{
    if (pauseUnitsRequested>0)
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
    if (!txQueue.empty())
    {
        // Other frames are queued, therefore wait IFG period and transmit next frame
        EV << "Transmit next frame in output queue, after IFG period\n";
        scheduleEndIFGPeriod();
    }
    else
    {
        transmitState = TX_IDLE_STATE;
        if (queueModule)
        {
            // tell queue module that we've become idle
            EV << "Requesting another frame from queue module\n";
            queueModule->requestPacket();
        }
        else
        {
            // No more frames set transmitter to idle
            EV << "No more frames to send, transmitter set to idle\n";
        }
    }
}

void EtherMACBase::fireChangeNotification(int type, cPacket *msg)
{
    if (nb) {
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
        recordScalar("txrate (Mb)", txrate/1000000);
        recordScalar("full duplex", duplexMode);
        recordScalar("frames sent",    numFramesSent);
        recordScalar("frames rcvd",    numFramesReceivedOK);
        recordScalar("bytes sent",     numBytesSent);
        recordScalar("bytes rcvd",     numBytesReceivedOK);
        recordScalar("frames from higher layer", numFramesFromHL);
        recordScalar("frames from higher layer dropped (iface down)", numDroppedIfaceDown);
        recordScalar("frames dropped (bit error)",  numDroppedBitError);
        recordScalar("frames dropped (not for us)", numDroppedNotForUs);
        recordScalar("frames passed up to HL", numFramesPassedToHL);
        recordScalar("PAUSE frames sent",  numPauseFramesSent);
        recordScalar("PAUSE frames rcvd",  numPauseFramesRcvd);

        if (t>0)
        {
            recordScalar("frames/sec sent", numFramesSent/t);
            recordScalar("frames/sec rcvd", numFramesReceivedOK/t);
            recordScalar("bits/sec sent",   8*numBytesSent/t);
            recordScalar("bits/sec rcvd",   8*numBytesReceivedOK/t);
        }
    }
}

void EtherMACBase::updateDisplayString()
{
    // icon coloring
    const char *color;
    if (receiveState==RX_COLLISION_STATE)
        color = "red";
    else if (transmitState==TRANSMITTING_STATE)
        color = "yellow";
    else if (transmitState==JAMMING_STATE)
        color = "red";
    else if (receiveState==RECEIVING_STATE)
        color = "#4040ff";
    else if (transmitState==BACKOFF_STATE)
        color = "white";
    else if (transmitState==PAUSE_STATE)
        color = "gray";
    else
        color = "";
    getDisplayString().setTagArg("i",1,color);
    if (!strcmp(getParentModule()->getClassName(),"EthernetInterface"))
        getParentModule()->getDisplayString().setTagArg("i",1,color);

    // connection coloring
    updateConnectionColor(transmitState);

#if 0
    // this code works but didn't turn out to be very useful
    const char *txStateName;
    switch (transmitState) {
        case TX_IDLE_STATE:      txStateName="IDLE"; break;
        case WAIT_IFG_STATE:     txStateName="WAIT_IFG"; break;
        case TRANSMITTING_STATE: txStateName="TX"; break;
        case JAMMING_STATE:      txStateName="JAM"; break;
        case BACKOFF_STATE:      txStateName="BACKOFF"; break;
        case PAUSE_STATE:        txStateName="PAUSE"; break;
        default: error("wrong tx state");
    }
    const char *rxStateName;
    switch (receiveState) {
        case RX_IDLE_STATE:      rxStateName="IDLE"; break;
        case RECEIVING_STATE:    rxStateName="RX"; break;
        case RX_COLLISION_STATE: rxStateName="COLL"; break;
        default: error("wrong rx state");
    }

    char buf[80];
    sprintf(buf, "tx:%s rx: %s\n#boff:%d #cTx:%d",
                 txStateName, rxStateName, backoffs, numConcurrentTransmissions);
    getDisplayString().setTagArg("t",0,buf);
#endif
}

void EtherMACBase::updateConnectionColor(int txState)
{
    const char *color;
    if (txState==TRANSMITTING_STATE)
        color = "yellow";
    else if (txState==JAMMING_STATE || txState==BACKOFF_STATE)
        color = "red";
    else
        color = "";

    cGate *g = physOutGate;
    while (g && g->getType()==cGate::OUTPUT)
    {
        g->getDisplayString().setTagArg("ls",0,color);
        g->getDisplayString().setTagArg("ls",1, color[0] ? "3" : "1");
        g = g->getNextGate();
    }
}

void EtherMACBase::receiveChangeNotification(int category, const cPolymorphic *)
{
    if (category==NF_SUBSCRIBERLIST_CHANGED)
        updateHasSubcribers();
}



/*
 * Copyright (C) 2003 CTIE, Monash University
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

#include "EtherMAC.h"
#include "utils.h"



static cEnvir& operator<< (cEnvir& ev, cMessage *msg)
{
    ev.printf("(%s)%s",msg->className(),msg->fullName());
    return ev;
}


static cEnvir& operator<< (cEnvir& ev, const MACAddress& addr)
{
    char buf[20];
    ev << addr.toHexString(buf);
    return ev;
}

static std::ostream& operator<< (std::ostream& ev, const MACAddress& addr)
{
    char buf[20];
    ev << addr.toHexString(buf);
    return ev;
}


Define_Module( EtherMAC );

unsigned int EtherMAC::autoAddressCtr = 0;

void EtherMAC::initialize()
{
    outputbuffer.setName("outputBuffer");

    frameBeingReceived = NULL;
    endTxMsg = new cMessage("EndTransmission", ENDTRANSMISSION);
    endJammingMsg = new cMessage("EndJamming", ENDJAMMING);
    endRxMsg = new cMessage("EndReception", ENDRECEPTION);
    endIFGMsg = new cMessage("EndIFG", ENDIFG);
    endBackoffMsg = new cMessage("EndBackoff", ENDBACKOFF);
    endPauseMsg = new cMessage("EndPause", ENDPAUSE);

    const char *addrstr = par("address");
    if (!strcmp(addrstr,"auto"))
    {
        // assign automatic address
        ++autoAddressCtr;
        unsigned char addrbytes[6];
        addrbytes[0] = 0x0A;
        addrbytes[1] = 0xAA;
        addrbytes[2] = (autoAddressCtr>>24)&0xff;
        addrbytes[3] = (autoAddressCtr>>16)&0xff;
        addrbytes[4] = (autoAddressCtr>>8)&0xff;
        addrbytes[5] = (autoAddressCtr)&0xff;
        myaddress.setAddressBytes(addrbytes);

        // change module parameter from "auto" to concrete address
        char buf[20];
        par("address").setStringValue(myaddress.toHexString(buf));
    }
    else
    {
        myaddress.setAddress(addrstr);
    }
    promiscuous = par("promiscuous");
    maxQueueSize = par("maxQueueSize");

    txrate = par("txrate");
    if (txrate!=ETHERNET_TXRATE && txrate!=FAST_ETHERNET_TXRATE && txrate!=GIGABIT_ETHERNET_TXRATE)
    {
        error("nonstandard txrate, must be %ld, %ld or %ld bit/sec", ETHERNET_TXRATE,
              FAST_ETHERNET_TXRATE, GIGABIT_ETHERNET_TXRATE);
    }
    bitTime = 1/(double)txrate;

    duplexChannel = par("duplexChannel");

    // Only if Gigabit Ethernet
    carrierExtension = (txrate==GIGABIT_ETHERNET_TXRATE && !duplexChannel);
    frameBursting = par("frameBursting");
    if (frameBursting && txrate<GIGABIT_ETHERNET_TXRATE)
        error("frameBursting can only be enabled with Gigabit Ethernet");

    // set slot time
    if (txrate==ETHERNET_TXRATE || txrate==FAST_ETHERNET_TXRATE)
        slotTime = SLOT_TIME;
    else
        slotTime = GIGABIT_SLOT_TIME;

    interFrameGap = INTERFRAME_GAP_BITS/(double)txrate;
    jamDuration = 8*JAM_SIGNAL_BYTES*bitTime;
    shortestFrameDuration = carrierExtension ? GIGABIT_MIN_FRAME_WITH_EXT : MIN_ETHERNET_FRAME;

    transmitState = TX_IDLE_STATE;
    receiveState = RX_IDLE_STATE;
    backoffs = 0;
    numConcurrentTransmissions = 0;
    framesSentInBurst = 0;
    bytesSentInBurst = 0;
    pauseUnitsRequested = 0;

    WATCH(transmitState);
    WATCH(receiveState);
    WATCH(backoffs);
    WATCH(numConcurrentTransmissions);
    WATCH(framesSentInBurst);
    WATCH(bytesSentInBurst);
    WATCH(pauseUnitsRequested);

    // init statistics
    totalCollisionTime = 0.0;
    totalSuccessfulRxTxTime = 0.0;
    numFramesSent = numFramesReceivedOK = numBytesSent = numBytesReceivedOK = 0;
    numFramesPassedToHL = numDroppedBitError = numDroppedNotForUs = numDroppedQueueFull = 0;
    numPauseFramesRcvd = numPauseFramesSent = numCollisions = numBackoffs = 0;

    WATCH(numFramesSent);
    WATCH(numFramesReceivedOK);
    WATCH(numBytesSent);
    WATCH(numBytesReceivedOK);
    WATCH(numDroppedQueueFull);
    WATCH(numDroppedBitError);
    WATCH(numDroppedNotForUs);
    WATCH(numFramesPassedToHL);
    WATCH(numPauseFramesRcvd);
    WATCH(numPauseFramesSent);
    WATCH(numCollisions);
    WATCH(numBackoffs);

    numFramesSentVector.setName("framesSent");
    numFramesReceivedOKVector.setName("framesReceivedOK");
    numBytesSentVector.setName("bytesSent");
    numBytesReceivedOKVector.setName("bytesReceivedOK");
    numDroppedQueueFullVector.setName("framesDroppedQueueFull");
    numDroppedBitErrorVector.setName("framesDroppedBitError");
    numDroppedNotForUsVector.setName("framesDroppedNotForUs");
    numFramesPassedToHLVector.setName("framesPassedToHL");
    numPauseFramesRcvdVector.setName("pauseFramesRcvd");
    numPauseFramesSentVector.setName("pauseFramesSent");
    numCollisionsVector.setName("collisions");
    numBackoffsVector.setName("backoffs");

    // Dump parameters
    EV << "Parameters of (" << className() << ") " << fullPath() << "\n";
    EV << "myaddress: " << myaddress << endl;
    EV << "promiscuous: " << promiscuous << endl;
    EV << "txrate: " << txrate << endl;
    EV << "bitTime: " << bitTime << endl;
    EV << "carrierExtension: " << carrierExtension << endl;
    EV << "frameBursting: " << frameBursting << endl;
    EV << "slotTime: " << slotTime << endl;
    EV << "interFrameGap: " << interFrameGap << endl;
    EV << "\n";
}


void EtherMAC::handleMessage (cMessage *msg)
{
    printState();
    // some consistency check
    if (!duplexChannel && transmitState==TRANSMITTING_STATE && receiveState!=RX_IDLE_STATE)
        error("Inconsistent state -- transmitting and receiving at the same time");

    if (!msg->isSelfMessage())
    {
        // either frame from upper layer, or frame/jam signal from the network
        if (msg->arrivalGate() == gate("UpperLayer_in"))
        {
            EV << "Received frame from upper layer: " << msg << endl;
            processFrameFromUpperLayer((EtherFrame *)msg);
        }
        else
        {
            EV << "Received frame from network: " << msg << endl;
            processMsgFromNetwork(msg);
        }
    }
    else
    {
        // Process different self-messages (timer signals)
        EV << "Self-message " << msg << " received\n";
        switch (msg->kind())
        {
            case ENDIFG:
                handleEndIFGPeriod();
                break;

            case ENDTRANSMISSION:
                handleEndTxPeriod();
                break;

            case ENDRECEPTION:
                handleEndRxPeriod();
                break;

            case ENDBACKOFF:
                handleEndBackoffPeriod();
                break;

            case ENDJAMMING:
                handleEndJammingPeriod();
                break;

            case ENDPAUSE:
                handleEndPausePeriod();
                break;

            default:
                error("self-message with unexpected message kind %d", msg->kind());
        }
    }
    printState();
}


void EtherMAC::processFrameFromUpperLayer(EtherFrame *frame)
{
    // check message kind
    if (frame->kind()!=ETH_FRAME && frame->kind()!=ETH_PAUSE)
        error("message with unexpected message kind %d arrived from higher layer", frame->kind());

    if (frame->length()>=8*MAX_ETHERNET_FRAME)
        error("packet from higher layer (%d bytes) exceeds maximum Ethernet frame size (%d)", frame->length()/8, MAX_ETHERNET_FRAME);

    // must be ETH_FRAME (or ETH_PAUSE) from upper layer
    if (outputbuffer.length() >= maxQueueSize)
    {
        EV << "Packet " << frame << " arrived from higher layers but queue full, dropping\n";
        numDroppedQueueFull++;
        numDroppedQueueFullVector.record(numDroppedQueueFull);
        delete frame;
        return;
    }

    // store frame and possibly begin transmitting
    EV << "Packet " << frame << " arrived from higher layers, enqueueing\n";
    if (frame->getSrc().isEmpty())
    {
        EV << "Filling in source address\n";
        frame->setSrc(myaddress);
    }

    outputbuffer.insert(frame);

    if ((duplexChannel || receiveState==RX_IDLE_STATE) && transmitState==TX_IDLE_STATE)
    {
        EV << "No incoming carrier signals detected, frame clear to send, wait IFG first\n";
        scheduleEndIFGPeriod();
    }
}


void EtherMAC::processMsgFromNetwork(cMessage *msg)
{
    // msg must be ETH_FRAME, ETH_PAUSE or JAM_SIGNAL
    if (msg->kind()!=ETH_FRAME && msg->kind()!=ETH_PAUSE && msg->kind()!=JAM_SIGNAL)
        error("message with unexpected message kind %d arrived from network", msg->kind());

    // detect cable length violation in half-duplex mode
    if (!duplexChannel && simTime()-msg->sendingTime()>=shortestFrameDuration)
        error("very long frame propagation time detected, maybe cable exceeds maximum allowed length? "
              "(%lgs corresponds to an approx. %lgm cable)",
              simTime()-msg->sendingTime(),
              (simTime()-msg->sendingTime())*200000000.0);

    simtime_t endRxTime = simTime() + msg->length()*bitTime;

    if (!duplexChannel && transmitState==TRANSMITTING_STATE)
    {
        // since we're halfduplex, receiveState must be RX_IDLE_STATE (asserted at top of handleMessage)
        if (msg->kind()==JAM_SIGNAL)
            error("Stray jam signal arrived while transmitting (usual cause is cable length exceeding allowed maximum)");

        EV << "Transmission interrupted by incoming frame, handling collision\n";
        cancelEvent(endTxMsg);

        EV << "Transmitting jam signal\n";
        sendJamSignal(); // backoff will be executed when jamming finished

        // set receive state and schedule end of reception
        receiveState = RX_COLLISION_STATE;
        numConcurrentTransmissions++;
        simtime_t endJamTime = simTime()+jamDuration;
        scheduleAt(endRxTime<endJamTime ? endJamTime : endRxTime, endRxMsg);
        delete msg;

        numCollisions++;
        numCollisionsVector.record(numCollisions);
    }
    else if (receiveState==RX_IDLE_STATE)
    {
        if (msg->kind()==JAM_SIGNAL)
            error("Stray jam signal arrived (usual cause is cable length exceeding allowed maximum)");

        EV << "Start reception of frame\n";
        numConcurrentTransmissions++;
        if (frameBeingReceived)
            error("frameBeingReceived!=0 in RX_IDLE_STATE");
        frameBeingReceived = (EtherFrame *)msg;
        scheduleEndRxPeriod(msg);
        channelBusySince = simTime();
    }
    else if (receiveState==RECEIVING_STATE && msg->kind()!=JAM_SIGNAL && endRxMsg->arrivalTime()-simTime()<bitTime)
    {
        // With the above condition we filter out "false" collisions that may occur with
        // back-to-back frames. That is: when "beginning of frame" message (this one) occurs
        // BEFORE "end of previous frame" event (endRxMsg) -- same simulation time,
        // only wrong order.

        EV << "Back-to-back frames: completing reception of current frame, starting reception of next one\n";

        // complete reception of previous frame
        cancelEvent(endRxMsg);
        EtherFrame *frame = frameBeingReceived;
        frameBeingReceived = NULL;
        frameReceptionComplete(frame);

        // start receiving next frame
        frameBeingReceived = (EtherFrame *)msg;
        scheduleEndRxPeriod(msg);
    }
    else // (receiveState==RECEIVING_STATE || receiveState==RX_COLLISION_STATE)
    {
        // handle overlapping receptions
        if (msg->kind()==JAM_SIGNAL)
        {
            if (numConcurrentTransmissions<=0)
                error("numConcurrentTransmissions=%d on jam arrival (stray jam?)",numConcurrentTransmissions);

            numConcurrentTransmissions--;
            EV << "Jam signal received, this marks end of one transmission\n";

            // by the time jamming ends, all transmissions will have been aborted
            if (numConcurrentTransmissions==0)
            {
                EV << "Last jam signal received, collision will ends when jam ends\n";
                cancelEvent(endRxMsg);
                scheduleAt(endRxTime, endRxMsg);
            }
        }
        else // ETH_FRAME or ETH_PAUSE
        {
            numConcurrentTransmissions++;
            if (endRxMsg->arrivalTime() < endRxTime)
            {
                // otherwise just wait until the end of the longest transmission
                EV << "Overlapping receptions -- setting collision state and extending collision period\n";
                cancelEvent(endRxMsg);
                scheduleAt(endRxTime, endRxMsg);
            }
            else
            {
                EV << "Overlapping receptions -- setting collision state\n";
            }
        }

        // delete collided frames: arrived frame as well as the one we're currently receiving
        delete msg;
        if (receiveState==RECEIVING_STATE)
        {
            delete frameBeingReceived;
            frameBeingReceived = NULL;

            numCollisions++;
            numCollisionsVector.record(numCollisions);
        }

        // go to collision state
        receiveState = RX_COLLISION_STATE;
    }
}


void EtherMAC::handleEndIFGPeriod()
{
    if (transmitState!=WAIT_IFG_STATE)
        error("Not in WAIT_IFG_STATE at the end of IFG period");

    if (outputbuffer.empty())
        error("End of IFG and no frame to transmit");

    // End of IFG period, okay to transmit, if Rx idle OR duplexChannel
    cMessage *frame = (cMessage *)outputbuffer.tail();
    EV << "IFG elapsed, now begin transmission of frame " << frame << endl;

    // Perform carrier extension if in Gigabit Ethernet
    if (carrierExtension && frame->length() < 8*GIGABIT_MIN_FRAME_WITH_EXT)
    {
        EV << "Performing carrier extension of small frame\n";
        frame->setLength(8*GIGABIT_MIN_FRAME_WITH_EXT);
    }

    // start frame burst, if enabled
    if (frameBursting)
    {
        EV << "Starting frame burst\n";
        framesSentInBurst = 0;
        bytesSentInBurst = 0;
    }

    // send frame to network
    startFrameTransmission();
}

void EtherMAC::startFrameTransmission()
{
    cMessage *origFrame = (cMessage *)outputbuffer.tail();
    EV << "Transmitting a copy of frame " << origFrame << endl;
    cMessage *frame = (cMessage *) origFrame->dup();

    // add preamble and SFD (Starting Frame Delimiter), then send out
    frame->setLength(frame->length()+8*PREAMBLE_BYTES+8*SFD_BYTES);
    send(frame, "LowerLayer_out");

    // update burst variables
    bytesSentInBurst = frame->length()/8;
    framesSentInBurst++;

    // check for collisions (there might be an ongoing reception which we don't know about, see below)
    if (!duplexChannel && receiveState!=RX_IDLE_STATE)
    {
        // During the IFG period the hardware cannot listen to the channel,
        // so it might happen that receptions have begun during the IFG,
        // and even collisions might be in progress.
        //
        // But we don't know of any ongoing transmission so we blindly
        // start transmitting, immediately collide and send a jam signal.
        //
        sendJamSignal();
        // numConcurrentTransmissions stays the same: +1 transmission, -1 jam

        if (receiveState==RECEIVING_STATE)
        {
            delete frameBeingReceived;
            frameBeingReceived = NULL;

            numCollisions++;
            numCollisionsVector.record(numCollisions);
        }
        // go to collision state
        receiveState = RX_COLLISION_STATE;
    }
    else
    {
        // no collision
        scheduleEndTxPeriod(frame);

        // only count transmissions in totalSuccessfulRxTxTime if channel is half-duplex
        if (!duplexChannel)
            channelBusySince = simTime();
    }
}

void EtherMAC::handleEndTxPeriod()
{
    // we only get here if transmission has finished successfully, without collision
    if (transmitState!=TRANSMITTING_STATE || (!duplexChannel && receiveState!=RX_IDLE_STATE))
        error("End of transmission, and incorrect state detected");

    if (outputbuffer.empty())
        error("Frame under transmission cannot be found");

    // get frame from buffer
    cMessage *frame = (cMessage*)outputbuffer.pop();

    numFramesSent++;
    numBytesSent += frame->length()/8;
    numFramesSentVector.record(numFramesSent);
    numBytesSentVector.record(numBytesSent);

    if (frame->kind()==ETH_PAUSE)
    {
        numPauseFramesSent++;
        numPauseFramesSentVector.record(numPauseFramesSent);
    }

    // only count transmissions in totalSuccessfulRxTxTime if channel is half-duplex
    if (!duplexChannel)
    {
        simtime_t dt = simTime()-channelBusySince;
        totalSuccessfulRxTxTime += dt;
    }

    EV << "Transmission of " << frame << " successfully completed\n";
    delete frame;
    backoffs = 0;

    // check for and obey received PAUSE frames after each transmission
    if (pauseUnitsRequested>0)
    {
        // if we received a PAUSE frame recently, go into PAUSE state
        EV << "Going to PAUSE mode for " << pauseUnitsRequested << " time units\n";
        scheduleEndPausePeriod(pauseUnitsRequested);
        pauseUnitsRequested = 0;
        return;
    }

    // Gigabit Ethernet: now decide if we transmit next frame right away (burst) or wait IFG
    // FIXME this is not entirely correct, there must be IFG between burst frames too
    bool burstFrame=false;
    if (frameBursting && !outputbuffer.empty())
    {
        // check if max bytes for burst not exceeded
        if (bytesSentInBurst<GIGABIT_MAX_BURST_BYTES)
        {
             burstFrame=true;
             EV << "Transmitting next frame in current burst\n";
        }
        else
        {
             EV << "Next frame does not fit in current burst\n";
        }
    }

    if (burstFrame)
        startFrameTransmission();
    else
        beginSendFrames();
}

void EtherMAC::handleEndRxPeriod()
{
    EV << "Frame reception complete\n";
    simtime_t dt = simTime()-channelBusySince;
    if (receiveState==RECEIVING_STATE) // i.e. not RX_COLLISION_STATE
    {
        EtherFrame *frame = frameBeingReceived;
        frameBeingReceived = NULL;
        frameReceptionComplete(frame);
        totalSuccessfulRxTxTime += dt;
    }
    else
    {
        totalCollisionTime += dt;
    }

    receiveState = RX_IDLE_STATE;
    numConcurrentTransmissions = 0;

    if (transmitState==TX_IDLE_STATE && !outputbuffer.empty())
    {
        EV << "Receiver now idle, can transmit frames in output buffer after IFG period\n";
        scheduleEndIFGPeriod();
    }
}

void EtherMAC::handleEndBackoffPeriod()
{
    if (transmitState != BACKOFF_STATE)
        error("At end of BACKOFF not in BACKOFF_STATE!");
    if (outputbuffer.empty())
        error("At end of BACKOFF and buffer empty!");

    if (receiveState==RX_IDLE_STATE)
    {
        EV << "Backoff period ended, wait IFG\n";
        scheduleEndIFGPeriod();
    }
    else
    {
        EV << "Backoff period ended but channel not free, idling\n";
        transmitState = TX_IDLE_STATE;
    }
}

void EtherMAC::handleEndJammingPeriod()
{
    if (transmitState != JAMMING_STATE)
        error("At end of JAMMING not in JAMMING_STATE!");
    EV << "Jamming finished, executing backoff\n";
    handleRetransmission();
}

void EtherMAC::handleEndPausePeriod()
{
    if (transmitState != PAUSE_STATE)
        error("At end of PAUSE not in PAUSE_STATE!");
    EV << "Pause finished, resuming transmissions\n";
    beginSendFrames();
}

void EtherMAC::sendJamSignal()
{
    cMessage *jam = new cMessage("JAM_SIGNAL", JAM_SIGNAL);
    jam->setLength(8*JAM_SIGNAL_BYTES);
    send(jam, "LowerLayer_out");

    scheduleAt(simTime()+jamDuration, endJammingMsg);
    transmitState = JAMMING_STATE;
}

void EtherMAC::scheduleEndIFGPeriod()
{
    scheduleAt(simTime()+interFrameGap, endIFGMsg);
    transmitState = WAIT_IFG_STATE;
}

void EtherMAC::scheduleEndTxPeriod(cMessage *frame)
{
    scheduleAt(simTime()+frame->length()*bitTime, endTxMsg);
    transmitState = TRANSMITTING_STATE;
}

void EtherMAC::scheduleEndRxPeriod(cMessage *frame)
{
    scheduleAt(simTime()+frame->length()*bitTime, endRxMsg);
    receiveState = RECEIVING_STATE;
}

void EtherMAC::scheduleEndPausePeriod(int pauseUnits)
{
    // length is interpreted as 512-bit-time units
    double pausePeriod = pauseUnits*PAUSE_BITTIME*bitTime;
    scheduleAt(simTime()+pausePeriod, endPauseMsg);
    transmitState = PAUSE_STATE;
}

void EtherMAC::frameReceptionComplete(EtherFrame *frame)
{
    int pauseUnits;

    switch (frame->kind())
    {
      case ETH_FRAME:
        processReceivedDataFrame((EtherFrame *)frame);
        break;

      case ETH_PAUSE:
        pauseUnits = ((EtherPauseFrame *)frame)->getPauseTime();
        delete frame;
        numPauseFramesRcvd++;
        numPauseFramesRcvdVector.record(numPauseFramesRcvd);
        processPauseCommand(pauseUnits);
        break;

      default:
        error("Invalid message kind %d",frame->kind());
    }
}

void EtherMAC::processReceivedDataFrame(EtherFrame *frame)
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
    frame->setLength(frame->length() - 8*PREAMBLE_BYTES - 8*SFD_BYTES);

    // statistics
    numFramesReceivedOK++;
    numBytesReceivedOK += frame->length()/8;
    numFramesReceivedOKVector.record(numFramesReceivedOK);
    numBytesReceivedOKVector.record(numBytesReceivedOK);

    // If not set to promiscuous = on, then checks if received frame contains destination MAC address
    // matching port's MAC address, also checks if broadcast bit is set
    if (!promiscuous && !frame->getDest().isBroadcast() && !frame->getDest().equals(myaddress))
    {
        EV << "Frame `" << frame->name() <<"' not destined to us, discarding\n";
        numDroppedNotForUs++;
        numDroppedNotForUsVector.record(numDroppedNotForUs);
        delete frame;
        return;
    }

    numFramesPassedToHL++;
    numFramesPassedToHLVector.record(numFramesPassedToHL);

    // pass up to upper layer
    send(frame, "UpperLayer_out");
}

void EtherMAC::processPauseCommand(int pauseUnits)
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

void EtherMAC::handleRetransmission()
{
    if (++backoffs > MAX_ATTEMPTS)
    {
        EV << "Number of retransmit attempts of frame exceeds maximum, cancelling transmission of frame\n";
        delete outputbuffer.pop();

        transmitState = TX_IDLE_STATE;
        backoffs = 0;
        // no beginSendFrames(), because end of jam signal sending will trigger it automatically
        return;
    }

    EV << "Executing backoff procedure\n";
    int backoffrange = (backoffs>=BACKOFF_RANGE_LIMIT) ? 1024 : (1 << backoffs);
    int slotNumber = intuniform(0,backoffrange-1);
    simtime_t backofftime = slotNumber*slotTime;

    scheduleAt(simTime()+backofftime, endBackoffMsg);
    transmitState = BACKOFF_STATE;

    numBackoffs++;
    numBackoffsVector.record(numBackoffs);
}

void EtherMAC::printState()
{
#define CASE(x) case x: EV << #x; break
    EV << "transmitState: ";
    switch (transmitState) {
        CASE(TX_IDLE_STATE);
        CASE(WAIT_IFG_STATE);
        CASE(TRANSMITTING_STATE);
        CASE(JAMMING_STATE);
        CASE(BACKOFF_STATE);
        CASE(PAUSE_STATE);
    }
    EV << ",  receiveState: ";
    switch (receiveState) {
        CASE(RX_IDLE_STATE);
        CASE(RECEIVING_STATE);
        CASE(RX_COLLISION_STATE);
    }
    EV << ",  backoffs: " << backoffs;
    EV << ",  numConcurrentTransmissions: " << numConcurrentTransmissions;
    EV << ",  queueLength: " << outputbuffer.length() << endl;
#undef CASE
}

void EtherMAC::beginSendFrames()
{
    if (!outputbuffer.empty())
    {
        // Other frames are queued, therefore wait IFG period and transmit next frame
        EV << "Transmit next frame in output queue, after IFG period\n";
        scheduleEndIFGPeriod();
    }
    else
    {
        // No more frames set transmitter to idle
        EV << "No more frames to send, transmitter set to idle\n";
        transmitState = TX_IDLE_STATE;
    }
}

void EtherMAC::finish()
{
    double t = simTime();
    simtime_t totalChannelIdleTime = t - totalSuccessfulRxTxTime - totalCollisionTime;
    recordScalar("simulated time", t);
    recordScalar("rx channel idle (%)", 100*totalChannelIdleTime/t);
    recordScalar("rx channel utilization (%)", 100*totalSuccessfulRxTxTime/t);
    recordScalar("rx channel collision (%)", 100*totalCollisionTime);
    recordScalar("frames sent",    numFramesSent);
    recordScalar("frames rcvd",    numFramesReceivedOK);
    recordScalar("bytes sent",     numBytesSent);
    recordScalar("bytes rcvd",     numBytesReceivedOK);
    recordScalar("frames from LLC dropped (queue full)", numDroppedQueueFull);
    recordScalar("frames dropped (bit error)",  numDroppedBitError);
    recordScalar("frames dropped (not for us)", numDroppedNotForUs);
    recordScalar("frames passed up to HL", numFramesPassedToHL);
    recordScalar("PAUSE frames sent",  numPauseFramesSent);
    recordScalar("PAUSE frames rcvd",  numPauseFramesRcvd);
    recordScalar("collisions",     numCollisions);
    recordScalar("backoffs",       numBackoffs);

    if (t>0)
    {
        recordScalar("frames/sec sent", numFramesSent/t);
        recordScalar("frames/sec rcvd", numFramesReceivedOK/t);
        recordScalar("bits/sec sent",   8*numBytesSent/t);
        recordScalar("bits/sec rcvd",   8*numBytesReceivedOK/t);
    }
}



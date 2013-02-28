//
// Copyright (C) 2013 OpenSim Ltd
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
// author: Zoltan Bojthe
//


#include "IdealRadio.h"


#define MK_TRANSMISSION_OVER  1
#define MK_RECEPTION_COMPLETE 2

simsignal_t IdealRadio::radioStateSignal = SIMSIGNAL_NULL;

Define_Module(IdealRadio);

IdealRadio::IdealRadio()
{
    rs = RadioState::IDLE;
    inTransmit = false;
    concurrentReceives = 0;
}

void IdealRadio::initialize(int stage)
{
    IdealChannelModelAccess::initialize(stage);

    EV << "Initializing IdealRadio, stage=" << stage << endl;

    if (stage == 0)
    {
        inTransmit = false;
        concurrentReceives = 0;

        upperLayerInGateId = gate("upperLayerIn")->getId();
        upperLayerOutGateId = gate("upperLayerOut")->getId();
        radioInGateId = gate("radioIn")->getId();

        gate(radioInGateId)->setDeliverOnReceptionStart(true);

        // read parameters
        transmissionRange = par("transmissionRange").doubleValue();
        bitrate = par("bitrate").doubleValue();
        drawCoverage = par("drawCoverage");

        rs = RadioState::IDLE;
        WATCH(rs);

        // signals
        radioStateSignal = registerSignal("radioState");
    }
    else if (stage == 2)
    {
        emit(radioStateSignal, rs);

        // draw the interference distance
        if (ev.isGUI() && drawCoverage)
            updateDisplayString();
    }
}

void IdealRadio::finish()
{
}

IdealRadio::~IdealRadio()
{
    // Clear the recvBuff
    for (RecvBuff::iterator it = recvBuff.begin(); it!=recvBuff.end(); ++it)
    {
        cMessage *endRxTimer = *it;
        cancelAndDelete(endRxTimer);
    }
    recvBuff.clear();
}

void IdealRadio::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        handleSelfMsg(msg);
    }
    else if (msg->getArrivalGateId() == upperLayerInGateId)
    {
        if (!msg->isPacket())
        {
            handleCommand(msg);
            return;
        }

        if (isEnabled())
        {
            handleUpperMsg(msg);
        }
        else
        {
            EV << "IdealRadio disabled. ignoring frame" << endl;
            delete msg;
        }
    }
    else if (msg->getArrivalGateId() == radioInGateId)
    {
        // must be an IdealAirFrame
        IdealAirFrame *airframe = check_and_cast<IdealAirFrame*>(msg);
        if (isEnabled())
        {
            handleLowerMsgStart(airframe);
        }
        else
        {
            EV << "IdealRadio disabled. ignoring airframe" << endl;
            delete msg;
        }
    }
    else
    {
        throw cRuntimeError("Model error: unknown arrival gate '%s'", msg->getArrivalGate()->getFullName());
        delete msg;
    }
}

IdealAirFrame *IdealRadio::encapsulatePacket(cPacket *frame)
{
    delete frame->removeControlInfo();

    // Note: we don't set length() of the IdealAirFrame, because duration will be used everywhere instead
    IdealAirFrame *airframe = createAirFrame();
    airframe->setName(frame->getName());
    airframe->setTransmissionRange(transmissionRange);
    airframe->encapsulate(frame);
    airframe->setDuration(airframe->getBitLength() / bitrate);
    airframe->setTransmissionStartPosition(getRadioPosition());

    EV << "Frame (" << frame->getClassName() << ")" << frame->getName()
       << " will be transmitted at " << (bitrate/1e6) << "Mbps" << endl;
    return airframe;
}

void IdealRadio::sendUp(IdealAirFrame *airframe)
{
    cPacket *frame = airframe->decapsulate();
    delete airframe;
    EV << "sending up frame " << frame->getName() << endl;
    send(frame, upperLayerOutGateId);
}

void IdealRadio::sendDown(IdealAirFrame *airframe)
{
    // change radio status
    EV << "sending, changing RadioState to TRANSMIT\n";
    inTransmit = true;
    updateRadioState();

    simtime_t endOfTransmission = simTime() + airframe->getDuration();
    sendToChannel(airframe);
    cMessage *timer = new cMessage("endTx", MK_TRANSMISSION_OVER);
    scheduleAt(endOfTransmission, timer);
}

/**
 * If a message is already being transmitted, an error is raised.
 *
 * Otherwise the RadioState is set to TRANSMIT and a timer is
 * started. When this timer expires the RadioState will be set back to RECV
 * (or IDLE respectively) again.
 */
void IdealRadio::handleUpperMsg(cMessage *msg)
{
    IdealAirFrame *airframe = encapsulatePacket(PK(msg));

    if (rs == RadioState::TRANSMIT)
        error("Trying to send a message while already transmitting -- MAC should "
              "take care this does not happen");

    sendDown(airframe);
}

void IdealRadio::handleCommand(cMessage *msg)
{
    throw cRuntimeError("Command '%s' not accepted.", msg->getName());
}

void IdealRadio::handleSelfMsg(cMessage *msg)
{
    EV << "IdealRadio::handleSelfMsg" << msg->getKind() << endl;
    if (msg->getKind() == MK_RECEPTION_COMPLETE)
    {
        EV << "frame is completely received now\n";

        // unbuffer the message
        IdealAirFrame *airframe = check_and_cast<IdealAirFrame *>(msg->removeControlInfo());
        bool found = false;
        for (RecvBuff::iterator it = recvBuff.begin(); it != recvBuff.end(); ++it)
        {
            if (*it == msg)
            {
                recvBuff.erase(it);
                found = true;
                break;
            }
        }
        if (!found)
            throw cRuntimeError("Model error: self message not found in recvBuff buffer");
        delete msg;
        handleLowerMsgEnd(airframe);
    }
    else if (msg->getKind() == MK_TRANSMISSION_OVER)
    {
        inTransmit = false;
        // Transmission has completed. The RadioState has to be changed
        // to IDLE or RECV, based on the noise level on the channel.
        // If the noise level is bigger than the sensitivity switch to receive mode,
        // otherwise to idle mode.

        // delete the timer
        delete msg;

        updateRadioState(); // now the radio changes the state and sends the signal
        EV << "transmission over, switch to state: "<< RadioState::stateName(rs) << endl;
    }
    else
    {
        error("Internal error: unknown self-message `%s'", msg->getName());
    }
    EV << "IdealRadio::handleSelfMsg END" << endl;
}

/**
 * This function is called right after a packet arrived, i.e. right
 * before it is buffered for 'transmission time'.
 *
 * The message is not treated as noise if the transmissionRange of the
 * received signal is higher than the distance.
 *
 * Update radioState.
 */
void IdealRadio::handleLowerMsgStart(IdealAirFrame *airframe)
{
    EV << "receiving frame " << airframe->getName() << endl;

    cMessage *endRxTimer = new cMessage("endRx", MK_RECEPTION_COMPLETE);
    endRxTimer->setControlInfo(airframe);
    recvBuff.push_back(endRxTimer);

    // NOTE: use arrivalTime instead of simTime, because we might be calling this
    // function during a channel change, when we're picking up ongoing transmissions
    // on the channel -- and then the message's arrival time is in the past!
    scheduleAt(airframe->getArrivalTime() + airframe->getDuration(), endRxTimer);
    concurrentReceives++;
    // check the RadioState and update if necessary
    updateRadioState();
}

/**
 * This function is called right after the transmission is over.
 * Additionally the RadioState has to be updated.
 */
void IdealRadio::handleLowerMsgEnd(IdealAirFrame *airframe)
{
    concurrentReceives--;
    sendUp(airframe);
    updateRadioState();
}

void IdealRadio::updateRadioState()
{
    RadioState::State newState = isEnabled()
            ? (inTransmit ? RadioState::TRANSMIT : ((concurrentReceives>0) ? RadioState::RECV : RadioState::IDLE))
            : RadioState::SLEEP;
    if (rs != newState)
    {
        rs = newState;
        if (rs == RadioState::SLEEP)
        {
            // Clear the recvBuff
            for (RecvBuff::iterator it = recvBuff.begin(); it!=recvBuff.end(); ++it)
            {
                cMessage *endRxTimer = *it;
                cancelAndDelete(endRxTimer);
            }
            recvBuff.clear();
        }
        emit(radioStateSignal, newState);
    }
}

void IdealRadio::updateDisplayString()
{
    if (myRadioRef) {
        cDisplayString& d = hostModule->getDisplayString();

        // communication area
        // FIXME this overrides the ranges if more than one radio is present is a host
        d.removeTag("r1");
        d.insertTag("r1");
        d.setTagArg("r1", 0, (long) transmissionRange);  // TODO 
        d.setTagArg("r1", 2, "gray");
        d.removeTag("r2");
    }
}

void IdealRadio::enablingInitialization()
{
    cc->enableReception(myRadioRef);
    updateRadioState();
}

void IdealRadio::disablingInitialization()
{
    cc->disableReception(myRadioRef);
    updateRadioState();
}

void IdealRadio::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    IdealChannelModelAccess::receiveSignal(source, signalID, obj);
}


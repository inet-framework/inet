/* -*- mode:c++ -*- ********************************************************
 * file:        GilbertElliotSnr.cc
 *
 * author:      Marc Loebbers
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 ***************************************************************************/


#include "GilbertElliotSnr.h"


Define_Module(GilbertElliotSnr);

GilbertElliotSnr::GilbertElliotSnr()
{
    stateChange = NULL;
}

GilbertElliotSnr::~GilbertElliotSnr()
{
    cancelAndDelete(stateChange);
}

/**
 * All values not present in the ned file will be read from the
 * ChannelControl module or assigned default values.
 */
void GilbertElliotSnr::initialize(int stage)
{
    SnrEval::initialize(stage);

    if (stage == 0)
    {
        meanGood = par("meanGood");
        meanBad = par("meanBad");
        stateChange = new cMessage();
        stateChange->setKind(38);
        state = GOOD;
        scheduleAt(simTime() + exponential(meanGood, 0), stateChange);
        EV << "GE state will change at: " << stateChange->getArrivalTime() << endl;
    }
}

/**
 *
 * If the selfMsg is the time to indicate a transmission is over, the
 * RadioState has to be changed based on the noise level on the
 * channel. If the noise level is bigger than the sensitivity switch
 * to receive mode otherwise to idle mode.
 *
 * If the timer indicates a change of state in the Gilbert-Elliot
 * model, the state variable has to be changed.
 */
void GilbertElliotSnr::handleSelfMsg(cMessage *msg)
{
    if (msg->getKind() == TRANSM_OVER)
    {

        if (noiseLevel < sensitivity)
        {
            // set the RadioState to IDLE
            rs.setState(RadioState::IDLE);
            EV << "transmission over, switch to idle mode (state:IDLE)\n";
            nb->fireChangeNotification(NF_RADIOSTATE_CHANGED, &rs);
        }
        else
        {
            // set the RadioState to RECV
            rs.setState(RadioState::RECV);
            EV << "transmission over but noise level to high, switch to recv mode (state:RECV)\n";
            nb->fireChangeNotification(NF_RADIOSTATE_CHANGED, &rs);
        }

        // delete the timer
        delete msg;
    }
    else if (msg == stateChange)
    {
        EV << "GilbertElliot state changed!\n";
        if (state == GOOD)
        {
            state = BAD;
            for (RecvBuff::iterator it = recvBuff.begin(); it != recvBuff.end(); it++)
                (it->first)->setBitError(true);
            scheduleAt(simTime() + exponential(meanBad, 0), stateChange);
        }
        else if (state == BAD)
        {
            state = GOOD;
            scheduleAt(simTime() + exponential(meanGood, 0), stateChange);
        }
    }
    else
        error("Internal error: unknown self-message `%s'", msg->getName());
}


/**
 * This function is called right after a packet arrived, i.e. right
 * before it is buffered for 'transmission time'.
 *
 * First the receive power of the packet has to be calculated and is
 * stored in the recvBuff. Afterwards it has to be decided whether the
 * packet is just noise or a "real" packet that needs to be received.
 *
 * The message is not treated as noise if all of the follwoing
 * conditions apply:
 *
 * -# the power of the received signal is higher than the
 * sensitivity.
 * -# the host is currently not sending a message
 * -# no other packet is already being received
 *
 * If all conditions apply a new SnrList is created and the RadioState
 * is changed to RECV. If the Gilbert Elliot model is in state BAD,
 * the frame is marked as corrupted. The cMessage's bitError flag is used
 * for it.
 * If the packet is just noise the receive power is added to the noise
 * Level of the channel. Additionally the snr information of the
 * currently received message (if any) has to be updated as
 * well as the RadioState.
 */
void GilbertElliotSnr::handleLowerMsgStart(AirFrame * frame)
{
    // Calculate the receive power of the message

    // calculate distance
    const Coord& myPos = getMyPosition();
    const Coord& framePos = frame->getSenderPos();
    double distance = myPos.distance(framePos);

    // receive power
    double rcvdPower = calcRcvdPower(frame->getPSend(), distance);

    if (state == BAD)
        frame->setBitError(true);

    // store the receive power in the recvBuff
    recvBuff[frame] = rcvdPower;

    // if receive power is bigger than sensitivity and if not sending
    // and currently not receiving another message
    if (rcvdPower >= sensitivity && rs.getState() != RadioState::TRANSMIT && snrInfo.ptr == NULL)
    {
        EV << "receiving frame\n";

        // Put frame and related SnrList in receive buffer
        SnrList snrList;        //defined in SnrList.h!!
        snrInfo.ptr = frame;
        snrInfo.rcvdPower = rcvdPower;
        snrInfo.sList = snrList;

        // add initial snr value
        addNewSnr();

        if (rs.getState() != RadioState::RECV)
        {
            // publish new RadioState
            rs.setState(RadioState::RECV);
            EV << "publish new RadioState:RECV\n";
            nb->fireChangeNotification(NF_RADIOSTATE_CHANGED, &rs);
        }
    }
    // receive power is to low or another message is being send or received
    else
    {
        EV << "frame is just noise\n";
        //add receive power to the noise level
        noiseLevel += rcvdPower;

        // if a message is being received add a new snr value
        if (snrInfo.ptr != NULL)
        {
            // update snr info for currently being received message
            EV << "add new snr value to snr list of message being received\n";
            addNewSnr();
        }

        // update the RadioState if the noiseLevel exceeded the threshold
        // and the radio is currently not in receive or in send mode
        if (noiseLevel >= sensitivity && rs.getState() == RadioState::IDLE)
        {
            // publish new RadioState
            rs.setState(RadioState::RECV);
            EV << "publish new RadioState:RECV\n";
            nb->fireChangeNotification(NF_RADIOSTATE_CHANGED, &rs);
        }
    }
}


/**
 * This function is called right after the transmission is over,
 * i.e. right after unbuffering.  The noise level of the channel and
 * the snr information of the buffered messages have to be updated.
 *
 * Additionally the RadioState has to be updated.
 *
 * If the corresponding AirFrame was not only noise the corresponding
 * SnrList and the AirFrame are sent to the decider.
 *
 */
void GilbertElliotSnr::handleLowerMsgEnd(AirFrame * frame)
{

    //check state again:
    if (state == BAD)
        frame->setBitError(true);

    // check if message has to be send to the decider
    if (snrInfo.ptr == frame)
    {
        EV << "reception of frame over, preparing to send packet to upper layer\n";
        // get Packet and list out of the receive buffer:
        SnrList list;
        list = snrInfo.sList;

        // delete the pointer to indicate that no message is currently
        // being received and clear the list
        snrInfo.ptr = NULL;
        snrInfo.sList.clear();

        // delete the frame from the recvBuff
        recvBuff.erase(frame);

        //Don't forget to send:
        sendUp(frame, list);
        EV << "packet sent to the decider\n";
    }
    // all other messages are noise
    else
    {
        EV << "reception of noise message over, removing recvdPower from noiseLevel....\n";
        // get the rcvdPower and subtract it from the noiseLevel
        noiseLevel -= recvBuff[frame];

        // delete message from the recvBuff
        recvBuff.erase(frame);

        // update snr info for message currently being received if any
        if (snrInfo.ptr != NULL)
        {
            addNewSnr();
        }

        // message should be deleted
        delete frame;
        EV << "message deleted\n";
    }

    // check the RadioState and update if necessary
    // change to idle if noiseLevel smaller than threshold and state was
    // not idle before
    // do not change state if currently sending or receiving a message!!!
    if (noiseLevel < sensitivity && rs.getState() == RadioState::RECV && snrInfo.ptr == NULL)
    {
        // publish the new RadioState:
        EV << "new RadioState is IDLE\n";
        rs.setState(RadioState::IDLE);
        nb->fireChangeNotification(NF_RADIOSTATE_CHANGED, &rs);
    }
}

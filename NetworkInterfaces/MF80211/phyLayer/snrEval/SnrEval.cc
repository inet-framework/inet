/* -*- mode:c++ -*- ********************************************************
 * file:        SnrEval.cc
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


#include "SnrEval.h"
#include "FWMath.h"


std::ostream& operator<<(std::ostream& os, const RadioState& rs)
{
    os << "state=" << rs.getState();
    return os;
}

Define_Module(SnrEval);

/**
 * All values not present in the ned file will be read from the
 * ChannelControl module or assigned default values.
 */
void SnrEval::initialize(int stage)
{
    BasicSnrEval::initialize(stage);

    if (stage == 0)
    {
        if (hasPar("thermalNoise"))
            thermalNoise = FWMath::dBm2mW(par("thermalNoise"));
        else
            thermalNoise = FWMath::dBm2mW(-100);

        if (hasPar("carrierFrequency"))
            carrierFrequency = par("carrierFrequency");
        else
            carrierFrequency = cc->par("carrierFrequency");

        if (hasPar("sensitivity"))
            sensitivity = FWMath::dBm2mW(par("sensitivity"));
        else
            sensitivity = FWMath::dBm2mW(-90);

        if (hasPar("pathLossAlpha"))
        {
            pathLossAlpha = par("pathLossAlpha");
            if (pathLossAlpha < (double) (cc->par("alpha")))
                error("SnrEval::initialize() pathLossAlpha can't be smaller than in "
                      "ChannelControl. Please adjust your omnetpp.ini file accordingly");
        }
        else
            pathLossAlpha = cc->par("alpha");

        // initialize noiseLevel
        noiseLevel = thermalNoise;

        EV << "Initialized channel with noise: " << noiseLevel << " sensitivity: " << sensitivity <<
            endl;

        // initialize the pointer of the snrInfo with NULL to indicate
        // that currently no message is received
        snrInfo.ptr = NULL;

        // Initialize radio state. If thermal noise is already to high, radio
        // state has to be initialized as RECV
        rs.setState(RadioState::IDLE);
        if (noiseLevel >= sensitivity)
            rs.setState(RadioState::RECV);

        WATCH(noiseLevel);
        WATCH(rs);
        WATCH(channel);
    }
    else if (stage == 1)
    {
        // tell initial value to MAC; must be done in stage 1, because they
        // subscribe in stage 0
        nb->fireChangeNotification(NF_RADIOSTATE_CHANGED, &rs);
    }
}

/**
 * delete the RadioState
 */
void SnrEval::finish()
{
    BasicSnrEval::finish();
}

/**
 * If a message is already being send the newly arrived one is
 * deletetd and a warning is printed.
 *
 * Otherwise the RadioState is set to TRANSMIT and a timer is
 * started. When this timer expires the RadioState is set back to RECV
 * (or IDLE respectively) again.
 *
 * If the host is receiving a packet this packet is from now on only
 * considered as noise.
*/
void SnrEval::handleUpperMsg(AirFrame * frame)
{
    if (rs.getState() == RadioState::TRANSMIT)
        error("Trying to send a message although already sending -- MAC should "
              "take care this does not happen");

    // if a packet was being received, it is corrupted now as should be treated as noise
    if (snrInfo.ptr != NULL)
    {
        EV << "Sending a message while receiving another. The received one is now corrupted.\n";

        // remove the snr information stored for the message currently being
        // received. This message is treated as noise now and the
        // receive power has to be added to the noiseLevel

        // delete the pointer to indicate that no message is being received
        snrInfo.ptr = NULL;
        // clear the snr list
        snrInfo.sList.clear();
        // add the receive power to the noise level
        noiseLevel += snrInfo.rcvdPower;
    }

    // now we are done with all the exception handling and can take care
    // about the "real" stuff

    //change radio status
    rs.setState(RadioState::TRANSMIT);
    EV << "sending, changing RadioState to TRANSMIT\n";
    nb->fireChangeNotification(NF_RADIOSTATE_CHANGED, &rs);

    cMessage *timer = new cMessage(NULL, TRANSM_OVER);
    scheduleAt(simTime() + frame->getDuration(), timer);
    sendDown(frame);
}


/**
 * The only self message that can arrive is a timer to indicate that
 * sending of a message is completed.
 *
 * The RadioState has to be changed based on the noise level on the
 * channel. If the noise level is bigger than the sensitivity switch
 * to receive mode odtherwise to idle mode.
 */
void SnrEval::handleSelfMsg(cMessage *msg)
{
    if (msg->kind() == TRANSM_OVER)
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
    else
        error("unknown selfMsg erhalten.....");
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
 * is changed to RECV.
 *
 * If the packet is just noise the receive power is added to the noise
 * Level of the channel. Additionally the snr information of the
 * currently being received message (if any) has to be updated as
 * well as the RadioState.
 */
void SnrEval::handleLowerMsgStart(AirFrame * frame)
{
    // Calculate the receive power of the message

    // calculate distance
    const Coord& myPos = myPosition();
    const Coord& framePos = frame->getSenderPos();
    double distance = myPos.distance(framePos);

    // calculate receive power
    double rcvdPower = calcRcvdPower(frame->getPSend(), distance);

    // store the receive power in the recvBuff
    recvBuff[frame] = rcvdPower;

    // if receive power is bigger than sensitivity and if not sending
    // and currently not receiving another message and the message has
    // arrived in time
    // NOTE: a message may have arrival time in the past here when we are
    // processing ongoing transmissions during a channel change
    if (frame->arrivalTime() == simTime() && rcvdPower >= sensitivity && rs.getState() != RadioState::TRANSMIT && snrInfo.ptr == NULL)
    {
        EV << "receiving frame " << frame->name() << endl;

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
    // receive power is too low or another message is being sent or received
    else
    {
        EV << "frame " << frame->name() << " is just noise\n";
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
 */
void SnrEval::handleLowerMsgEnd(AirFrame * frame)
{
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


/**
 * The Snr information of the buffered message is updated....
 */
void SnrEval::addNewSnr()
{
    //print("NoiseLevel: "<<noiseLevel<<" recvPower: "<<snrInfo.rcvdPower);

    SnrListEntry listEntry;     //create a new entry
    listEntry.time = simTime();
    listEntry.snr = snrInfo.rcvdPower / noiseLevel;
    snrInfo.sList.push_back(listEntry);
    //print("New Snr added: "<<listEntry.snr<<" at time:"<<simTime());
}


/**
 * This function simply calculates with how much power the signal
 * arrives "here". If a different way of computing the path loss is
 * required this function can be redefined.
 */
double SnrEval::calcRcvdPower(double pSend, double distance)
{
    double speedOfLight = 300000000.0;
    double waveLength = speedOfLight / carrierFrequency;
    return (pSend * waveLength * waveLength / (16 * M_PI * M_PI * pow(distance, pathLossAlpha)));
}

void SnrEval::changeChannel(const int channel)
{
    if (channel == this->channel)
        return;
    else if (rs.getState() == RadioState::TRANSMIT)
        // TODO: is there a reasonable solution for this?
        error("changing channel while transmitting is not allowed");
    else if (rs.getState() == RadioState::RECV)
        // TODO: what should we do with the message being received?
        error("changing channel while receiving is not _yet_ allowed");
    else
    {
        EV << "changing channel to " << channel << " during radio idle\n";

        this->channel = channel;
        cc->updateHostChannel(myHostRef, channel);
        ChannelControl::TransmissionList tl = cc->getOngoingTransmissions(channel);

        // clear snr info
        snrInfo.ptr = NULL;
        snrInfo.sList.clear();

        for (ChannelControl::TransmissionList::const_iterator it = tl.begin(); it != tl.end(); ++it)
        {
            AirFrame *frame = *it;
            // time for the message to reach us
            double distance = myHostRef->pos.distance(frame->getSenderPos());

            EV << "processing ongoing transmission for channel change\n";

            // if this transmission is on our new channel and it would reach us in the future, then schedule it
            if (channel == frame->getChannelNumber())
            {
                // if there is a message on the air which will reach us in the future
                if (frame->timestamp() + distance / LIGHT_SPEED >= simTime())
                {
                    EV << "scheduling ongoing transmission to be processed in the future\n";

                    // we need to send to each radioIn[] gate
                    cGate *radioGate = gate("radioIn");
                    for (int i = 0; i < radioGate->size(); i++)
                        sendDirect((cMessage*)frame->dup(), frame->timestamp() + distance / LIGHT_SPEED - simTime(), this, radioGate->id() + i);
                }
                // if we hear some part of the message
                else if (frame->timestamp() + frame->getDuration() + distance / LIGHT_SPEED > simTime())
                {
                    EV << "processing ongoing transmission as noise\n";

                    AirFrame *frameDup = (AirFrame*)frame->dup();
                    frameDup->setArrivalTime(frame->timestamp() + distance / LIGHT_SPEED);
                    handleLowerMsgStart(frameDup);
                    bufferMsg(frameDup);
                }
            }
        }
    }
}

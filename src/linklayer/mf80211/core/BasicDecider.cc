/***************************************************************************
 * file:        BasicDecider.cc
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


#include "BasicDecider.h"


#define coreEV (ev.isDisabled()||!coreDebug) ? ev : ev << logName() << "::BasicDecider: "

Define_Module(BasicDecider);

/**
 * First we have to initialize the module from which we derived ours,
 * in this case BasicModule.
 *
 * Then we have to intialize the gates and - if necessary - some own
 * variables.
 *
 */
void BasicDecider::initialize(int stage)
{
    BasicModule::initialize(stage);

    if (stage == 0)
    {
        uppergateOut = findGate("uppergateOut");
        lowergateIn = findGate("lowergateIn");
        numRcvd = 0;
        numSentUp = 0;
        WATCH(numRcvd);
        WATCH(numSentUp);
    }
}

/**
 * The basic handle message function.
 *
 * Depending on the gate a message arrives handleMessage just calls
 * different handle*Msg functions to further process the message.
 *
 * The decider module only handles messages from lower layers. All
 * messages from upper layers are directly passed to the snrEval layer
 * and cannot be processed in the decider module
 *
 * You should not make any changes in this function but implement all
 * your functionality into the handle*Msg functions called from here.
 *
 * @sa handleLowerMsg, handleSelfMsg
 */
void BasicDecider::handleMessage(cMessage *msg)
{
    if (msg->getArrivalGateId() == lowergateIn)
    {
        numRcvd++;

        //remove the control info from the AirFrame
        SnrControlInfo *cInfo = static_cast<SnrControlInfo *>(msg->removeControlInfo());

        // read in the snrList from the control info
        handleLowerMsg(check_and_cast<AirFrame *>(msg), cInfo->getSnrList());

        // delete the control info
        delete cInfo;

    }
    else if (msg->isSelfMessage())
    {
        handleSelfMsg(msg);
    }
}

/**
 * Decapsulate and send message to the upper layer.
 *
 * to be called within @ref handleLowerMsg.
 */
void BasicDecider::sendUp(AirFrame * frame)
{
    numSentUp++;
    cPacket *macMsg = frame->decapsulate();
    send(macMsg, uppergateOut);
    coreEV << "sending up msg " << frame->getName() << endl;
    delete frame;
}

/**
 * Redefine this function if you want to process messages from the
 * channel before they are forwarded to upper layers
 *
 * In this function it has to be decided whether this message got lost
 * or not. This can be done with a simple SNR threshold or with
 * transformations of SNR into bit error probabilities...
 *
 * If you want to forward the message to upper layers please use @ref
 * sendUp which will decapsulate the MAC frame before sending
 */
void BasicDecider::handleLowerMsg(AirFrame * frame, SnrList& snrList)
{

    bool correct = true;
    //check the entries in the snrList if a level greater than the
    //acceptable minimum occured:
    double min = 0.0000000000000001;    // just a senseless example

    for (SnrList::iterator iter = snrList.begin(); iter != snrList.end(); iter++)
    {
        coreEV << "snr=" << iter->snr << "...\n";
        if (iter->snr < min)
            correct = false;
    }

    if (correct)
    {
        coreEV << "frame " << frame->getName() << " correct\n";
        sendUp(frame);
    }
    else
    {
        coreEV << "frame " << frame->getName() << " corrupted -> delete\n";
        delete frame;
    }
}

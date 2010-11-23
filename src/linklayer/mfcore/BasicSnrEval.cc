/***************************************************************************
 * file:        BasicSnrEval.cc
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


#include "BasicSnrEval.h"
//#include "MacPkt_m.h"
#include "TransmComplete_m.h"


#define coreEV (ev.isDisabled()||!coreDebug) ? ev : ev << logName() << "::BasicSnrEval: "

Define_Module(BasicSnrEval);

/**
 * First we have to initialize the module from which we derived ours,
 * in this case ChannelAccess.
 *
 * Then we have to intialize the gates and - if necessary - some own
 * variables.
 *
 * If you want to use your own AirFrames you have to redefine createCapsulePkt
 * function.
 */
void BasicSnrEval::initialize(int stage)
{
    // ChannelAccess::initialize(stage);
    ChannelAccessExtended::initialize(stage);

    coreEV << "Initializing BasicSnrEval, stage=" << stage << endl;

    if (stage == 0)
    {
        gate("radioIn")->setDeliverOnReceptionStart(true);

        uppergateIn = findGate("uppergateIn");
        uppergateOut = findGate("uppergateOut");

        headerLength = par("headerLength");
        bitrate = par("bitrate");

        transmitterPower = par("transmitterPower");
        carrierFrequency = par("carrierFrequency");

        // transmitter power CANNOT be greater than in ChannelControl
        if (transmitterPower > (double) (cc->par("pMax")))
            error("transmitterPower cannot be bigger than pMax in ChannelControl!");
    }
}

/**
 *
Determine if the packet must be delete or process
 */

bool BasicSnrEval::processAirFrame(AirFrame *airframe)
{

	int chnum = airframe->getChannelNumber();
	AirFrameExtended *airframeext = dynamic_cast<AirFrameExtended *>(airframe);
	if (ccExt && airframeext)
	{
		double perc = ccExt->getPercentage();
		double fqFrame = airframeext->getCarrierFrequency();
		if (fqFrame > 0.0 && carrierFrequency>0.0)
		{
			if (chnum == getChannelNumber() && (fabs((fqFrame - carrierFrequency)/carrierFrequency)<=perc))
				return true;
			else
				return false;
		}
		else
			return (chnum == getChannelNumber());
	}
	else
	{
		return (chnum == getChannelNumber());
	}
}

/**
 * The basic handle message function.
 *
 * Depending on the gate a message arrives handleMessage just calls
 * different handle*Msg functions to further process the message.
 *
 * Messages from the channel are also buffered here in order to
 * simulate a transmission delay
 *
 * You should not make any changes in this function but implement all
 * your functionality into the handle*Msg functions called from here.
 *
 * @sa handleUpperMsg, handleLowerMsgStart, handleLowerMsgEnd,
 * handleSelfMsg
 */
void BasicSnrEval::handleMessage(cMessage *msg)
{
    if (msg->getArrivalGateId() == uppergateIn)
    {
        AirFrame *frame = encapsMsg(PK(msg));
        handleUpperMsg(frame);
    }
    else if (msg->isSelfMessage())
    {
        if (dynamic_cast<TransmComplete *>(msg) != 0)
        {
            coreEV << "frame is completely received now\n";

            // unbuffer the message
            AirFrame *frame = unbufferMsg(msg);

            handleLowerMsgEnd(frame);
        }
        else
            handleSelfMsg(msg);
    }
    else if (processAirFrame (check_and_cast<AirFrame *>(msg)))
    {
        // must be an AirFrame
        AirFrame *frame = (AirFrame *) msg;
        handleLowerMsgStart(frame);
        bufferMsg(frame);
    }
    else
    {
        EV << "listening to different channel when receiving message -- dropping it\n";
        delete msg;
    }
}

/**
 * The packet is put in a buffer for the time the transmission would
 * last in reality. A timer indicates when the transmission is
 * complete. So, look at unbufferMsg to see what happens when the
 * transmission is complete..
 */
void BasicSnrEval::bufferMsg(AirFrame * frame) //FIXME: add explicit simtime_t atTime arg?
{
    // set timer to indicate transmission is complete
    TransmComplete *endRxTimer = new TransmComplete(NULL);
    endRxTimer->setContextPointer(frame);
    frame->setContextPointer(endRxTimer);
    // NOTE: use arrivalTime instead of simTime, because we might be calling this
    // function during a channel change, when we're picking up ongoing transmissions
    // on the channel -- and then the message's arrival time is in the past!
    scheduleAt(frame->getArrivalTime() + frame->getDuration(), endRxTimer);
}

/**
 * This function encapsulates messages from the upper layer into an
 * AirFrame, copies the type and channel fields, adds the
 * headerLength, sets the pSend (transmitterPower) and returns the
 * AirFrame.
 */
AirFrame *BasicSnrEval::encapsMsg(cPacket *msg)
{
	AirFrameExtended *frame = createCapsulePkt();
    frame->setName(msg->getName());
    frame->setPSend(transmitterPower);
    frame->setBitLength(headerLength);
    frame->setChannelNumber(getChannelNumber());
    frame->encapsulate(msg);
    frame->setDuration(calcDuration(frame));
    frame->setSenderPos(getMyPosition());
    frame->setCarrierFrequency(carrierFrequency);
    return frame;
}

/**
 * Usually the duration is just the frame length divided by the
 * bitrate. However there may be cases (like 802.11) where the header
 * has a different modulation (and thus a different bitrate) than the
 * rest of the message.
 *
 * Just redefine this function in such a case!
 */
double BasicSnrEval::calcDuration(cPacket *af)
{
    double duration;
    duration = (double) af->getBitLength() / (double) bitrate;
    return duration;
}

/**
 * Attach control info to the message and send message to the upper
 * layer.
 *
 * @param msg AirFrame to pass to the decider
 * @param list Snr list to attach as control info
 *
 * to be called within @ref handleLowerMsgEnd.
 */
void BasicSnrEval::sendUp(AirFrame *msg, SnrList& list)
{
    // create ControlInfo
    SnrControlInfo *cInfo = new SnrControlInfo;
    // attach the list to cInfo
    cInfo->setSnrList(list);
    // attach the cInfo to the AirFrame
    msg->setControlInfo(cInfo);

    send(msg, uppergateOut);
}

/**
 * @brief Sends a message to the channel
 *
 * @sa sendToChannel
 */
void BasicSnrEval::sendDown(AirFrame *msg)
{
    sendToChannel(msg);
}

/**
 * Get the context pointer to the now completely received AirFrame and
 * delete the self message
 */
AirFrame *BasicSnrEval::unbufferMsg(cMessage *msg)
{
    AirFrame *frame = (AirFrame *) msg->getContextPointer();
    //delete the self message
    delete msg;

    return frame;
}

/**
 * Redefine this function if you want to process messages from upper
 * layers before they are send to the channel.
 *
 * The MAC frame is already encapsulated in an AirFrame and all standard
 * header fields are set.
 *
 * To forward the message to lower layers after processing it please
 * use @ref sendDown. It will take care of decapsulation and anything
 * else needed
 */
void BasicSnrEval::handleUpperMsg(AirFrame * frame)
{
    sendDown(frame);
}

/**
 * Redefine this function if you want to process messages from the
 * channel before they are forwarded to upper layers
 *
 * This function is called right before a packet is handed on to the
 * upper layer, i.e. right after unbufferMsg. Again you can caluculate
 * some more SNR information if you want.
 *
 * You have to copy / create the SnrList related to the message and
 * pass it to sendUp() if you want to pass the message to the decider.
 *
 * Do not forget to send the message to the upper layer with sendUp()
 *
 * For a "real" implementaion take a look at SnrEval
 *
 * @sa SnrList, SnrEval
 */
void BasicSnrEval::handleLowerMsgEnd(AirFrame * frame)
{
    coreEV << "in handleLowerMsgEnd\n";

    // We need to create a "dummy" snr list that we can pass together
    // with the message to the decider module so that also the
    // BasicSnrEval is able to work.
    SnrList snrList;

    // However you can take this as a reference how to create your own
    // snr entries.

    // Everytime you want to add something to the snr information list
    // it has to look like this:
    // 1. create a list entry and fill the fields
    SnrListEntry listEntry;
    listEntry.time = simTime();
    listEntry.snr = 3;          //just a senseless example

    // 2. add an entry to the SnrList
    snrList.push_back(listEntry);

    // 3. pass the message together with the list to the decider
    sendUp(frame, snrList);
}

/**
 * Redefine this function if you want to process messages from the
 * channel before they are forwarded to upper layers
 *
 * This function is called right after a message is received,
 * i.e. right before it is buffered for 'transmission time'.
 *
 * Here you should decide whether the message is "really" received or
 * whether it's receive power is so low that it is just treated as
 * noise.
 *
 * If the energy of the message is high enough to really receive it
 * you should create an snr list (@ref SnrList) to be able to store
 * sn(i)r information for that message. Every time a new message
 * arrives you can add a new snr value together with a timestamp to
 * that list. Make sure to store a pointer to the mesage together with
 * the snr information to be able to retrieve it later.
 *
 * In this function also an initial SNR value can be calculated for
 * this message.
 *
 * Please take a look at SnrEval to see a "real" example.
 *
 * @sa SnrList, SnrEval
 */
void BasicSnrEval::handleLowerMsgStart(AirFrame * frame)
{
    coreEV << "in handleLowerMsgStart, receiving frame " << frame->getName() << endl;

    //calculate the receive power

    // calculate snr information, like snr=pSend/noise or whatever....

    // if receive power is actually high enough to be able to read the
    // message and no other message is currently being received, store
    // the snr information for the message someweher where you can find
    // it in handleLowerMsgEnd
}

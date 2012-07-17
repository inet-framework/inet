///
/// @file   OnuMacLayer2.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   Jul/6/2012
///
/// @brief  Implements 'OnuMacLayer2' class for a Hybrid TDM/WDM-PON ONU.
///
/// @remarks Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///

// For debugging
// #define DEBUG_ONU_MAC_LAYER

#include "OnuMacLayer2.h"

// Register modules.
Define_Module(OnuMacLayer2);

OnuMacLayer2::OnuMacLayer2()
{
    endTxMsg = NULL;
}

OnuMacLayer2::~OnuMacLayer2()
{
    cancelAndDelete(endTxMsg);
}

///
/// Handles an Ethernet frame from UNI (i.e., Ethernet switch).
/// Puts the Ethernet frame into a FIFO, if there is enough space.
/// Otherwise, drops it.
///
/// @param[in] frame	an EtherFrame pointer
///
void OnuMacLayer2::handleEthernetFrameFromUni(EtherFrame *frame)
{
    Enter_Method("handleEthernetFrameFromUni()");

    if ((queue.empty() == true) && (txState == STATE_IDLE))
    {
        scheduleAt(simTime()+(frame->getBitLength()+INTERFRAME_GAP_BITS)/lineRate, endTxMsg);
        send(frame, "wdmg$o");
        txState = STATE_TRANSMITTING;
    }
    else
    {
        if (busyQueue + frame->getBitLength() <= queueSize)
        {
            busyQueue += frame->getBitLength();
            queue.insert(frame);

#ifdef DEBUG_ONU_MAC_LAYER
            ev << getFullPath() << ": busyQueue = " << busyQueue << endl;
#endif
        }
        else
        {
            delete frame;

#ifdef DEBUG_ONU_MAC_LAYER
            ev << getFullPath() << ": A frame from UNI dropped!" << endl;
#endif
        }
    }
}

///
/// Handles the EndTxMsg to model the end of Ethernet frame transmission
/// (including guard band).
///
void OnuMacLayer2::handleEndTxMsg()
{
    txState = STATE_IDLE;

    if (queue.empty() != true)
    {
        EtherFrame *frame = (EtherFrame *) queue.pop();
        txState = STATE_TRANSMITTING;
        scheduleAt(simTime()+(frame->getBitLength()+INTERFRAME_GAP_BITS)/lineRate, endTxMsg);
        send(frame, "wdmg$o");
    }
}

///
/// Initializes member variables and allocates memory for them, if needed.
///
void OnuMacLayer2::initialize()
{
    cModule *onu = getParentModule();
    cDatarateChannel *chan = check_and_cast<cDatarateChannel *>(onu->gate("phyg$o")->getChannel());
    lineRate = chan->getDatarate();
    queueSize = par("queueSize");
    busyQueue = 0;
    txState = STATE_IDLE;

    EV << "ONU initialization results:" << endl;
    EV << "- Line rate = " << lineRate << endl;

    // initialize self messages
    endTxMsg = new cMessage("EndTransmission", MSG_TX_END);
}

///
/// Handles messages by calling appropriate functions for their processing.
/// Starts simulation and run until it will be terminated by kernel.
///
/// @param[in] msg a cMessage pointer
///
void OnuMacLayer2::handleMessage(cMessage *msg)
{
    //#ifdef TRACE_MSG
    //	ev.printf();
    //	PrintMsg(*msg);
    //#endif

    if (msg->isSelfMessage() == true)
    {
        if (msg->getKind() == MSG_TX_END)
        {
            handleEndTxMsg();
        }
        else
        {   // unknown type of message
            error("%s: ERROR: unexpected message kind %d received", getFullPath().c_str(), msg->getKind());
        }
    }
    else
    {
        if (msg->getArrivalGateId() == findGate("wdmg$i"))
        { // Ethernet frame from the WDM layer
            send(msg, "ethg$o");
        }
        else if (msg->getArrivalGateId() == findGate("ethg$i"))
        { // Ethernet frame from the upper layer (i.e., Ethernet switch)
            handleEthernetFrameFromUni(check_and_cast<EtherFrame *>(msg));
        }
    }
}

///
/// Does post-processing.
///
void OnuMacLayer2::finish()
{
}

//
// Copyright (C) 2010 Kyeong Soo (Joseph) Kim
// Copyright (C) 2006 Levente Meszaros
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
#include "EtherMAC3.h"
#include "IPassiveQueue.h"
#include "NotificationBoard.h"
#include "NotifierConsts.h"

Define_Module(EtherMAC3);

//EtherMAC3::EtherMAC3()
//{
//}

//void EtherMAC3::initialize()
//{
//    EtherMACBase::initialize();
//
//    duplexMode = true;
//    calculateParameters();
//
//    beginSendFrames();
//}

//void EtherMAC3::initializeTxrate()
//{
//    // if we're connected, find the gate with transmission rate
//    txrate = 0;
//
//    if (connected)
//    {
//        // obtain txrate from channel. As a side effect, this also asserts
//        // that the other end is an EtherMAC3, since normal EtherMAC
//        // insists that the connection has *no* datarate set.
//        // if we're connected, get the gate with transmission rate
//        cChannel *datarateChannel = physOutGate->getTransmissionChannel();
//        txrate = datarateChannel->par("datarate").doubleValue();
//    }
//}

void EtherMAC3::initializeStatistics()
{
    numDroppedTxQueueOverflow= 0;

    WATCH(numDroppedTxQueueOverflow);

    numDroppedTxQueueOverflowVector.setName("framesDroppedTxQueueOverflow");
}

//void EtherMAC3::handleMessage(cMessage *msg)
//{
//    if (!connected)
//        processMessageWhenNotConnected(msg);
//    else if (disabled)
//        processMessageWhenDisabled(msg);
//    else if (msg->isSelfMessage())
//    {
//        EV << "Self-message " << msg << " received\n";
//
//        if (msg == endTxMsg)
//            handleEndTxPeriod();
//        else if (msg == endIFGMsg)
//            handleEndIFGPeriod();
//        else if (msg == endPauseMsg)
//            handleEndPausePeriod();
//        else
//            error("Unknown self message received!");
//    }
//    else
//    {
//        if (msg->getArrivalGate() == gate("upperLayerIn"))
//            processFrameFromUpperLayer(check_and_cast<EtherFrame *>(msg));
//        else if (msg->getArrivalGate() == gate("phys$i"))
//            processMsgFromNetwork(check_and_cast<EtherFrame *>(msg));
//        else
//            error("Message received from unknown gate!");
//    }
//
//    if (ev.isGUI())  updateDisplayString();
//}
//
//void EtherMAC3::startFrameTransmission()
//{
//    EtherFrame *origFrame = (EtherFrame *)txQueue.front();
//    EV << "Transmitting a copy of frame " << origFrame << endl;
//
//    EtherFrame *frame = (EtherFrame *) origFrame->dup();
//    frame->addByteLength(PREAMBLE_BYTES+SFD_BYTES);
//
//    if (hasSubscribers)
//    {
//        // fire notification
//        notifDetails.setPacket(frame);
//        nb->fireChangeNotification(NF_PP_TX_BEGIN, &notifDetails);
//    }
//
//    // fill in src address if not set
//    if (frame->getSrc().isUnspecified())
//        frame->setSrc(address);
//
//    // send
//    EV << "Starting transmission of " << frame << endl;
//    send(frame, physOutGate);
//    scheduleEndTxPeriod(frame);
//
//    // update burst variables
//    if (frameBursting)
//    {
//        bytesSentInBurst = frame->getByteLength();
//        framesSentInBurst++;
//    }
//}

void EtherMAC3::processFrameFromUpperLayer(EtherFrame *frame)
{
//    EtherMACBase::processFrameFromUpperLayer(frame);

	// beginning of modified implementation of EtherMACBase::processFrameFromUpperLayer(frame)
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

        if (txQueueLimit && txQueue.length()>=txQueueLimit)
        {
//            error("txQueue length exceeds %d -- this is probably due to "
//                  "a bogus app model generating excessive traffic "
//                  "(or if this is normal, increase txQueueLimit!)",
//                  txQueueLimit);
            numDroppedTxQueueOverflow++;
			numDroppedTxQueueOverflowVector.record(numDroppedTxQueueOverflow);
            delete frame;
        }
        else
        {
            // fill in src address if not set
            if (frame->getSrc().isUnspecified())
                frame->setSrc(address);

            // store frame and possibly begin transmitting
            EV << "Packet " << frame << " arrived from higher layers, enqueueing\n";
            txQueue.insert(frame);
        }
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
	// end of modified implementation of EtherMACBase::processFrameFromUpperLayer(frame)

    if (transmitState == TX_IDLE_STATE && !txQueue.empty()) // now we need to check txQueue
        startFrameTransmission();
}

//void EtherMAC3::processMsgFromNetwork(cPacket *msg)
//{
//    EtherMACBase::processMsgFromNetwork(msg);
//    EtherFrame *frame = check_and_cast<EtherFrame *>(msg);
//
//    if (hasSubscribers)
//    {
//        // fire notification
//        notifDetails.setPacket(frame);
//        nb->fireChangeNotification(NF_PP_RX_END, &notifDetails);
//    }
//
//    if (checkDestinationAddress(frame))
//        frameReceptionComplete(frame);
//}
//
//void EtherMAC3::handleEndIFGPeriod()
//{
//    EtherMACBase::handleEndIFGPeriod();
//
//    startFrameTransmission();
//}
//
//void EtherMAC3::handleEndTxPeriod()
//{
//    if (hasSubscribers)
//    {
//        // fire notification
//        notifDetails.setPacket((cPacket *)txQueue.front());
//        nb->fireChangeNotification(NF_PP_TX_END, &notifDetails);
//    }
//
//    if (checkAndScheduleEndPausePeriod())
//        return;
//
//    EtherMACBase::handleEndTxPeriod();
//
//    beginSendFrames();
//}
//
//void EtherMAC3::updateHasSubcribers()
//{
//    hasSubscribers = nb->hasSubscribers(NF_PP_TX_BEGIN) ||
//                     nb->hasSubscribers(NF_PP_TX_END) ||
//                     nb->hasSubscribers(NF_PP_RX_END);
//}

void EtherMAC3::finish()
{
    if (!disabled)
    {
//        simtime_t t = simTime();
        recordScalar("frames dropped (txQueue overflow)",  numDroppedTxQueueOverflow);
    }
}

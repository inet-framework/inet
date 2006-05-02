//
// Copyright (C) 2006 Levente Meszaros
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "EtherDuplexMAC.h"
#include "IPassiveQueue.h"
#include "NotificationBoard.h"
#include "NotifierConsts.h"

Define_Module(EtherDuplexMAC);

EtherDuplexMAC::EtherDuplexMAC()
{
}

void EtherDuplexMAC::initialize()
{
    EtherMACBase::initialize();

    duplexMode = true;
    calculateParameters();

    beginSendFrames();
}

void EtherDuplexMAC::initializeTxrate()
{
    // if we're connected, find the gate with transmission rate
    cGate *g = gate("physOut");
    txrate = 0;

    if (connected)
    {
        while (g)
        {
            // does this gate have data rate?
            cSimpleChannel *chan = dynamic_cast<cSimpleChannel*>(g->channel());
            if (chan && (txrate=chan->datarate())>0)
                break;
            // otherwise just check next connection in path
            g = g->toGate();
        }

        if (!g)
            error("gate physOut must be connected (directly or indirectly) to a link with data rate");
    }
}

void EtherDuplexMAC::handleMessage(cMessage *msg)
{
    if (!connected)
        processMessageWhenNotConnected(msg);
    else if (disabled)
        processMessageWhenDisabled(msg);
    else if (msg->isSelfMessage())
    {
        EV << "Self-message " << msg << " received\n";

        if (msg == endTxMsg)
            handleEndTxPeriod();
        else if (msg == endIFGMsg)
            handleEndIFGPeriod();
        else if (msg == endPauseMsg)
            handleEndPausePeriod();
        else
            error("Unknown self message received!");
    }
    else
    {
        if (msg->arrivalGate() == gate("upperLayerIn"))
            processFrameFromUpperLayer(check_and_cast<EtherFrame *>(msg));
        else if (msg->arrivalGate() == gate("physIn"))
            processMsgFromNetwork(check_and_cast<EtherFrame *>(msg));
        else
            error("Message received from unknown gate!");
    } 

    if (ev.isGUI())  updateDisplayString();
}

void EtherDuplexMAC::startFrameTransmission()
{
    EtherFrame *origFrame = (EtherFrame *)txQueue.tail();
    EV << "Transmitting a copy of frame " << origFrame << endl;

    EtherFrame *frame = (EtherFrame *) origFrame->dup();
    frame->addByteLength(PREAMBLE_BYTES+SFD_BYTES);

    fireChangeNotification(NF_PP_TX_BEGIN, frame);

    // fill in src address if not set
    if (frame->getSrc().isUnspecified())
        frame->setSrc(address);

    // send
    EV << "Starting transmission of " << frame << endl;
    send(frame, "physOut");
    scheduleEndTxPeriod(frame);

    // update burst variables
    if (frameBursting)
    {
        bytesSentInBurst = frame->byteLength();
        framesSentInBurst++;
    }
}

void EtherDuplexMAC::processFrameFromUpperLayer(EtherFrame *frame)
{
    EtherMACBase::processFrameFromUpperLayer(frame);

    if (transmitState == TX_IDLE_STATE)
        startFrameTransmission();
}

void EtherDuplexMAC::processMsgFromNetwork(cMessage *msg)
{
    EtherMACBase::processMsgFromNetwork(msg);
    EtherFrame *frame = check_and_cast<EtherFrame *>(msg);

    fireChangeNotification(NF_PP_RX_END, frame);

    if (checkDestinationAddress(frame))
        frameReceptionComplete(frame);
}

void EtherDuplexMAC::handleEndIFGPeriod()
{
    EtherMACBase::handleEndIFGPeriod();
    
    startFrameTransmission();
}

void EtherDuplexMAC::handleEndTxPeriod()
{
    fireChangeNotification(NF_PP_TX_END, (cMessage *)txQueue.tail());
    
    if (checkAndScheduleEndPausePeriod())
        return;

    EtherMACBase::handleEndTxPeriod();
    
    beginSendFrames();
}
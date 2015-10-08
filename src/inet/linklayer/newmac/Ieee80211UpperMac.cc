//
// Copyright (C) 2015 Andras Varga
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
// Author: Andras Varga
//

#include "Ieee80211UpperMac.h"
#include "Ieee80211NewMac.h"
#include "inet_old/linklayer/contract/PhyControlInfo_m.h"

namespace inet {

void Ieee80211UpperMac::upperFrameReceived(Ieee80211DataOrMgmtFrame* frame)
{
    if (queueModule)
        queueModule->requestPacket();
    // check for queue overflow
    if (maxQueueSize && (int)transmissionQueue.size() == maxQueueSize)
    {
        EV << "message " << frame << " received from higher layer but MAC queue is full, dropping message\n";
        delete frame;
        return;
    }

    // must be a Ieee80211DataOrMgmtFrame, within the max size because we don't support fragmentation
    if (frame->getByteLength() > fragmentationThreshold)
        opp_error("message from higher layer (%s)%s is too long for 802.11b, %d bytes (fragmentation is not supported yet)",
              frame->getClassName(), frame->getName(), (int)(frame->getByteLength()));
    EV << "frame " << frame << " received from higher layer, receiver = " << frame->getReceiverAddress() << endl;

    ASSERT(!frame->getReceiverAddress().isUnspecified());

    // fill in missing fields (receiver address, seq number), and insert into the queue
    frame->setTransmitterAddress(mac->getAddress());
    frame->setSequenceNumber(sequenceNumber);
    sequenceNumber = (sequenceNumber+1) % 4096;  //XXX seqNum must be checked upon reception of frames!

    if (frameExchange)
        transmissionQueue.push_back(frame);
    else if (frame->getByteLength() >= mac->rtsThreshold)
    {
        frameExchange = new Ieee80211SendRtsCtsDataAckFrameExchange(mac, this, frame);
        frameExchange->start();
    }
    else
    {
        frameExchange = new Ieee80211SendDataAckFrameExchange(mac, this, frame);
        frameExchange->start();
    }
}

void Ieee80211UpperMac::lowerFrameReceived(Ieee80211Frame* frame)
{
    if (dynamic_cast<Ieee80211RTSFrame *>(frame))
    {
        sendCts(dynamic_cast<Ieee80211RTSFrame *>(frame));
    }
    else if (dynamic_cast<Ieee80211DataOrMgmtFrame *>(frame))
    {
        sendAck(dynamic_cast<Ieee80211DataOrMgmtFrame *>(frame));
        mac->sendUp(frame);
    }
    else if (frameExchange)
        frameExchange->lowerFrameReceived(frame);
    else
    {
        EV_INFO << "Dropped frame " << frame->getName() << std::endl;
        delete frame;
    }
}

void Ieee80211UpperMac::transmissionFinished()
{
   if (frameExchange) // TODO:
       frameExchange->transmissionFinished();
}

void Ieee80211UpperMac::initializeQueueModule()
{
    // use of external queue module is optional -- find it if there's one specified
    if (mac->par("queueModule").stringValue()[0])
    {
        cModule *module = mac->getParentModule()->getSubmodule(mac->par("queueModule").stringValue());
        queueModule = check_and_cast<IPassiveQueue *>(module);

        EV << "Requesting first two frames from queue module\n";
        queueModule->requestPacket();
    }
}

Ieee80211UpperMac::Ieee80211UpperMac(Ieee80211NewMac* mac) : Ieee80211MacPlugin(mac)
{
    maxQueueSize = mac->par("maxQueueSize");
    cwMinData = mac->par("cwMinData");
    if (cwMinData == -1) cwMinData = CW_MIN;
    ASSERT(cwMinData >= 0);

    cwMinBroadcast = mac->par("cwMinBroadcast");
    if (cwMinBroadcast == -1) cwMinBroadcast = 31;
    ASSERT(cwMinBroadcast >= 0);

    initializeQueueModule();
}

Ieee80211DataOrMgmtFrame *Ieee80211UpperMac::setDataFrameDuration(Ieee80211DataOrMgmtFrame *frameToSend)
{
    Ieee80211DataOrMgmtFrame *frame = (Ieee80211DataOrMgmtFrame *)frameToSend->dup();

    if (isBroadcast(frameToSend))
        frame->setDuration(0);
    else if (!frameToSend->getMoreFragments())
        frame->setDuration(getSIFS() + computeFrameDuration(LENGTH_ACK, mac->basicBitrate));
    else
        // FIXME: shouldn't we use the next frame to be sent?
        frame->setDuration(3 * getSIFS() + 2 * computeFrameDuration(LENGTH_ACK, mac->basicBitrate)
                + computeFrameDuration(frameToSend));

    return frame;
}

Ieee80211ACKFrame *Ieee80211UpperMac::buildACKFrame(Ieee80211DataOrMgmtFrame *frameToACK)
{
    Ieee80211ACKFrame *frame = new Ieee80211ACKFrame("ACK Frame");
    frame->setReceiverAddress(frameToACK->getTransmitterAddress());

    if (!frameToACK->getMoreFragments())
        frame->setDuration(0);
    else
        frame->setDuration(frameToACK->getDuration() - getSIFS()
                - computeFrameDuration(LENGTH_ACK, mac->basicBitrate));

    return frame;
}

Ieee80211DataOrMgmtFrame *Ieee80211UpperMac::buildBroadcastFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    Ieee80211DataOrMgmtFrame *frame = (Ieee80211DataOrMgmtFrame *)frameToSend->dup();
    frame->setDuration(0);
    return frame;
}

double Ieee80211UpperMac::computeFrameDuration(Ieee80211Frame *msg)
{
    return computeFrameDuration(msg->getBitLength(), mac->bitrate);
}

double Ieee80211UpperMac::computeFrameDuration(int bits, double bitrate)
{
    return bits / bitrate + PHY_HEADER_LENGTH / BITRATE_HEADER;
}

simtime_t Ieee80211UpperMac::getSIFS() const
{
// TODO:   return aRxRFDelay() + aRxPLCPDelay() + aMACProcessingDelay() + aRxTxTurnaroundTime();
    return SIFS;
}

simtime_t Ieee80211UpperMac::getPIFS() const
{
    return getSIFS() + mac->getSlotTime();
}

simtime_t Ieee80211UpperMac::getDIFS() const
{
    return getSIFS() + 2 * mac->getSlotTime();
}

simtime_t Ieee80211UpperMac::getEIFS() const
{
// FIXME:   return getSIFS() + getDIFS() + (8 * ACKSize + aPreambleLength + aPLCPHeaderLength) / lowestDatarate;
    return getSIFS() + getDIFS() + (8 * LENGTH_ACK + PHY_HEADER_LENGTH) / BITRATE_HEADER;
}

Ieee80211Frame *Ieee80211UpperMac::setBasicBitrate(Ieee80211Frame *frame)
{
    ASSERT(frame->getControlInfo()==NULL);
    PhyControlInfo *ctrl = new PhyControlInfo();
    ctrl->setBitrate(mac->basicBitrate);
    frame->setControlInfo(ctrl);
    return frame;
}

bool Ieee80211UpperMac::isForUs(Ieee80211Frame *frame) const
{
    return frame && frame->getReceiverAddress() == mac->getAddress();
}

bool Ieee80211UpperMac::isBroadcast(Ieee80211Frame *frame) const
{
    return frame && frame->getReceiverAddress().isBroadcast();
}

void Ieee80211UpperMac::frameExchangeFinished(Ieee80211FrameExchange* what, bool successful)
{
    EV_INFO << "Frame exchange finished" << std::endl;
    delete frameExchange;
    frameExchange = nullptr;
    if (!transmissionQueue.empty())
    {
        Ieee80211DataOrMgmtFrame *frame = check_and_cast<Ieee80211DataOrMgmtFrame *>(transmissionQueue.front());
        transmissionQueue.pop_front();
        if (frame->getByteLength() >= mac->rtsThreshold)
            frameExchange = new Ieee80211SendRtsCtsDataAckFrameExchange(mac, this, frame);
        else
            frameExchange = new Ieee80211SendDataAckFrameExchange(mac, this, frame);
        frameExchange->start();
    }
}

void Ieee80211UpperMac::sendAck(Ieee80211DataOrMgmtFrame* frame)
{
    Ieee80211ACKFrame *ackFrame = buildACKFrame(frame);
    mac->transmitImmediateFrame(ackFrame, getSIFS());
}

void Ieee80211UpperMac::sendCts(Ieee80211RTSFrame* frame)
{
    Ieee80211CTSFrame *ctsFrame = buildCtsFrame(frame);
    mac->transmitImmediateFrame(ctsFrame, getSIFS());
}

Ieee80211CTSFrame* Ieee80211UpperMac::buildCtsFrame(Ieee80211RTSFrame* rtsFrame)
{
    Ieee80211CTSFrame *frame = new Ieee80211CTSFrame("CTS Frame");
    frame->setReceiverAddress(rtsFrame->getTransmitterAddress());
    frame->setDuration(rtsFrame->getDuration() - getSIFS()
            - computeFrameDuration(LENGTH_CTS, mac->basicBitrate));
    return frame;
}

Ieee80211UpperMac::~Ieee80211UpperMac()
{
    while(!transmissionQueue.empty())
    {
        Ieee80211Frame *temp = transmissionQueue.front();
        transmissionQueue.pop_front();
        delete temp;
    }
}

void Ieee80211UpperMac::handleMessage(cMessage* msg)
{
}


} /* namespace inet */


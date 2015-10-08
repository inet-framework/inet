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

#include "UpperMac.h"
#include "Ieee80211NewMac.h"
#include "IUpperMacContext.h"
#include "inet/common/queue/IPassiveQueue.h"
#include "inet/common/ModuleAccess.h"
#include "FrameExchanges.h"

namespace inet {
namespace ieee80211 {

Define_Module(UpperMac);

UpperMac::UpperMac()
{
}

UpperMac::~UpperMac()
{
    while(!transmissionQueue.empty())
    {
        Ieee80211Frame *temp = transmissionQueue.front();
        transmissionQueue.pop_front();
        delete temp;
    }
}

void UpperMac::initialize()
{
    mac = check_and_cast<Ieee80211NewMac*>(getParentModule());  //TODO
    tx = check_and_cast<ITx*>(getModuleByPath("^.tx"));  //TODO
    rx = check_and_cast<IRx*>(getModuleByPath("^.rx"));  //TODO

    maxQueueSize = mac->par("maxQueueSize");
    initializeQueueModule();
}

void UpperMac::handleMessage(cMessage* msg)
{
    if (msg->getContextPointer() != nullptr)
        ((MacPlugin *)msg->getContextPointer())->handleMessage(msg);
    else
        ASSERT(false);
}

void UpperMac::initializeQueueModule()
{
    // use of external queue module is optional -- find it if there's one specified
    if (mac->par("queueModule").stringValue()[0])
    {
        cModule *module = getModuleFromPar<cModule>(mac->par("queueModule"), mac);
        queueModule = check_and_cast<IPassiveQueue *>(module);

        EV_DEBUG << "Requesting first two frames from queue module\n";
        queueModule->requestPacket();
    }
}

void UpperMac::upperFrameReceived(Ieee80211DataOrMgmtFrame* frame)
{
    Enter_Method("upperFrameReceived()");
    take(frame);

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
    EV_INFO << "Frame " << frame << " received from higher layer, receiver = " << frame->getReceiverAddress() << endl;
    ASSERT(!frame->getReceiverAddress().isUnspecified());

    // fill in missing fields (receiver address, seq number), and insert into the queue
    frame->setTransmitterAddress(context->getAddress());
    frame->setSequenceNumber(sequenceNumber);
    sequenceNumber = (sequenceNumber+1) % 4096;  //XXX seqNum must be checked upon reception of frames!
    context->setDataBitrate(frame);
    if (frameExchange)
        transmissionQueue.push_back(frame);
    else
    {
        frameExchange = new Ieee80211SendDataWithAckFrameExchange(this, context, this, frame);
        frameExchange->start();
    }
}

void UpperMac::lowerFrameReceived(Ieee80211Frame* frame)
{
    Enter_Method("lowerFrameReceived()");
    EV_INFO << "Lower frame received" << std::endl;
    take(frame);

    if (context->isForUs(frame))
    {
        if (Ieee80211RTSFrame *rtsFrame = dynamic_cast<Ieee80211RTSFrame *>(frame))
        {
            sendCts(rtsFrame);
        }
        else if (Ieee80211DataOrMgmtFrame *dataOrMgmtFrame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(frame))
        {
            sendAck(dataOrMgmtFrame);
            mac->sendUp(dataOrMgmtFrame);
        }
        else if (frameExchange)
        {
            bool processed = frameExchange->lowerFrameReceived(frame);
            if (!processed)
            {
                EV_INFO << "Unexpected frame " << frame->getName() << "\n";
                // TODO: do something
            }
        }
        else
        {
            EV_INFO << "Dropped frame " << frame->getName() << std::endl;
            delete frame;
        }
    }
    else
    {
        EV_INFO << "This frame is not for us" << std::endl;
        delete frame;
    }
}

void UpperMac::transmissionComplete(ITx::ICallback *callback, int txIndex)
{
    Enter_Method("transmissionComplete()");
    callback->transmissionComplete(txIndex);
}

void UpperMac::transmissionComplete(int txIndex)
{
    Enter_Method("transmissionComplete()");
    //TODO
}

void UpperMac::internalCollision(int txIndex)
{
    Enter_Method("internalCollision()");
    //TODO
}

void UpperMac::frameExchangeFinished(IFrameExchange* what, bool successful)
{
    EV_INFO << "Frame exchange finished" << std::endl;
    delete frameExchange;
    frameExchange = nullptr;
    if (!transmissionQueue.empty())
    {
        Ieee80211DataOrMgmtFrame *frame = check_and_cast<Ieee80211DataOrMgmtFrame *>(transmissionQueue.front());
        transmissionQueue.pop_front();
        frameExchange = new Ieee80211SendDataWithAckFrameExchange(this, context, this, frame);
        frameExchange->start();
    }
}

Ieee80211DataOrMgmtFrame *UpperMac::buildBroadcastFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    Ieee80211DataOrMgmtFrame *frame = (Ieee80211DataOrMgmtFrame *)frameToSend->dup();
    frame->setDuration(0);
    return frame;
}

void UpperMac::sendAck(Ieee80211DataOrMgmtFrame* frame)
{
    Ieee80211ACKFrame *ackFrame = context->buildAckFrame(frame);
    tx->transmitImmediateFrame(ackFrame, context->getSIFS(), this);
}

void UpperMac::sendCts(Ieee80211RTSFrame* frame)
{
    Ieee80211CTSFrame *ctsFrame = context->buildCtsFrame(frame);
    tx->transmitImmediateFrame(ctsFrame, context->getSIFS(), this);
}

} // namespace ieee80211
} // namespace inet


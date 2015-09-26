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

#include "BasicUpperMac.h"
#include "UpperMacContext.h"
#include "Ieee80211NewMac.h"
#include "IRx.h"
#include "IContentionTx.h"
#include "IImmediateTx.h"
#include "IUpperMacContext.h"
#include "FrameExchanges.h"
#include "inet/common/queue/IPassiveQueue.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"

namespace inet {
namespace ieee80211 {

Define_Module(BasicUpperMac);

BasicUpperMac::BasicUpperMac()
{
}

BasicUpperMac::~BasicUpperMac()
{
    int n = context ? context->getNumAccessCategories() : 0;
    for (int i = 0; i < n; i++) {
        delete acData[i].frameExchange;
        while (!acData[i].transmissionQueue.empty()) {
            Ieee80211Frame *temp = acData[i].transmissionQueue.front();
            acData[i].transmissionQueue.pop_front();
            delete temp;
        }
    }
    delete[] acData;
    delete context;
}

void BasicUpperMac::initialize()
{
    mac = check_and_cast<Ieee80211NewMac *>(getModuleByPath(par("macModule")));
    rx = check_and_cast<IRx *>(getModuleByPath(par("rxModule")));

    maxQueueSize = mac->par("maxQueueSize");
    initializeQueueModule();

    context = createContext();
    rx->setAddress(context->getAddress());

    acData = new AccessCategoryData[context->getNumAccessCategories()];

    WATCH(maxQueueSize);
    WATCH(fragmentationThreshold);
    WATCH(sequenceNumber);
    //WATCH_LIST(transmissionQueue);
}

IUpperMacContext *BasicUpperMac::createContext()
{
    IImmediateTx *immediateTx = check_and_cast<IImmediateTx *>(getModuleByPath(par("immediateTxModule")));
    IContentionTx **contentionTx = nullptr;
    collectContentionTxModules(getModuleByPath(par("firstContentionTxModule")), contentionTx);

    MACAddress address(mac->par("address").stringValue());    // note: we rely on MAC to have replaced "auto" with concrete address by now

    const Ieee80211ModeSet *modeSet = Ieee80211ModeSet::getModeSet(*par("opMode").stringValue());
    double bitrate = par("bitrate");
    const IIeee80211Mode *dataFrameMode = (bitrate == -1) ? modeSet->getFastestMode() : modeSet->getMode(bps(bitrate));
    const IIeee80211Mode *basicFrameMode = modeSet->getSlowestMode();
    const IIeee80211Mode *controlFrameMode = modeSet->getSlowestMode(); //TODO check

    int rtsThreshold = par("rtsThresholdBytes");
    int shortRetryLimit = par("retryLimit");
    if (shortRetryLimit == -1)
        shortRetryLimit = 7;
    ASSERT(shortRetryLimit > 0);

    bool useEDCA = false; //TODO

    return new UpperMacContext(address, dataFrameMode, basicFrameMode, controlFrameMode, shortRetryLimit, rtsThreshold, useEDCA, immediateTx, contentionTx);
}

void BasicUpperMac::handleMessage(cMessage *msg)
{
    if (msg->getContextPointer() != nullptr)
        ((MacPlugin *)msg->getContextPointer())->handleSelfMessage(msg);
    else
        ASSERT(false);
}

void BasicUpperMac::initializeQueueModule()
{
    // use of external queue module is optional -- find it if there's one specified
    if (mac->par("queueModule").stringValue()[0]) {
        cModule *module = getModuleFromPar<cModule>(mac->par("queueModule"), mac);
        queueModule = check_and_cast<IPassiveQueue *>(module);

        EV_DEBUG << "Requesting first two frames from queue module\n";
        queueModule->requestPacket();
    }
}

void BasicUpperMac::upperFrameReceived(Ieee80211DataOrMgmtFrame *frame)
{
    Enter_Method("upperFrameReceived(\"%s\")", frame->getName());
    take(frame);

    if (queueModule)
        queueModule->requestPacket();  //TODO: use internal queue only! (remove queue from mgmt module)

    int ac = classifyFrame(frame);

    // check for queue overflow
    if (maxQueueSize && (int)acData[ac].transmissionQueue.size() == maxQueueSize) {
        EV << "message " << frame << " received from higher layer but MAC queue is full, dropping message\n";
        delete frame;
        return;
    }

    // must be a Ieee80211DataOrMgmtFrame, within the max size because we don't support fragmentation
    if (frame->getByteLength() > fragmentationThreshold)
        throw cRuntimeError("message from higher layer (%s)%s is too long for 802.11b, %d bytes (fragmentation is not supported yet)",
                frame->getClassName(), frame->getName(), (int)(frame->getByteLength()));
    EV_INFO << "Frame " << frame << " received from higher layer, receiver = " << frame->getReceiverAddress() << endl;
    ASSERT(!frame->getReceiverAddress().isUnspecified());

    // fill in missing fields (receiver address, seq number), and insert into the queue
    frame->setTransmitterAddress(context->getAddress());
    frame->setSequenceNumber(sequenceNumber);
    sequenceNumber = (sequenceNumber + 1) % 4096;    //XXX seqNum must be checked upon reception of frames!
    if (acData[ac].frameExchange)
        acData[ac].transmissionQueue.push_back(frame);
    else {
        startSendDataFrameExchange(frame, ac);
    }
}

int BasicUpperMac::classifyFrame(Ieee80211DataOrMgmtFrame *frame)
{
    return intrand(context->getNumAccessCategories()); //TODO temporary hack
}

void BasicUpperMac::lowerFrameReceived(Ieee80211Frame *frame)
{
    Enter_Method("lowerFrameReceived(\"%s\")", frame->getName());
    delete frame->removeControlInfo();          //TODO
    take(frame);

    if (context->isForUs(frame)) {
        if (Ieee80211RTSFrame *rtsFrame = dynamic_cast<Ieee80211RTSFrame *>(frame)) {
            sendCts(rtsFrame);
        }
        else if (Ieee80211DataOrMgmtFrame *dataOrMgmtFrame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(frame)) {
            if (!context->isBroadcast(frame) && !context->isMulticast(frame))
                sendAck(dataOrMgmtFrame);
            mac->sendUp(dataOrMgmtFrame);
        }
        else {
            // offer frame to all ongoing frame exchanges
            int n = context->getNumAccessCategories();
            bool processed = false;
            for (int i = 0; i < n && !processed; i++)
                if (acData[i].frameExchange)
                    processed = acData[i].frameExchange->lowerFrameReceived(frame);
            if (!processed) {
                EV_INFO << "Unexpected frame " << frame->getName() << ", dropping\n";
                delete frame;
            }
        }
    }
    else {
        EV_INFO << "This frame is not for us" << std::endl;    //TODO except when in an AP
        delete frame;
    }
}

void BasicUpperMac::transmissionComplete(ITxCallback *callback, int txIndex)
{
    Enter_Method("transmissionComplete()");
    if (callback)
        callback->transmissionComplete(txIndex);
}

void BasicUpperMac::internalCollision(ITxCallback *callback, int txIndex)
{
    Enter_Method("transmissionComplete()");
    if (callback)
        callback->internalCollision(txIndex);
}

void BasicUpperMac::startSendDataFrameExchange(Ieee80211DataOrMgmtFrame *frame, int ac)
{
    ASSERT(!acData[ac].frameExchange);

    IFrameExchange *frameExchange;
    int txIndex = ac; //TODO

    if (context->isBroadcast(frame) || context->isMulticast(frame))
        context->setBasicBitrate(frame);
    else
        context->setDataBitrate(frame);

    bool useRtsCts = frame->getByteLength() > context->getRtsThreshold();
    if (context->isBroadcast(frame) || context->isMulticast(frame))
        frameExchange = new SendMulticastDataFrameExchange(this, context, this, frame, txIndex, ac);
    else if (useRtsCts)
        frameExchange = new SendDataWithRtsCtsFrameExchange(this, context, this, frame, txIndex, ac);
    else
        frameExchange = new SendDataWithAckFrameExchange(this, context, this, frame, txIndex, ac);

    frameExchange->start();
    acData[ac].frameExchange = frameExchange;
}

void BasicUpperMac::frameExchangeFinished(IFrameExchange *what, bool successful)
{
    EV_INFO << "Frame exchange finished" << std::endl;

    // find ac for this frame exchange
    int n = context->getNumAccessCategories();
    int ac = -1;
    for (int i = 0; i < n; i++)
        if (acData[i].frameExchange == what)
            ac = i;
    ASSERT(ac != -1);

    delete acData[ac].frameExchange;
    acData[ac].frameExchange = nullptr;

    if (!acData[ac].transmissionQueue.empty()) {
        Ieee80211DataOrMgmtFrame *frame = check_and_cast<Ieee80211DataOrMgmtFrame *>(acData[ac].transmissionQueue.front());
        acData[ac].transmissionQueue.pop_front();
        startSendDataFrameExchange(frame, ac);
    }
}

void BasicUpperMac::sendAck(Ieee80211DataOrMgmtFrame *frame)
{
    Ieee80211ACKFrame *ackFrame = context->buildAckFrame(frame);
    context->transmitImmediateFrame(ackFrame, context->getSifsTime(), nullptr);
}

void BasicUpperMac::sendCts(Ieee80211RTSFrame *frame)
{
    Ieee80211CTSFrame *ctsFrame = context->buildCtsFrame(frame);
    context->transmitImmediateFrame(ctsFrame, context->getSifsTime(), nullptr);
}

} // namespace ieee80211
} // namespace inet


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

#include "EdcaUpperMac.h"
#include "Ieee80211NewMac.h"
#include "IRx.h"
#include "IContentionTx.h"
#include "IImmediateTx.h"
#include "MacUtils.h"
#include "MacParameters.h"
#include "FrameExchanges.h"
#include "DuplicateDetectors.h"
#include "Fragmentation.h"
#include "inet/common/queue/IPassiveQueue.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"

namespace inet {
namespace ieee80211 {

Define_Module(EdcaUpperMac);

inline std::string suffix(const char *s, int i) {std::stringstream ss; ss << s << i; return ss.str();}

EdcaUpperMac::EdcaUpperMac()
{
}

EdcaUpperMac::~EdcaUpperMac()
{
    delete duplicateDetection;
    delete fragmenter;
    delete reassembly;

    int numACs = params->isEdcaEnabled() ? 4 : 1;
    for (int i = 0; i < numACs; i++) {
        delete acData[i].frameExchange;
    }
    delete[] acData;
}

void EdcaUpperMac::initialize()
{
    mac = check_and_cast<Ieee80211NewMac *>(getModuleByPath(par("macModule")));
    rx = check_and_cast<IRx *>(getModuleByPath(par("rxModule")));
    immediateTx = check_and_cast<IImmediateTx *>(getModuleByPath(par("immediateTxModule")));
    contentionTx = nullptr;
    collectContentionTxModules(getModuleByPath(par("firstContentionTxModule")), contentionTx);

    maxQueueSize = par("maxQueueSize");

    readParameters();
    utils = new MacUtils(params);
    rx->setAddress(params->getAddress());

    int numACs = params->isEdcaEnabled() ? 4 : 1;
    acData = new AccessCategoryData[numACs];
    CompareFunc compareFunc = par("prioritizeMulticast") ? (CompareFunc)MacUtils::cmpMgmtOverMulticastOverUnicast : (CompareFunc)MacUtils::cmpMgmtOverData;
    for (int i = 0; i < numACs; i++) {
        acData[i].transmissionQueue.setName(suffix("txQueue-", i).c_str());
        acData[i].transmissionQueue.setup(compareFunc);
    }

    duplicateDetection = new QoSDuplicateDetector();
    fragmenter = new FragmentationNotSupported();
    reassembly = new ReassemblyNotSupported();

    WATCH(maxQueueSize);
    WATCH(fragmentationThreshold);
}

inline double fallback(double a, double b) {return a!=-1 ? a : b;}
inline simtime_t fallback(simtime_t a, simtime_t b) {return a!=-1 ? a : b;}

void EdcaUpperMac::readParameters()
{
    MacParameters *params = new MacParameters();

    const Ieee80211ModeSet *modeSet = Ieee80211ModeSet::getModeSet(*par("opMode").stringValue());
    double bitrate = par("bitrate");
    const IIeee80211Mode *dataFrameMode = (bitrate == -1) ? modeSet->getFastestMode() : modeSet->getMode(bps(bitrate));
    const IIeee80211Mode *basicFrameMode = modeSet->getSlowestMode();

    params->setAddress(mac->getAddress());
    params->setBasicFrameMode(basicFrameMode);
    params->setDefaultDataFrameMode(dataFrameMode);
    params->setShortRetryLimit(fallback(par("shortRetryLimit"), 7));
    params->setRtsThreshold(par("rtsThreshold"));

    params->setEdcaEnabled(true);
    params->setSlotTime(fallback(par("slotTime"), dataFrameMode->getSlotTime()));
    params->setSifsTime(fallback(par("sifsTime"), dataFrameMode->getSifsTime()));
    for (int i = 0; i < 4; i++) {
        AccessCategory ac = (AccessCategory)i;
        int aifsn = fallback(par(suffix("aifsn",i).c_str()), dataFrameMode->getAifsNumber(ac));
        params->setAifsTime(ac, params->getSifsTime() + aifsn*params->getSlotTime());
        params->setEifsTime(ac, params->getSifsTime() + params->getAifsTime(ac) + basicFrameMode->getDuration(LENGTH_ACK));
        params->setCwMin(ac, fallback(par(suffix("cwMin",i).c_str()), dataFrameMode->getCwMin(ac)));
        params->setCwMax(ac, fallback(par(suffix("cwMax",i).c_str()), dataFrameMode->getCwMax(ac)));
        params->setCwMulticast(ac, fallback(par(suffix("cwMulticast",i).c_str()), dataFrameMode->getCwMin(ac)));
    }

    this->params = params;
}

void EdcaUpperMac::handleMessage(cMessage *msg)
{
    if (msg->getContextPointer() != nullptr)
        ((MacPlugin *)msg->getContextPointer())->handleSelfMessage(msg);
    else
        ASSERT(false);
}

void EdcaUpperMac::upperFrameReceived(Ieee80211DataOrMgmtFrame *frame)
{
    Enter_Method("upperFrameReceived(\"%s\")", frame->getName());
    take(frame);

    AccessCategory ac = classifyFrame(frame);

    EV_INFO << "Frame " << frame << " received from higher layer, receiver = " << frame->getReceiverAddress() << endl;

    if (maxQueueSize > 0 && acData[ac].transmissionQueue.length() >= maxQueueSize) {
        EV << "Frame " << frame << " received from higher layer, but its MAC subqueue is full, dropping\n";
        delete frame;
        return;
    }

    ASSERT(!frame->getReceiverAddress().isUnspecified());
    frame->setTransmitterAddress(params->getAddress());
    duplicateDetection->assignSequenceNumber(frame);

    if (frame->getByteLength() <= fragmentationThreshold)
        enqueue(frame, ac);
    else {
        auto fragments = fragmenter->fragment(frame);
        for (Ieee80211DataOrMgmtFrame *fragment : fragments)
            enqueue(fragment, ac);
    }
}

void EdcaUpperMac::enqueue(Ieee80211DataOrMgmtFrame *frame, AccessCategory ac)
{
    if (acData[ac].frameExchange)
        acData[ac].transmissionQueue.insert(frame);
    else {
        int txIndex = (int)ac;  //one-to-one mapping
        startSendDataFrameExchange(frame, txIndex, ac);
    }
}

AccessCategory EdcaUpperMac::classifyFrame(Ieee80211DataOrMgmtFrame *frame)
{
    if (frame->getType() == ST_DATA) {
        return AC_BE;  // non-QoS frames are Best Effort
    }
    else if (frame->getType() == ST_DATA_WITH_QOS) {
        Ieee80211DataFrame *dataFrame = check_and_cast<Ieee80211DataFrame*>(frame);
        return mapTidToAc(dataFrame->getTid());  // QoS frames: map TID to AC
    }
    else {
        return AC_VO; // management frames travel in the Voice category
    }
}

AccessCategory EdcaUpperMac::mapTidToAc(int tid)
{
    // standard static mapping (see "UP-to-AC mappings" table in the 802.11 spec.)
    switch (tid) {
        case 1: case 2: return AC_BK;
        case 0: case 3: return AC_BE;
        case 4: case 5: return AC_VI;
        case 6: case 7: return AC_VO;
        default: throw cRuntimeError("No mapping from TID=%d to AccessCategory (must be in the range 0..7)", tid);
    }
}

void EdcaUpperMac::lowerFrameReceived(Ieee80211Frame *frame)
{
    Enter_Method("lowerFrameReceived(\"%s\")", frame->getName());
    delete frame->removeControlInfo();
    take(frame);

    if (utils->isForUs(frame)) {
        if (Ieee80211RTSFrame *rtsFrame = dynamic_cast<Ieee80211RTSFrame *>(frame)) {
            sendCts(rtsFrame);
            delete rtsFrame;
        }
        else if (Ieee80211DataOrMgmtFrame *dataOrMgmtFrame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(frame)) {
            if (!utils->isBroadcast(frame) && !utils->isMulticast(frame))
                sendAck(dataOrMgmtFrame);
            if (duplicateDetection->isDuplicate(dataOrMgmtFrame)) {
                EV_INFO << "Duplicate frame " << frame->getName() << ", dropping\n";
                delete dataOrMgmtFrame;
            }
            else {
                if (!utils->isFragment(dataOrMgmtFrame))
                    mac->sendUp(dataOrMgmtFrame);
                else {
                    Ieee80211DataOrMgmtFrame *completeFrame = reassembly->addFragment(dataOrMgmtFrame);
                    if (completeFrame)
                        mac->sendUp(completeFrame);
                }
            }
        }
        else {
            // offer frame to all ongoing frame exchanges
            int numACs = params->isEdcaEnabled() ? 4 : 1;
            bool processed = false;
            for (int i = 0; i < numACs && !processed; i++)
                if (acData[i].frameExchange)
                    processed = acData[i].frameExchange->lowerFrameReceived(frame);
            if (!processed) {
                EV_INFO << "Unexpected frame " << frame->getName() << ", dropping\n";
                delete frame;
            }
        }
    }
    else {
        EV_INFO << "This frame is not for us" << std::endl;
        delete frame;
    }
}

void EdcaUpperMac::transmissionComplete(ITxCallback *callback, int txIndex)
{
    Enter_Method("transmissionComplete()");
    if (callback)
        callback->transmissionComplete(txIndex);
}

void EdcaUpperMac::internalCollision(ITxCallback *callback, int txIndex)
{
    Enter_Method("transmissionComplete()");
    if (callback)
        callback->internalCollision(txIndex);
}

void EdcaUpperMac::startSendDataFrameExchange(Ieee80211DataOrMgmtFrame *frame, int txIndex, AccessCategory ac)
{
    ASSERT(!acData[ac].frameExchange);

    if (utils->isBroadcast(frame) || utils->isMulticast(frame))
        utils->setFrameMode(frame, params->getBasicFrameMode());
    else
        utils->setFrameMode(frame, params->getDefaultDataFrameMode());

    FrameExchangeContext context;
    context.ownerModule = this;
    context.params = params;
    context.utils = utils;
    context.contentionTx = contentionTx;
    context.immediateTx = immediateTx;

    IFrameExchange *frameExchange;
    bool useRtsCts = frame->getByteLength() > params->getRtsThreshold();
    if (utils->isBroadcast(frame) || utils->isMulticast(frame))
        frameExchange = new SendMulticastDataFrameExchange(&context, this, frame, txIndex, ac);
    else if (useRtsCts)
        frameExchange = new SendDataWithRtsCtsFrameExchange(&context, this, frame, txIndex, ac);
    else
        frameExchange = new SendDataWithAckFrameExchange(&context, this, frame, txIndex, ac);

    frameExchange->start();
    acData[ac].frameExchange = frameExchange;
}

void EdcaUpperMac::frameExchangeFinished(IFrameExchange *what, bool successful)
{
    EV_INFO << "Frame exchange finished" << std::endl;

    // find ac for this frame exchange
    int numACs = params->isEdcaEnabled() ? 4 : 1;
    AccessCategory ac = (AccessCategory) -1;
    for (int i = 0; i < numACs; i++)
        if (acData[i].frameExchange == what)
            ac = (AccessCategory)i;
    ASSERT(ac != -1);

    delete acData[ac].frameExchange;
    acData[ac].frameExchange = nullptr;

    if (!acData[ac].transmissionQueue.empty()) {
        Ieee80211DataOrMgmtFrame *frame = check_and_cast<Ieee80211DataOrMgmtFrame *>(acData[ac].transmissionQueue.pop());
        int txIndex = (int)ac;  //one-to-one mapping
        startSendDataFrameExchange(frame, txIndex, ac);
    }
}

void EdcaUpperMac::sendAck(Ieee80211DataOrMgmtFrame *frame)
{
    Ieee80211ACKFrame *ackFrame = utils->buildAckFrame(frame);
    immediateTx->transmitImmediateFrame(ackFrame, params->getSifsTime(), nullptr);
}

void EdcaUpperMac::sendCts(Ieee80211RTSFrame *frame)
{
    Ieee80211CTSFrame *ctsFrame = utils->buildCtsFrame(frame);
    immediateTx->transmitImmediateFrame(ctsFrame, params->getSifsTime(), nullptr);
}

} // namespace ieee80211
} // namespace inet


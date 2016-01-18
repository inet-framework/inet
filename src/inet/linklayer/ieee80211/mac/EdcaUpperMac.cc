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
#include "Ieee80211Mac.h"
#include "IRx.h"
#include "IContention.h"
#include "ITx.h"
#include "MacUtils.h"
#include "MacParameters.h"
#include "FrameExchanges.h"
#include "DuplicateDetectors.h"
#include "IFragmentation.h"
#include "IRateSelection.h"
#include "IRateControl.h"
#include "IStatistics.h"
#include "inet/common/INETUtils.h"
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
    delete utils;
    delete [] contention;

    int numACs = params->isEdcaEnabled() ? 4 : 1;
    for (int i = 0; i < numACs; i++) {
        delete acData[i].frameExchange;
    }
    delete params;
    delete[] acData;
}

void EdcaUpperMac::initialize()
{
    mac = check_and_cast<Ieee80211Mac *>(getModuleByPath(par("macModule")));
    rx = check_and_cast<IRx *>(getModuleByPath(par("rxModule")));
    tx = check_and_cast<ITx *>(getModuleByPath(par("txModule")));
    contention = nullptr;
    collectContentionModules(getModuleByPath(par("firstContentionModule")), contention);

    maxQueueSize = par("maxQueueSize");

    rateSelection = check_and_cast<IRateSelection*>(getModuleByPath(par("rateSelectionModule")));
    rateControl = dynamic_cast<IRateControl*>(getModuleByPath(par("rateControlModule"))); // optional module
    rateSelection->setRateControl(rateControl);

    params = extractParameters(rateSelection->getSlowestMandatoryMode());
    utils = new MacUtils(params, rateSelection);
    rx->setAddress(params->getAddress());

    int numACs = params->isEdcaEnabled() ? 4 : 1;
    acData = new AccessCategoryData[numACs];
    CompareFunc compareFunc = par("prioritizeMulticast") ? (CompareFunc)MacUtils::cmpMgmtOverMulticastOverUnicast : (CompareFunc)MacUtils::cmpMgmtOverData;
    for (int i = 0; i < numACs; i++) {
        acData[i].transmissionQueue.setName(suffix("txQueue-", i).c_str());
        acData[i].transmissionQueue.setup(compareFunc);
    }

    statistics = check_and_cast<IStatistics*>(getModuleByPath(par("statisticsModule")));
    statistics->setMacUtils(utils);
    statistics->setRateControl(rateControl);

    duplicateDetection = new QoSDuplicateDetector();
    fragmenter = check_and_cast<IFragmenter *>(inet::utils::createOne(par("fragmenterClass")));
    reassembly = check_and_cast<IReassembly *>(inet::utils::createOne(par("reassemblyClass")));

    WATCH(maxQueueSize);
    WATCH(fragmentationThreshold);
}

inline double fallback(double a, double b) {return a!=-1 ? a : b;}
inline simtime_t fallback(simtime_t a, simtime_t b) {return a!=-1 ? a : b;}

IMacParameters *EdcaUpperMac::extractParameters(const IIeee80211Mode *slowestMandatoryMode)
{
    const IIeee80211Mode *referenceMode = slowestMandatoryMode;  // or any other; slotTime etc must be the same for all modes we use

    MacParameters *params = new MacParameters();
    params->setAddress(mac->getAddress());
    params->setShortRetryLimit(fallback(par("shortRetryLimit"), 7));
    params->setLongRetryLimit(fallback(par("longRetryLimit"), 4));
    params->setRtsThreshold(par("rtsThreshold"));
    params->setPhyRxStartDelay(referenceMode->getPhyRxStartDelay());
    params->setUseFullAckTimeout(par("useFullAckTimeout"));
    params->setEdcaEnabled(true);
    params->setSlotTime(fallback(par("slotTime"), referenceMode->getSlotTime()));
    params->setSifsTime(fallback(par("sifsTime"), referenceMode->getSifsTime()));
    int aCwMin = referenceMode->getLegacyCwMin();
    int aCwMax = referenceMode->getLegacyCwMax();

    for (int i = 0; i < 4; i++) {
        AccessCategory ac = (AccessCategory)i;
        int aifsn = fallback(par(suffix("aifsn",i).c_str()), MacUtils::getAifsNumber(ac));
        params->setAifsTime(ac, params->getSifsTime() + aifsn*params->getSlotTime());
        params->setEifsTime(ac, params->getSifsTime() + params->getAifsTime(ac) + slowestMandatoryMode->getDuration(LENGTH_ACK));
        params->setCwMin(ac, fallback(par(suffix("cwMin",i).c_str()), MacUtils::getCwMin(ac, aCwMin)));
        params->setCwMax(ac, fallback(par(suffix("cwMax",i).c_str()), MacUtils::getCwMax(ac, aCwMax, aCwMin)));
        params->setCwMulticast(ac, fallback(par(suffix("cwMulticast",i).c_str()), MacUtils::getCwMin(ac, aCwMin)));
    }
    return params;
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

    if (maxQueueSize > 0 && acData[ac].transmissionQueue.length() >= maxQueueSize && dynamic_cast<Ieee80211DataFrame *>(frame)) {
        EV << "Dataframe " << frame << " received from higher layer, but its MAC subqueue is full, dropping\n";
        delete frame;
        return;
    }

    ASSERT(!frame->getReceiverAddress().isUnspecified());
    frame->setTransmitterAddress(params->getAddress());
    duplicateDetection->assignSequenceNumber(frame);

    if (frame->getByteLength() <= fragmentationThreshold)
        enqueue(frame, ac);
    else {
        auto fragments = fragmenter->fragment(frame, fragmentationThreshold);
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

    if (!utils->isForUs(frame)) {
        EV_INFO << "This frame is not for us" << std::endl;
        delete frame;
        int numACs = params->isEdcaEnabled() ? 4 : 1;
        for (int i = 0; i < numACs; i++)
            if (acData[i].frameExchange)
                acData[i].frameExchange->corruptedOrNotForUsFrameReceived();
    }
    else {
        // show frame to ALL ongoing frame exchanges
        int numACs = params->isEdcaEnabled() ? 4 : 1;
        bool alreadyProcessed = false;
        bool shouldDelete = false;
        for (int i = 0; i < numACs; i++) {
            if (acData[i].frameExchange) {
                IFrameExchange::FrameProcessingResult result = acData[i].frameExchange->lowerFrameReceived(frame);
                bool justProcessed = (result != IFrameExchange::IGNORED);
                ASSERT(!alreadyProcessed || !justProcessed); // ensure it's not double-processed
                if (justProcessed) {
                    alreadyProcessed = true;
                    shouldDelete = (result == IFrameExchange::PROCESSED_DISCARD);
                }
            }
        }

        if (alreadyProcessed) {
            // jolly good, nothing more to do
            if (shouldDelete)
                delete frame;
        }
        else if (Ieee80211RTSFrame *rtsFrame = dynamic_cast<Ieee80211RTSFrame *>(frame)) {
            sendCts(rtsFrame);
            delete rtsFrame;
        }
        else if (Ieee80211DataOrMgmtFrame *dataOrMgmtFrame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(frame)) {
            if (!utils->isBroadcastOrMulticast(frame))
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
            EV_INFO << "Unexpected frame " << frame->getName() << ", dropping\n";
            delete frame;
        }
    }
}

void EdcaUpperMac::corruptedFrameReceived()
{
    int numACs = params->isEdcaEnabled() ? 4 : 1;
    for (int i = 0; i < numACs; i++)
        if (acData[i].frameExchange)
            acData[i].frameExchange->corruptedOrNotForUsFrameReceived();
}

void EdcaUpperMac::channelAccessGranted(IContentionCallback *callback, int txIndex)
{
    Enter_Method("channelAccessGranted()");
    callback->channelAccessGranted(txIndex);
}

void EdcaUpperMac::internalCollision(IContentionCallback *callback, int txIndex)
{
    Enter_Method("internalCollision()");
    if (callback)
        callback->internalCollision(txIndex);
}

void EdcaUpperMac::transmissionComplete(ITxCallback *callback)
{
    Enter_Method("transmissionComplete()");
    if (callback)
        callback->transmissionComplete();
}

void EdcaUpperMac::startSendDataFrameExchange(Ieee80211DataOrMgmtFrame *frame, int txIndex, AccessCategory ac)
{
    ASSERT(!acData[ac].frameExchange);

    if (utils->isBroadcastOrMulticast(frame))
        utils->setFrameMode(frame, rateSelection->getModeForMulticastDataOrMgmtFrame(frame));
    else
        utils->setFrameMode(frame, rateSelection->getModeForUnicastDataOrMgmtFrame(frame));

    FrameExchangeContext context;
    context.ownerModule = this;
    context.params = params;
    context.utils = utils;
    context.contention = contention;
    context.tx = tx;
    context.rx = rx;
    context.statistics = statistics;

    IFrameExchange *frameExchange;
    bool useRtsCts = frame->getByteLength() > params->getRtsThreshold();
    if (utils->isBroadcastOrMulticast(frame))
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
    ASSERT((int)ac != -1);

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
    tx->transmitFrame(ackFrame, params->getSifsTime(), nullptr);
}

void EdcaUpperMac::sendCts(Ieee80211RTSFrame *frame)
{
    Ieee80211CTSFrame *ctsFrame = utils->buildCtsFrame(frame);
    tx->transmitFrame(ctsFrame, params->getSifsTime(), nullptr);
}

} // namespace ieee80211
} // namespace inet


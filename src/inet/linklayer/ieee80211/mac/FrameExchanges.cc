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

#include "FrameExchanges.h"
#include "inet/common/INETUtils.h"
#include "inet/common/FSMA.h"
#include "IContention.h"
#include "ITx.h"
#include "IRx.h"
#include "IMacParameters.h"
#include "IStatistics.h"
#include "MacUtils.h"
#include "Ieee80211Frame_m.h"

using namespace inet::utils;

namespace inet {
namespace ieee80211 {

#if 0
//TODO with the introduction of earlyAckTimeout, this class has become obsolete
SendDataWithAckFsmBasedFrameExchange::SendDataWithAckFsmBasedFrameExchange(FrameExchangeContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *frame, int txIndex, AccessCategory accessCategory) :
    FsmBasedFrameExchange(context, callback), frame(frame), txIndex(txIndex), accessCategory(accessCategory)
{
    frame->setDuration(params->getSifsTime() + utils->getAckDuration());
}

SendDataWithAckFsmBasedFrameExchange::~SendDataWithAckFsmBasedFrameExchange()
{
    delete frame;
    if (ackTimer)
        delete cancelEvent(ackTimer);
}

IFrameExchange::FrameProcessingResult SendDataWithAckFsmBasedFrameExchange::handleWithFSM(EventType event, cMessage *frameOrTimer)
{
    FrameProcessingResult result = IGNORED;
    Ieee80211Frame *receivedFrame = event == EVENT_FRAMEARRIVED ? check_and_cast<Ieee80211Frame *>(frameOrTimer) : nullptr;

    FSMA_Switch(fsm) {
        FSMA_State(INIT) {
            FSMA_Enter();
            FSMA_Event_Transition(Start,
                    event == EVENT_START,
                    TRANSMITDATA,
                    transmitDataFrame();
                    );
        }
        FSMA_State(TRANSMITDATA) {
            FSMA_Enter();
            FSMA_Event_Transition(Wait - ACK,
                    event == EVENT_TXFINISHED,
                    WAITACK,
                    scheduleAckTimeout();
                    );
            FSMA_Event_Transition(Frame - arrived,
                    event == EVENT_FRAMEARRIVED,
                    TRANSMITDATA,
                    ;
                    );
        }
        FSMA_State(WAITACK) {
            FSMA_Enter();
            FSMA_Event_Transition(Ack - arrived,
                    event == EVENT_FRAMEARRIVED && isAck(receivedFrame),
                    SUCCESS,
                    result = PROCESSED_DISCARD;
                    );
            FSMA_Event_Transition(Frame - arrived,
                    event == EVENT_FRAMEARRIVED && !isAck(receivedFrame),
                    FAILURE,
                    ;
                    );
            FSMA_Event_Transition(Ack - timeout - retry,
                    event == EVENT_TIMER && retryCount < params->getShortRetryLimit(),
                    TRANSMITDATA,
                    retryDataFrame();
                    );
            FSMA_Event_Transition(Ack - timeout - giveup,
                    event == EVENT_TIMER && retryCount == params->getShortRetryLimit(),
                    FAILURE,
                    ;
                    );
        }
        FSMA_State(SUCCESS) {
            FSMA_Enter(reportSuccess());
        }
        FSMA_State(FAILURE) {
            FSMA_Enter(reportFailure());
        }
    }
    return result;
}

void SendDataWithAckFsmBasedFrameExchange::transmitDataFrame()
{
    retryCount = 0;
    AccessCategory ac = accessCategory; // abbreviate
    contention[txIndex]->transmitContentionFrame(dupPacketAndControlInfo(frame), params->getAifsTime(ac), params->getEifsTime(ac), params->getCwMin(ac), params->getCwMax(ac), params->getSlotTime(), retryCount, this);
}

void SendDataWithAckFsmBasedFrameExchange::retryDataFrame()
{
    retryCount++;
    frame->setRetry(true);
    AccessCategory ac = accessCategory; // abbreviate
    contention[txIndex]->transmitContentionFrame(dupPacketAndControlInfo(frame), params->getAifsTime(ac), params->getEifsTime(ac), params->getCwMin(ac), params->getCwMax(ac), params->getSlotTime(), retryCount, this);
}

void SendDataWithAckFsmBasedFrameExchange::scheduleAckTimeout()
{
    if (ackTimer == nullptr)
        ackTimer = new cMessage("timeout");
    simtime_t t = simTime() + utils->getAckFullTimeout();
    scheduleAt(t, ackTimer);
}

bool SendDataWithAckFsmBasedFrameExchange::isAck(Ieee80211Frame *frame)
{
    return dynamic_cast<Ieee80211ACKFrame *>(frame) != nullptr;
}
#endif

//------------------------------

SendDataWithAckFrameExchange::SendDataWithAckFrameExchange(FrameExchangeContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *dataFrame, int txIndex, AccessCategory accessCategory) :
    StepBasedFrameExchange(context, callback, txIndex, accessCategory), dataFrame(dataFrame)
{
    dataFrame->setDuration(params->getSifsTime() + utils->getAckDuration());
}

SendDataWithAckFrameExchange::~SendDataWithAckFrameExchange()
{
    delete dataFrame;
}

std::string SendDataWithAckFrameExchange::info() const
{
    std::string ret = StepBasedFrameExchange::info();
    if (dataFrame) {
        ret += ", frame=";
        ret += dataFrame->getName();
    }
    return ret;
}

void SendDataWithAckFrameExchange::doStep(int step)
{
    switch (step) {
        case 0: startContentionIfNeeded(retryCount); break;
        case 1: transmitFrame(dupPacketAndControlInfo(dataFrame)); break;
        case 2: {
            if (params->getUseFullAckTimeout())
                expectFullReplyWithin(utils->getAckFullTimeout());
            else
                expectReplyRxStartWithin(utils->getAckEarlyTimeout());
            break;
        }
        case 3: statistics->frameTransmissionSuccessful(dataFrame, retryCount); releaseChannel(); succeed(); break;
        default: ASSERT(false);
    }
}

IFrameExchange::FrameProcessingResult SendDataWithAckFrameExchange::processReply(int step, Ieee80211Frame *frame)
{
    switch (step) {
        case 2: if (utils->isAck(frame)) return PROCESSED_DISCARD; else return IGNORED;
        default: ASSERT(false); return IGNORED;
    }
}

void SendDataWithAckFrameExchange::processTimeout(int step)
{
    switch (step) {
        case 2: retry(); break;
        default: ASSERT(false);
    }
}

#ifdef NS3_VALIDATION
static const char *ac[] = {"AC_BK", "AC_BE", "AC_VI", "AC_VO", "???"};
#endif

void SendDataWithAckFrameExchange::processInternalCollision(int step)
{
#ifdef NS3_VALIDATION
    const char *lastSeq = strchr(dataFrame->getName(), '-');
    if (lastSeq == nullptr)
        lastSeq = "-1";
    else
        lastSeq++;
    std::cout << "IC: " << "ac = " << ac[defaultAccessCategory] << ", seq = " << lastSeq << endl;
#endif

    switch (step) {
        case 0: retry(); break;
        default: ASSERT(false);
    }
}

void SendDataWithAckFrameExchange::retry()
{
    releaseChannel();
    // 9.19.2.6 Retransmit procedures
    // Retries for failed transmission attempts shall continue until the short retry count for the MSDU, A-MSDU, or
    // MMPDU is equal to dot11ShortRetryLimit or until the long retry count for the MSDU, A-MSDU, or MMPDU
    // is equal to dot11LongRetryLimit.
    //
    // Annex C MIB
    // This attribute indicates the maximum number of transmission attempts of a
    // frame, the length of which is less than or equal to dot11RTSThreshold,
    // that is made before a failure condition is indicated.

#ifdef NS3_VALIDATION
    const char *lastSeq = strchr(dataFrame->getName(), '-');
    if (lastSeq == nullptr)
        lastSeq = "-1";
    else
        lastSeq++;
#endif

    if (retryCount + 1 < params->getShortRetryLimit()) {
        statistics->frameTransmissionUnsuccessful(dataFrame, retryCount);
        dataFrame->setRetry(true);
        retryCount++;
        gotoStep(0);

#ifdef NS3_VALIDATION
        std::cout << "RE: " << "ac = " << ac[defaultAccessCategory] << ", seq = " << lastSeq << ", num = " << retryCount << endl;
#endif
    }
    else {
        statistics->frameTransmissionUnsuccessfulGivingUp(dataFrame, retryCount);
        fail();

#ifdef NS3_VALIDATION
        if (*lastSeq == '0' && defaultAccessCategory == 1)
            std::cout << "BREAK\n";
        std::cout << "CA: " << "ac = " << ac[defaultAccessCategory] << ", seq = " << lastSeq << endl;
#endif
    }
}

//------------------------------

SendDataWithRtsCtsFrameExchange::SendDataWithRtsCtsFrameExchange(FrameExchangeContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *dataFrame, int txIndex, AccessCategory accessCategory) :
    StepBasedFrameExchange(context, callback, txIndex, accessCategory), dataFrame(dataFrame)
{
    dataFrame->setDuration(params->getSifsTime() + utils->getAckDuration());
}

SendDataWithRtsCtsFrameExchange::~SendDataWithRtsCtsFrameExchange()
{
    delete dataFrame;
}

std::string SendDataWithRtsCtsFrameExchange::info() const
{
    std::string ret = StepBasedFrameExchange::info();
    if (dataFrame) {
        ret += ", frame=";
        ret += dataFrame->getName();
    }
    return ret;
}

void SendDataWithRtsCtsFrameExchange::doStep(int step)
{
    switch (step) {
        case 0: startContentionIfNeeded(shortRetryCount); break;
        case 1: transmitFrame(utils->buildRtsFrame(dataFrame)); break;
        case 2: expectReplyRxStartWithin(utils->getCtsEarlyTimeout()); break;
        case 3: transmitFrame(dupPacketAndControlInfo(dataFrame), params->getSifsTime()); break;
        case 4: expectReplyRxStartWithin(utils->getAckEarlyTimeout()); break;
        case 5: statistics->frameTransmissionSuccessful(dataFrame, longRetryCount); releaseChannel(); succeed(); break;
        default: ASSERT(false);
    }
}

IFrameExchange::FrameProcessingResult SendDataWithRtsCtsFrameExchange::processReply(int step, Ieee80211Frame *frame)
{
    switch (step) {
        case 2: if (utils->isCts(frame)) return PROCESSED_DISCARD; else return IGNORED;
        case 4: if (utils->isAck(frame)) return PROCESSED_DISCARD; else return IGNORED;
        default: ASSERT(false); return IGNORED;
    }
}

void SendDataWithRtsCtsFrameExchange::processTimeout(int step)
{
    switch (step) {
        case 2: retryRtsCts(); break;
        case 4: retryData(); break;
        default: ASSERT(false);
    }
}

void SendDataWithRtsCtsFrameExchange::processInternalCollision(int step)
{
    switch (step) {
        case 0: retryRtsCts(); break;
        default: ASSERT(false);
    }
}

void SendDataWithRtsCtsFrameExchange::retryRtsCts()
{
    releaseChannel();
    if (shortRetryCount < params->getShortRetryLimit()) {
        shortRetryCount++;
        gotoStep(0);
    }
    else {
        statistics->frameTransmissionGivenUp(dataFrame);
        fail();
    }
}

void SendDataWithRtsCtsFrameExchange::retryData()
{
    releaseChannel();
    if (longRetryCount < params->getLongRetryLimit()) {
        statistics->frameTransmissionUnsuccessful(dataFrame, longRetryCount);
        dataFrame->setRetry(true);
        longRetryCount++;
        gotoStep(0);
    }
    else {
        statistics->frameTransmissionUnsuccessfulGivingUp(dataFrame, longRetryCount);
        fail();
    }
}

//------------------------------

SendMulticastDataFrameExchange::SendMulticastDataFrameExchange(FrameExchangeContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *dataFrame, int txIndex, AccessCategory accessCategory) :
    FrameExchange(context, callback), dataFrame(dataFrame), txIndex(txIndex), accessCategory(accessCategory)
{
    ASSERT(utils->isBroadcastOrMulticast(dataFrame));
    dataFrame->setDuration(0);
}

SendMulticastDataFrameExchange::~SendMulticastDataFrameExchange()
{
    delete dataFrame;
}

std::string SendMulticastDataFrameExchange::info() const
{
    return dataFrame ? std::string("frame=") + dataFrame->getName() : "";
}

void SendMulticastDataFrameExchange::handleSelfMessage(cMessage *msg)
{
    ASSERT(false);
}

void SendMulticastDataFrameExchange::start()
{
    startContention();
}

void SendMulticastDataFrameExchange::startContention()
{
    AccessCategory ac = accessCategory;  // abbreviate
    contention[txIndex]->startContention(params->getAifsTime(ac), params->getEifsTime(ac), params->getCwMulticast(ac), params->getCwMulticast(ac), params->getSlotTime(), 0, this);
}

void SendMulticastDataFrameExchange::internalCollision(int txIndex)
{
    if (++retryCount < params->getShortRetryLimit()) {
        dataFrame->setRetry(true);
        startContention();
    }
    else {
        reportFailure();
    }
}

void SendMulticastDataFrameExchange::channelAccessGranted(int txIndex)
{
    tx->transmitFrame(dupPacketAndControlInfo(dataFrame), SIMTIME_ZERO, this);
}

void SendMulticastDataFrameExchange::transmissionComplete()
{
    releaseChannel(txIndex);
    reportSuccess();
}


} // namespace ieee80211
} // namespace inet


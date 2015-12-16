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

#ifndef __INET_FRAMEEXCHANGE_H
#define __INET_FRAMEEXCHANGE_H

#include "IFrameExchange.h"
#include "MacPlugin.h"
#include "ITxCallback.h"
#include "AccessCategory.h"

namespace inet {
namespace ieee80211 {

class IMacParameters;
class MacUtils;
class ITx;
class IContention;
class IRx;
class IStatistics;

/**
 * A collection of information needed by frame exchanges (FrameExchange class) to operate.
 */
class INET_API FrameExchangeContext
{
    public:
        cSimpleModule *ownerModule = nullptr;
        IMacParameters *params = nullptr;
        MacUtils *utils = nullptr;
        ITx *tx = nullptr;
        IContention **contention = nullptr;
        IRx *rx = nullptr;
        IStatistics *statistics = nullptr;
};

/**
 * The default base class for implementing frame exchanges (see IFrameExchange).
 */
class INET_API FrameExchange : public MacPlugin, public IFrameExchange, public ITxCallback, public IContentionCallback
{
    protected:
        IMacParameters *params;
        MacUtils *utils;
        ITx *tx;
        IContention **contention;
        IRx *rx;
        IStatistics *statistics;
        IFinishedCallback *finishedCallback = nullptr;

    protected:
        virtual void startContention(int txIndex, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, simtime_t slotTime, int retryCount);
        virtual void transmitFrame(Ieee80211Frame *frame, simtime_t ifs);
        virtual void releaseChannel(int txIndex);
        virtual void reportSuccess();
        virtual void reportFailure();

        virtual FrameProcessingResult lowerFrameReceived(Ieee80211Frame *frame) override;
        virtual void corruptedOrNotForUsFrameReceived() override;
        virtual void channelAccessGranted(int txIndex) override;

    public:
        FrameExchange(FrameExchangeContext *context, IFinishedCallback *callback);
        virtual ~FrameExchange();
};

class INET_API FsmBasedFrameExchange : public FrameExchange
{
    protected:
        cFSM fsm;
        enum EventType { EVENT_START, EVENT_FRAMEARRIVED, EVENT_CORRUPTEDFRAMEARRIVED, EVENT_TXFINISHED, EVENT_INTERNALCOLLISION, EVENT_TIMER };

    protected:
        virtual FrameProcessingResult handleWithFSM(EventType eventType, cMessage *frameOrTimer=nullptr) = 0;  // return value: for EVENT_FRAMEARRIVED: whether frame was processed; unused otherwise

    public:
        FsmBasedFrameExchange(FrameExchangeContext *context, IFinishedCallback *callback) : FrameExchange(context, callback) { fsm.setName("Frame Exchange FSM"); }
        virtual void start() override;
        virtual FrameProcessingResult lowerFrameReceived(Ieee80211Frame* frame) override;
        virtual void corruptedOrNotForUsFrameReceived() override;
        virtual void transmissionComplete() override;
        virtual void internalCollision(int txIndex) override;
        virtual void handleSelfMessage(cMessage* timer) override;
};

class INET_API StepBasedFrameExchange : public FrameExchange
{
    protected:
        enum Operation { NONE, START_CONTENTION, TRANSMIT_FRAME, EXPECT_FULL_REPLY, EXPECT_REPLY_RXSTART, GOTO_STEP, FAIL, SUCCEED };
        enum Status { INPROGRESS, SUCCEEDED, FAILED };
        int defaultTxIndex;
        AccessCategory defaultAccessCategory;
        int step = 0;
        Operation operation = NONE;
        Status status = INPROGRESS;
        cMessage *timeoutMsg = nullptr;
        int gotoTarget = -1;

    protected:
        // to be redefined by user
        virtual void doStep(int step) = 0;
        virtual FrameProcessingResult processReply(int step, Ieee80211Frame *frame) = 0; // true = frame accepted as reply and processing will continue on next step
        virtual void processTimeout(int step) = 0;
        virtual void processInternalCollision(int step) = 0;

        // operations that can be called from doStep()
        virtual void startContentionIfNeeded(int retryCount);
        virtual void startContention(int retryCount); // may invoke processInternalCollision()
        virtual void startContention(int retryCount, int txIndex, AccessCategory accessCategory);
        virtual void startContentionForMulticast(); // may invoke processInternalCollision()
        virtual void startContentionForMulticast(int txIndex, AccessCategory accessCategory);
        virtual void startContention(int txIndex, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, simtime_t slotTime, int retryCount) override;
        virtual void transmitFrame(Ieee80211Frame *frame);
        virtual void transmitFrame(Ieee80211Frame *frame, simtime_t ifs) override;
        virtual void expectFullReplyWithin(simtime_t timeout);  // may invoke processReply() and processTimeout()
        virtual void expectReplyRxStartWithin(simtime_t timeout); // may invoke processReply() and processTimeout()
        virtual void gotoStep(int step); // ~setNextStep()
        virtual void releaseChannel();
        virtual void fail();
        virtual void succeed();
        using FrameExchange::releaseChannel;

        // internal
        virtual void proceed();
        virtual void cleanup();
        virtual void setOperation(Operation type);
        virtual void logStatus(const char *what);
        virtual void checkOperation(Operation stepType, const char *where);
        virtual void handleTimeout();
        virtual void cleanupAndReportResult();
        static const char *statusName(Status status);
        static const char *operationName(Operation operation);
        static const char *operationFunctionName(Operation operation);

    public:
        StepBasedFrameExchange(FrameExchangeContext *context, IFinishedCallback *callback, int txIndex, AccessCategory accessCategory);
        virtual ~StepBasedFrameExchange();
        std::string info() const override;
        virtual void start() override;
        virtual FrameProcessingResult lowerFrameReceived(Ieee80211Frame *frame) override;
        virtual void corruptedOrNotForUsFrameReceived() override;
        virtual void channelAccessGranted(int txIndex) override;
        virtual void internalCollision(int txIndex) override;
        virtual void transmissionComplete() override;
        virtual void handleSelfMessage(cMessage *timer) override;
};

} // namespace ieee80211
} // namespace inet

#endif


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
class IImmediateTx;
class IContentionTx;
class IRx;

class INET_API FrameExchangeContext
{
    public:
        cSimpleModule *ownerModule;
        IMacParameters *params;
        IImmediateTx *immediateTx;
        IContentionTx **contentionTx;
        IRx *rx;
        MacUtils *utils;
};

/**
 * The default base class for implementing frame exchanges (see IFrameExchange).
 */
class INET_API FrameExchange : public MacPlugin, public IFrameExchange, public ITxCallback
{
    protected:
        IMacParameters *params;
        MacUtils *utils;
        IImmediateTx *immediateTx;
        IContentionTx **contentionTx;
        IRx *rx;
        IFinishedCallback *finishedCallback = nullptr;

    protected:
        virtual void transmitContentionFrame(Ieee80211Frame *frame, int txIndex, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, simtime_t slotTime, int retryCount);
        virtual void startContention(int txIndex, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, simtime_t slotTime, int retryCount);
        virtual void transmitImmediateFrame(Ieee80211Frame *frame, simtime_t ifs);
        virtual void reportSuccess();
        virtual void reportFailure();

        virtual bool lowerFrameReceived(Ieee80211Frame *frame) override;
        virtual void corruptedFrameReceived() override;
        virtual Ieee80211DataOrMgmtFrame *getFrameToTransmit(int txIndex) override;

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
        virtual bool handleWithFSM(EventType eventType, cMessage *frameOrTimer=nullptr) = 0;  // return value: for EVENT_FRAMEARRIVED: whether frame was processed; unused otherwise

    public:
        FsmBasedFrameExchange(FrameExchangeContext *context, IFinishedCallback *callback) : FrameExchange(context, callback) { fsm.setName("Frame Exchange FSM"); }
        virtual void start() override;
        virtual bool lowerFrameReceived(Ieee80211Frame* frame) override;
        virtual void corruptedFrameReceived() override;
        virtual void transmissionComplete(int txIndex) override;
        virtual void internalCollision(int txIndex) override;
        virtual void handleSelfMessage(cMessage* timer) override;
};

class INET_API StepBasedFrameExchange : public FrameExchange
{
    private:
        enum Operation { NONE, TRANSMIT_CONTENTION_FRAME, TRANSMIT_IMMEDIATE_FRAME, EXPECT_FULL_REPLY, EXPECT_REPLY_RXSTART, GOTO_STEP, FAIL, SUCCEED };
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
        virtual bool processReply(int step, Ieee80211Frame *frame) = 0; // true = frame accepted as reply and processing will continue on next step
        virtual void processTimeout(int step) = 0;
        virtual void processInternalCollision(int step) = 0;

        // operations that can be called from doStep()
        virtual void transmitContentionFrame(Ieee80211Frame *frame, int retryCount); // may invoke processInternalCollision()
        virtual void transmitContentionFrame(Ieee80211Frame *frame, int retryCount, int txIndex, AccessCategory accessCategory);
        virtual void transmitMulticastContentionFrame(Ieee80211Frame *frame);
        virtual void transmitMulticastContentionFrame(Ieee80211Frame *frame, int txIndex, AccessCategory accessCategory);
        virtual void transmitContentionFrame(Ieee80211Frame *frame, int txIndex, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, simtime_t slotTime, int retryCount) override;
        virtual void transmitImmediateFrame(Ieee80211Frame *frame, simtime_t ifs) override;
        virtual void expectFullReplyWithin(simtime_t timeout);  // may invoke processReply() and processTimeout()
        virtual void expectReplyRxStartWithin(simtime_t timeout); // may invoke processReply() and processTimeout()
        virtual void gotoStep(int step); // ~setNextStep()
        virtual void fail();
        virtual void succeed();

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
        virtual bool lowerFrameReceived(Ieee80211Frame *frame) override; // true = frame processed
        virtual void corruptedFrameReceived() override;
        virtual void transmissionComplete(int txIndex) override;
        virtual void internalCollision(int txIndex) override;
        virtual void handleSelfMessage(cMessage *timer) override;
};

} // namespace ieee80211
} // namespace inet

#endif


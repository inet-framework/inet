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

namespace inet {
namespace ieee80211 {

class IUpperMacContext;

class FrameExchange : public MacPlugin, public IFrameExchange, public ITxCallback
{
    protected:
        IUpperMacContext *context = nullptr;
        IFinishedCallback *finishedCallback = nullptr;

    protected:
        virtual void reportSuccess();
        virtual void reportFailure();

    public:
        FrameExchange(cSimpleModule *ownerModule, IUpperMacContext *context, IFinishedCallback *callback) : MacPlugin(ownerModule), context(context), finishedCallback(callback) {}
        virtual ~FrameExchange() {}
};

class FsmBasedFrameExchange : public FrameExchange
{
    protected:
        cFSM fsm;
        enum EventType { EVENT_START, EVENT_FRAMEARRIVED, EVENT_TXFINISHED, EVENT_INTERNALCOLLISION, EVENT_TIMER };

    protected:
        virtual bool handleWithFSM(EventType eventType, cMessage *frameOrTimer) = 0;

    public:
        FsmBasedFrameExchange(cSimpleModule *ownerModule, IUpperMacContext *context, IFinishedCallback *callback) : FrameExchange(ownerModule, context, callback) { fsm.setName("Frame Exchange FSM"); }
        virtual void start() override;
        virtual bool lowerFrameReceived(Ieee80211Frame* frame) override;
        virtual void transmissionComplete(int txIndex) override;
        virtual void internalCollision(int txIndex) override;
        virtual void handleSelfMessage(cMessage* timer) override;
};

class StepBasedFrameExchange : public FrameExchange
{
    private:
        enum Operation { NONE, TRANSMIT_CONTENTION_FRAME, TRANSMIT_IMMEDIATE_FRAME, EXPECT_REPLY, GOTO_STEP, FAIL, SUCCEED };
        enum Status { INPROGRESS, SUCCEEDED, FAILED };
        int step = 0;
        Operation operation = NONE;
        Status status = INPROGRESS;
        cMessage *timeoutMsg = nullptr;
        int gotoTarget = -1;

    protected:
        // to be redefined by user
        virtual void doStep(int step) = 0;
        virtual bool processReply(int step, Ieee80211Frame *frame) = 0; // true = frame accepted as reply
        virtual void processTimeout(int step) = 0;
        virtual void processInternalCollision(int step) = 0;

        // operations that can be called from doStep()
        virtual void transmitContentionFrame(Ieee80211Frame *frame, int retryCount);
        virtual void transmitContentionFrame(Ieee80211Frame *frame, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, simtime_t slotTime, int retryCount);
        virtual void transmitImmediateFrame(Ieee80211Frame *frame, simtime_t ifs);
        virtual void expectReply(simtime_t timeout);
        virtual void gotoStep(int step); // ~setNextStep()
        virtual void fail();
        virtual void succeed();

        // internal
        virtual void proceed();
        virtual void cleanup();
        virtual void setOperation(Operation type);
        virtual void logStatus(const char *what);
        virtual void checkOperation(Operation stepType, const char *where);
        static const char *statusName(Status status);
        static const char *operationName(Operation operation);
        static const char *operationFunctionName(Operation operation);

    public:
        StepBasedFrameExchange(cSimpleModule *ownerModule, IUpperMacContext *context, IFinishedCallback *callback);
        virtual ~StepBasedFrameExchange();
        std::string info() const override;
        virtual void start();
        virtual bool lowerFrameReceived(Ieee80211Frame *frame); // true = frame processed
        virtual void transmissionComplete(int txIndex) override;
        virtual void internalCollision(int txIndex) override;
        virtual void handleSelfMessage(cMessage *timer) override;
};

} // namespace ieee80211
} // namespace inet

#endif


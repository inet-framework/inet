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

#ifndef __INET_FRAMEEXCHANGES_H
#define __INET_FRAMEEXCHANGES_H

#include "FrameExchange.h"

namespace inet {
namespace ieee80211 {

class Ieee80211DataOrMgmtFrame;

#if 0
// just to demonstrate the use FsmBasedFrameExchange; otherwise we prefer the step-based because it's simpler
class INET_API SendDataWithAckFsmBasedFrameExchange : public FsmBasedFrameExchange
{
    protected:
        Ieee80211DataOrMgmtFrame *frame;
        int txIndex;
        AccessCategory accessCategory;
        cMessage *ackTimer = nullptr;
        int retryCount = 0;

        enum State { INIT, TRANSMITDATA, WAITACK, SUCCESS, FAILURE };
        State state = INIT;

    protected:
        FrameProcessingResult handleWithFSM(EventType event, cMessage *frameOrTimer) override;

        void transmitDataFrame();
        void retryDataFrame();
        void scheduleAckTimeout();
        bool isAck(Ieee80211Frame *frame);

    public:
        SendDataWithAckFsmBasedFrameExchange(FrameExchangeContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *frame, int txIndex, AccessCategory accessCategory);
        ~SendDataWithAckFsmBasedFrameExchange();
};
#endif

class INET_API SendDataWithAckFrameExchange : public StepBasedFrameExchange
{
    protected:
        Ieee80211DataOrMgmtFrame *dataFrame = nullptr;
        int retryCount = 0;
    protected:
        virtual void retry();
        virtual void doStep(int step) override;
        virtual FrameProcessingResult processReply(int step, Ieee80211Frame *frame) override;
        virtual void processTimeout(int step) override;
        virtual void processInternalCollision(int step) override;
    public:
        SendDataWithAckFrameExchange(FrameExchangeContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *dataFrame, int txIndex, AccessCategory accessCategory);
        ~SendDataWithAckFrameExchange();
        virtual std::string info() const override;
};

class INET_API SendDataWithRtsCtsFrameExchange : public StepBasedFrameExchange
{
    protected:
        Ieee80211DataOrMgmtFrame *dataFrame = nullptr;
        int shortRetryCount = 0;
        int longRetryCount = 0;
    protected:
        virtual void doStep(int step) override;
        virtual FrameProcessingResult processReply(int step, Ieee80211Frame *frame) override;
        virtual void processTimeout(int step) override;
        virtual void processInternalCollision(int step) override;
        virtual void retryRtsCts();
        virtual void retryData();

    public:
        SendDataWithRtsCtsFrameExchange(FrameExchangeContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *dataFrame, int txIndex, AccessCategory accessCategory);
        ~SendDataWithRtsCtsFrameExchange();
        virtual std::string info() const override;
};

class INET_API SendMulticastDataFrameExchange : public FrameExchange
{
    protected:
        Ieee80211DataOrMgmtFrame *dataFrame;
        int txIndex;
        AccessCategory accessCategory;
        int retryCount = 0;
    protected:
        virtual void startContention();
    public:
        SendMulticastDataFrameExchange(FrameExchangeContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *dataFrame, int txIndex, AccessCategory accessCategory);
        ~SendMulticastDataFrameExchange();
        virtual void start() override;
        virtual void channelAccessGranted(int txIndex) override;
        virtual void internalCollision(int txIndex) override;
        virtual void transmissionComplete() override;
        virtual void handleSelfMessage(cMessage* timer) override;
        virtual std::string info() const override;
};

} // namespace ieee80211
} // namespace inet

#endif


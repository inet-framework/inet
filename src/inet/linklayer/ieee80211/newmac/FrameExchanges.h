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

// just to demonstrate the use FsmBasedFrameExchange; otherwise we prefer the step-based because it's simpler
class SendDataWithAckFsmBasedFrameExchange : public FsmBasedFrameExchange
{
    protected:
        Ieee80211DataOrMgmtFrame *frame;
        int retryCount = 0;
        cMessage *ackTimer = nullptr;

        enum State { INIT, TRANSMITDATA, WAITACK, SUCCESS, FAILURE };
        State state = INIT;

    protected:
        bool handleWithFSM(EventType event, cMessage *frameOrTimer);

        void transmitDataFrame();
        void retryDataFrame();
        void scheduleAckTimeout();
        bool isAck(Ieee80211Frame *frame);

    public:
        SendDataWithAckFsmBasedFrameExchange(cSimpleModule *ownerModule, IUpperMacContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *frame);
        ~SendDataWithAckFsmBasedFrameExchange();
};

class SendDataWithAckFrameExchange : public StepBasedFrameExchange
{
    protected:
        Ieee80211DataOrMgmtFrame *dataFrame = nullptr;
        int retryCount = 0;
    protected:
        virtual void doStep(int step);
        virtual bool processReply(int step, Ieee80211Frame *frame);
        virtual void processTimeout(int step);
        virtual void processInternalCollision(int step);
    public:
        SendDataWithAckFrameExchange(cSimpleModule *ownerModule, IUpperMacContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *dataFrame);
};

class SendDataWithRtsCtsFrameExchange : public StepBasedFrameExchange
{
    protected:
        Ieee80211DataOrMgmtFrame *dataFrame = nullptr;
        int retryCount = 0;
    protected:
        virtual void doStep(int step);
        virtual bool processReply(int step, Ieee80211Frame *frame);
        virtual void processTimeout(int step);
        virtual void processInternalCollision(int step) = 0;
    public:
        SendDataWithRtsCtsFrameExchange(cSimpleModule *ownerModule, IUpperMacContext *context, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *dataFrame);
};

} // namespace ieee80211
} // namespace inet

#endif


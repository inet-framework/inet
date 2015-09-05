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

#ifndef IEEE80211MACFRAMEEXCHANGE_H_
#define IEEE80211MACFRAMEEXCHANGE_H_

#include "Ieee80211MacPlugin.h"

namespace inet {
namespace ieee80211 {

class Ieee80211FrameExchange : public Ieee80211MacPlugin
{
    public:
        class IFinishedCallback {
            public:
                virtual void frameExchangeFinished(Ieee80211FrameExchange *what, bool successful) = 0;
                virtual ~IFinishedCallback() {}
        };

    protected:
        IFinishedCallback *finishedCallback = nullptr;

    protected:
        virtual void reportSuccess();
        virtual void reportFailure();

    public:
        Ieee80211FrameExchange(Ieee80211NewMac *mac, IFinishedCallback *callback) : Ieee80211MacPlugin(mac), finishedCallback(callback) {}
        virtual ~Ieee80211FrameExchange() {}

        virtual void start() = 0;

        virtual void lowerFrameReceived(Ieee80211Frame *frame) = 0;
        virtual void transmissionFinished() = 0;
};

class Ieee80211FSMBasedFrameExchange : public Ieee80211FrameExchange
{
    protected:
        cFSM fsm;
        enum EventType { EVENT_START, EVENT_FRAMEARRIVED, EVENT_TXFINISHED, EVENT_TIMER };

    protected:
        virtual void handleWithFSM(EventType eventType, cMessage *frameOrTimer) = 0;

    public:
        Ieee80211FSMBasedFrameExchange(Ieee80211NewMac *mac, IFinishedCallback *callback) : Ieee80211FrameExchange(mac, callback) { fsm.setName("Frame Exchange FSM"); }
        virtual void start() { EV_INFO << "Starting " << getClassName() << std::endl; handleWithFSM(EVENT_START, nullptr); }
        virtual void lowerFrameReceived(Ieee80211Frame *frame) { handleWithFSM(EVENT_FRAMEARRIVED, frame); }
        virtual void transmissionFinished() { handleWithFSM(EVENT_TXFINISHED, nullptr); }
        virtual void handleMessage(cMessage *timer) { handleWithFSM(EVENT_TIMER, timer); } //TODO make it handleTimer in MAC and MACPlugin too!
};

class Ieee80211SendDataWithAckFrameExchange : public Ieee80211FSMBasedFrameExchange
{
    protected:
        Ieee80211DataOrMgmtFrame *frame;
        int maxRetryCount;
        simtime_t ifs;
        int cwMin;
        int cwMax;

        int cw = 0;
        int retryCount = 0;
        cMessage *ackTimer = nullptr;

        enum State { INIT, TRANSMITDATA, WAITACK, SUCCESS, FAILURE };
        State state = INIT;

    protected:
        void handleWithFSM(EventType event, cMessage *frameOrTimer);

        void transmitDataFrame();
        void retryDataFrame();
        void scheduleAckTimeout();
        void processFrame(Ieee80211Frame *receivedFrame);
        bool isAck(Ieee80211Frame *frame);

    public:
        Ieee80211SendDataWithAckFrameExchange(Ieee80211NewMac *mac, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *frame, int maxRetryCount, simtime_t ifs, int cwMin, int cwMax) :
            Ieee80211FSMBasedFrameExchange(mac, callback), frame(frame), maxRetryCount(maxRetryCount), ifs(ifs), cwMin(cwMin), cwMax(cwMax) {}
        ~Ieee80211SendDataWithAckFrameExchange() { delete frame; if (ackTimer) delete cancelEvent(ackTimer); }
};

class Ieee80211SendRtsCtsFrameExchangeXXX : public Ieee80211FSMBasedFrameExchange
{
    protected:
        Ieee80211RTSFrame *rtsFrame;
        int maxRetryCount;
        simtime_t ifs;
        int cwMin;
        int cwMax;

        int cw = 0;
        int retryCount = 0;
        cMessage *ctsTimer = nullptr;

        enum State { INIT, TRANSMITRTS, WAITCTS, SUCCESS, FAILURE };
        State state = INIT;

    protected:
        void handleWithFSM(EventType event, cMessage *frameOrTimer);

        void transmitRtsFrame();
        void retryRtsFrame();
        void scheduleCtsTimeout();
        void processFrame(Ieee80211Frame *receivedFrame);
        bool isCts(Ieee80211Frame *frame);

    public:
        Ieee80211SendRtsCtsFrameExchangeXXX(Ieee80211NewMac *mac, IFinishedCallback *callback, Ieee80211RTSFrame *rtsFrame, int maxRetryCount, simtime_t ifs, int cwMin, int cwMax) :
            Ieee80211FSMBasedFrameExchange(mac, callback), rtsFrame(rtsFrame), maxRetryCount(maxRetryCount), ifs(ifs), cwMin(cwMin), cwMax(cwMax) {}
        ~Ieee80211SendRtsCtsFrameExchangeXXX() { delete rtsFrame; if (ctsTimer) delete cancelEvent(ctsTimer); }
};

/* IMPLEMENTATION DRAFT

class Ieee80211StepBasedFrameExchange : public Ieee80211FrameExchange  //TODO take code from summit slides!
{
    protected:
        int step = 0;
        enum Action { TRANSMIT_IMMEDIATE_FRAME, TRANSMIT_CONTENTION_FRAME, TRANSMIT_ACKED_CONTENTION_FRAME, EXPECT_REPLY };

    protected:
        virtual bool doStep(int step) = 0;  // true: frame exchange done
        virtual bool processReply(int step, Ieee80211Frame *frame) = 0; // true: frame accepted as reply

        void transmitImmediateFrame(Ieee80211Frame *frame, simtime_t ifs);
        void transmitContentionFrame(Ieee80211DataOrMgmtFrame *frame, int maxRetryCount, simtime_t ifs, int cw);
        void transmitAckedContentionFrame(Ieee80211DataOrMgmtFrame *frame, int maxRetryCount, simtime_t ifs, int cwMin, int cwMax, simtime_t timeout); //TODO this should be removed
        void expectReply(simtime_t timeout);

    public:
        Ieee80211StepBasedFrameExchange(Ieee80211NewMac *mac, IFinishedCallback *callback) : Ieee80211FrameExchange(mac, callback) { }
        virtual void start();
        virtual void lowerFrameReceived(Ieee80211Frame *frame);
        virtual void transmissionFinished();
        virtual void handleMessage(cMessage *timer);
};

class Ieee80211SendDataWithRtsCtsFrameExchange : public Ieee80211StepBasedFrameExchange
{
    protected:
        Ieee80211DataOrMgmtFrame *dataFrame;
    protected:
        virtual bool doStep(int step);
        virtual bool processReply(int step, Ieee80211Frame *frame);
    public:
        Ieee80211SendDataWithRtsCtsFrameExchange(Ieee80211NewMac *mac, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *dataFrame) : Ieee80211FrameExchange(mac, callback), dataFrame(dataFrame) { }
};

IMPLEMENTATION DRAFT */

}
} /* namespace inet */

#endif


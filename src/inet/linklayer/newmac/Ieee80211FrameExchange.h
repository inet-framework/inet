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

#ifndef IEEE80211MACFRAMEEXCHANGE_H_
#define IEEE80211MACFRAMEEXCHANGE_H_

#include "IIeee80211FrameExchange.h"
#include "Ieee80211MacPlugin.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "Ieee80211UpperMac.h"  //TODO remove

namespace inet {
namespace ieee80211 {

class Ieee80211MacContext;

class Ieee80211FrameExchange : public Ieee80211MacPlugin, public IIeee80211FrameExchange
{
    protected:
        IIeee80211MacContext *context = nullptr;
        IFinishedCallback *finishedCallback = nullptr;

        Ieee80211UpperMac *getUpperMac() { return (Ieee80211UpperMac *)mac->upperMac; }  //FIXME remove! todo remove 'mac' ptr!

    protected:
        virtual void reportSuccess();
        virtual void reportFailure();

    public:
        //TODO init context!
        Ieee80211FrameExchange(Ieee80211NewMac *mac, IIeee80211MacContext *context, IFinishedCallback *callback) : Ieee80211MacPlugin(mac), context(context), finishedCallback(callback) {}
        virtual ~Ieee80211FrameExchange() {}

        virtual void start() = 0;

        virtual bool lowerFrameReceived(Ieee80211Frame *frame) = 0;  // true = processed
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
        Ieee80211FSMBasedFrameExchange(Ieee80211NewMac *mac, IIeee80211MacContext *context, IFinishedCallback *callback) : Ieee80211FrameExchange(mac, context, callback) { fsm.setName("Frame Exchange FSM"); }
        virtual void start() { EV_INFO << "Starting " << getClassName() << std::endl; handleWithFSM(EVENT_START, nullptr); }
        virtual bool lowerFrameReceived(Ieee80211Frame *frame) { handleWithFSM(EVENT_FRAMEARRIVED, frame); return true; }
        virtual void transmissionFinished() { handleWithFSM(EVENT_TXFINISHED, nullptr); }
        virtual void handleMessage(cMessage *timer) { handleWithFSM(EVENT_TIMER, timer); } //TODO make it handleTimer in MAC and MACPlugin too!
};

class Ieee80211StepBasedFrameExchange : public Ieee80211FrameExchange
{
    protected:
        enum StepType { NONE, TRANSMIT_CONTENTION_FRAME, TRANSMIT_IMMEDIATE_FRAME, EXPECT_REPLY };
        enum Status { INPROGRESS, SUCCEEDED, FAILED };
        int step = 0;
        StepType stepType = NONE;
        Status status = INPROGRESS;
        cMessage *timeoutMsg = nullptr;

    protected:
        virtual void doStep(int step) = 0;
        virtual bool processReply(int step, Ieee80211Frame *frame) = 0; // true = frame accepted as reply
        virtual void processTimeout(int step) = 0;

        void transmitContentionFrame(Ieee80211Frame *frame, int retryCount);
        void transmitContentionFrame(Ieee80211Frame *frame, simtime_t ifs, simtime_t eifs, int cwMin, int cwMax, int retryCount);
        void transmitImmediateFrame(Ieee80211Frame *frame, simtime_t ifs);
        void expectReply(simtime_t timeout);
        void gotoStep(int step); // ~setNextStep()
        void fail();
        void succeed();

        void proceed();

    public:
        Ieee80211StepBasedFrameExchange(Ieee80211NewMac *mac, IIeee80211MacContext *context, IFinishedCallback *callback) : Ieee80211FrameExchange(mac, context, callback) { }
        virtual void start();
        virtual bool lowerFrameReceived(Ieee80211Frame *frame); // true = frame processed
        virtual void transmissionFinished();
        virtual void handleMessage(cMessage *timer);
};

}
} /* namespace inet */

#endif


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
// Author: Benjamin Seregi
//

#ifndef IEEE80211MACADVANCEDFRAMEEXCHANGE_H_
#define IEEE80211MACADVANCEDFRAMEEXCHANGE_H_

#include "Ieee80211FrameExchange.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include <functional>

class Ieee80211MacTransmission;

//namespace inet {
//namespace ieee80211 {
//
//class IStep
//{
//    protected:
//        const char *name = nullptr;
//
//    public:
//        const char *getName() const { return name; }
//
//        IStep(const char *name) : name(name) {}
//        virtual ~IStep() {}
//};
//
//class WaitTxCompleteStep : public IStep
//{
//    protected:
//        IStep *what = nullptr;
//    public:
//        WaitTxCompleteStep(const char *name, IStep *what) : IStep(name), what(what) {}
//        IStep *getWhat() const { return what; }
//};
//
//class ExchangeFinishedStep : public IStep
//{
//    public:
//        ExchangeFinishedStep(const char *name) : IStep(name) {}
//};
//
//class FrameExchangeStep : public IStep
//{
//    protected:
//        Ieee80211Frame *frame = nullptr;
//        int retryCount = 0;
//        simtime_t deferDuration = SIMTIME_ZERO;
//        int cw = 0;
//        int cwMax = 0;
//        bool immediate = false;
//        int maxRetryCount = 0;
//
//    public:
//        Ieee80211Frame* getFrame() const { return frame; }
//        bool isImmediate() const { return immediate; }
//        int getMaxRetryCount() const { return maxRetryCount; }
//        int getRetryCount() const { return retryCount; }
//        void increaseRetryCount() { retryCount++; }
//        void increaseContentionWindowIfPossible() { cw = cw < cwMax ? ((cw+1)<<1)-1 : cw; }
//        const simtime_t getDeferDuration() const { return deferDuration; }
//        int getCw() const { return cw; }
//
//        FrameExchangeStep(const char *name, Ieee80211Frame *frame, simtime_t deferDuration, int cw, int cwMax, bool isImmediate, int maxRetryCount) :
//            IStep(name), frame(frame), deferDuration(deferDuration), cw(cw), cwMax(cwMax), immediate(isImmediate), maxRetryCount(maxRetryCount) {}
//};
//
//class FrameExchangeStepWithResponse : public FrameExchangeStep
//{
//    protected:
//        simtime_t timeout = SIMTIME_ZERO;
//        std::function<bool(Ieee80211Frame *)> isFrameOk = nullptr;
//
//    public:
//        simtime_t getTimeout() const { return timeout; }
//        bool isResponseOk(Ieee80211Frame *frame) { return isFrameOk(frame); }
//
//        FrameExchangeStepWithResponse(const char *name, Ieee80211Frame *frame, simtime_t deferDuration, int cw, int cwMax, bool isImmediate, simtime_t timeout, std::function<bool(Ieee80211Frame *)> isFrameOk, int maxRetryCount) :
//            FrameExchangeStep(name, frame, deferDuration, cw, cwMax, isImmediate, maxRetryCount), timeout(timeout), isFrameOk(isFrameOk) {}
//};
//
//class Ieee80211AdvancedFrameExchange : public Ieee80211FrameExchange
//{
//    protected:
//        ExchangeFinishedStep *exchangeFinished = nullptr;
//
//        int stepId = 0;
//        IStep *currentStep = nullptr;
//        cMessage *timeout = nullptr;
//
//        virtual void setCurrentStep(int step) = 0;
//        void doStep();
//
//    public:
//        virtual void start() { EV_INFO << "Starting " << getClassName() << std::endl; setCurrentStep(0); doStep(); }
//        virtual void lowerFrameReceived(Ieee80211Frame *frame);
//        virtual void transmissionFinished();
//        virtual void transmit(FrameExchangeStep *step);
//        virtual void handleMessage(cMessage *timer);
//
//        Ieee80211AdvancedFrameExchange(Ieee80211NewMac *mac, IFinishedCallback *callback);
//        ~Ieee80211AdvancedFrameExchange() { delete cancelEvent(timeout); delete exchangeFinished; }
//};
//
//class Ieee80211SendRtsCtsFrameExchange : virtual public Ieee80211AdvancedFrameExchange
//{
//    protected:
//        FrameExchangeStepWithResponse *rtsCtsExchange = nullptr;
//        WaitTxCompleteStep *waitRtsTxComplete = nullptr;
//
//    protected:
//        simtime_t computeCtsTimeout();
//        bool isCtsFrame(Ieee80211Frame *frame) const { return dynamic_cast<Ieee80211CTSFrame *>(frame) != nullptr; }
//        Ieee80211RTSFrame *buildRtsFrame(Ieee80211DataOrMgmtFrame *frameToSend);
//
//        void setCurrentStep(int step);
//
//    public:
//        Ieee80211SendRtsCtsFrameExchange(Ieee80211NewMac *mac, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *frameToSend);
//        ~Ieee80211SendRtsCtsFrameExchange() { delete rtsCtsExchange; }
//};
//
//class Ieee80211SendDataAckFrameExchange : virtual public Ieee80211AdvancedFrameExchange
//{
//    protected:
//        FrameExchangeStepWithResponse *dataAckExchange = nullptr;
//        WaitTxCompleteStep *waitDataTxComplete = nullptr;
//
//    protected:
//        simtime_t computeAckTimeout(Ieee80211DataOrMgmtFrame *frameToSend);
//        bool isAckFrame(Ieee80211Frame *frame) const { return dynamic_cast<Ieee80211ACKFrame *>(frame) != nullptr; }
//
//        void setCurrentStep(int step);
//
//    public:
//        Ieee80211SendDataAckFrameExchange(Ieee80211NewMac *mac, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *frameToSend);
//        ~Ieee80211SendDataAckFrameExchange() { delete dataAckExchange; }
//};
//
//class Ieee80211SendRtsCtsDataAckFrameExchange : public Ieee80211SendRtsCtsFrameExchange, public Ieee80211SendDataAckFrameExchange
//{
//    protected:
//        void setCurrentStep(int step);
//
//    public:
//        Ieee80211SendRtsCtsDataAckFrameExchange(Ieee80211NewMac *mac, IFinishedCallback *callback, Ieee80211DataOrMgmtFrame *frameToSend);
//};
//
//}
//} /* namespace inet */

#endif /* IEEE80211MACADVANCEDFRAMEEXCHANGE_H_ */

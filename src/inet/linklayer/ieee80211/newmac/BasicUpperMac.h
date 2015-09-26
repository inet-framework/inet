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

#ifndef __INET_BASICUPPERMAC_H
#define __INET_BASICUPPERMAC_H

#include "IUpperMac.h"
#include "IFrameExchange.h"

namespace inet {

class IPassiveQueue;

namespace ieee80211 {

class IRx;
class ITxCallback;
class Ieee80211NewMac;
class Ieee80211RTSFrame;

class BasicUpperMac : public cSimpleModule, public IUpperMac, protected IFrameExchange::IFinishedCallback
{
    public:
        typedef std::list<Ieee80211DataOrMgmtFrame*> Ieee80211DataOrMgmtFrameList;

    protected:
        IPassiveQueue *queueModule = nullptr;
        Ieee80211NewMac *mac = nullptr;
        IRx *rx = nullptr;

        IUpperMacContext *context = nullptr;

        int maxQueueSize;
        int fragmentationThreshold = 2346;
        uint16 sequenceNumber;

        struct AccessCategoryData {
            Ieee80211DataOrMgmtFrameList transmissionQueue;
            IFrameExchange *frameExchange = nullptr;
        };
        AccessCategoryData *acData = nullptr;  // dynamically allocated array

    protected:
        void initialize() override;
        void handleMessage(cMessage *msg) override;
        virtual void initializeQueueModule();
        virtual IUpperMacContext *createContext();
        virtual int classifyFrame(Ieee80211DataOrMgmtFrame *frame);

        virtual void startSendDataFrameExchange(Ieee80211DataOrMgmtFrame *frame, int accessCategory);
        virtual void frameExchangeFinished(IFrameExchange *what, bool successful) override;

        void sendAck(Ieee80211DataOrMgmtFrame *frame);
        void sendCts(Ieee80211RTSFrame *frame);

    public:
        BasicUpperMac();
        ~BasicUpperMac();
        virtual void setContext(IUpperMacContext *context) override { this->context = context; }
        virtual void upperFrameReceived(Ieee80211DataOrMgmtFrame *frame) override;
        virtual void lowerFrameReceived(Ieee80211Frame *frame) override;
        virtual void transmissionComplete(ITxCallback *callback, int txIndex) override;
        virtual void internalCollision(ITxCallback *callback, int txIndex) override;
};

} // namespace ieee80211
} // namespace inet

#endif


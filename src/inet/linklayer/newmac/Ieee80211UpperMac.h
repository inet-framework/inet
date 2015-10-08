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

#ifndef __INET_IEEE80211UPPERMAC_H
#define __INET_IEEE80211UPPERMAC_H

#include "Ieee80211MacPlugin.h"
#include "IIeee80211UpperMac.h"
#include "IIeee80211MacTx.h"
#include "IIeee80211MacRx.h"
#include "IIeee80211FrameExchange.h"

namespace inet {

class IPassiveQueue;

namespace ieee80211 {

class Ieee80211NewMac;

class Ieee80211UpperMac : public cSimpleModule, public IIeee80211UpperMac, public IIeee80211FrameExchange::IFinishedCallback, public IIeee80211MacTx::ICallback
{
    public:
        typedef std::list<Ieee80211DataOrMgmtFrame*> Ieee80211DataOrMgmtFrameList;

    protected:
        Ieee80211NewMac *mac = nullptr;
        IIeee80211MacTx *tx = nullptr;
        IIeee80211MacRx *rx = nullptr;

        /** Maximum number of frames in the queue; should be set in the omnetpp.ini */
        int maxQueueSize;

        /** Messages longer than this threshold will be sent in multiple fragments. see spec 361 */
        static const int fragmentationThreshold = 2346;
        //@}
        /** Messages received from upper layer and to be transmitted later */
        Ieee80211DataOrMgmtFrameList transmissionQueue;

        /** Sequence number to be assigned to the next frame */
        uint16 sequenceNumber;

        /** Passive queue module to request messages from */
        IPassiveQueue *queueModule = nullptr;

        IIeee80211FrameExchange *frameExchange = nullptr;

        IIeee80211UpperMacContext *context = nullptr; //TODO fill in!

    protected:
        void initialize();
        void handleMessage(cMessage *msg);

        virtual void frameExchangeFinished(IIeee80211FrameExchange *what, bool successful);

        virtual Ieee80211DataOrMgmtFrame *buildBroadcastFrame(Ieee80211DataOrMgmtFrame *frameToSend);
        void initializeQueueModule();

        void sendAck(Ieee80211DataOrMgmtFrame *frame);
        void sendCts(Ieee80211RTSFrame *frame);

        virtual void transmissionComplete(int txIndex) override;
        virtual void internalCollision(int txIndex) override;

    public:
        Ieee80211UpperMac();
        ~Ieee80211UpperMac();
        virtual void setContext(IIeee80211UpperMacContext *context) override { this->context = context; }
        virtual void upperFrameReceived(Ieee80211DataOrMgmtFrame *frame) override;
        virtual void lowerFrameReceived(Ieee80211Frame *frame) override;

};

} // namespace ieee80211
} // namespace inet

#endif


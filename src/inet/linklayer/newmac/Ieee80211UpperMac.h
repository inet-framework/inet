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

#ifndef IEEE80211UPPERMAC_H_
#define IEEE80211UPPERMAC_H_

#include "inet/common/queue/IPassiveQueue.h"
#include "Ieee80211NewMac.h"
#include "Ieee80211MacPlugin.h"
#include "Ieee80211FrameExchange.h"
#include "Ieee80211MacAdvancedFrameExchange.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "Ieee80211MacTransmission.h"

namespace inet {

namespace ieee80211 {

class Ieee80211NewMac;
class ITransmissionCompleteCallback;

class Ieee80211UpperMac : public Ieee80211MacPlugin, public Ieee80211FrameExchange::IFinishedCallback, public ITransmissionCompleteCallback
{
    public:
        typedef std::list<Ieee80211DataOrMgmtFrame*> Ieee80211DataOrMgmtFrameList;

    protected:
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

        Ieee80211FrameExchange *frameExchange = nullptr;

        IIeee80211MacContext *context = nullptr; //TODO fill in!

    protected:
        void handleMessage(cMessage *msg);

        virtual void frameExchangeFinished(Ieee80211FrameExchange *what, bool successful);

        virtual Ieee80211DataOrMgmtFrame *buildBroadcastFrame(Ieee80211DataOrMgmtFrame *frameToSend);
        void initializeQueueModule();

        void sendAck(Ieee80211DataOrMgmtFrame *frame);
        void sendCts(Ieee80211RTSFrame *frame);

    public:
        Ieee80211UpperMac(Ieee80211NewMac *mac);
        ~Ieee80211UpperMac();
        void setContext(IIeee80211MacContext *context) { this->context = context; }
        void upperFrameReceived(Ieee80211DataOrMgmtFrame *frame);
        void lowerFrameReceived(Ieee80211Frame *frame);
        void transmissionComplete(Ieee80211MacTransmission *tx); // callback for MAC

};

} // namespace 80211

} /* namespace inet */

#endif /* IEEE80211UPPERMAC_H_ */

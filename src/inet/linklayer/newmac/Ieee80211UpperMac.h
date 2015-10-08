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

#include "inet_old/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/common/queue/IPassiveQueue.h"
#include "Ieee80211NewMac.h"
#include "Ieee80211MacPlugin.h"
#include "Ieee80211MacFrameExchange.h"
#include "Ieee80211MacAdvancedFrameExchange.h"

namespace inet {

class Ieee80211NewMac;

class Ieee80211UpperMac : public Ieee80211MacPlugin, public Ieee80211FrameExchange::IFinishedCallback
{
    public:
        typedef std::list<Ieee80211DataOrMgmtFrame*> Ieee80211DataOrMgmtFrameList;
        /**
         * This is used to populate fragments and identify duplicated messages. See spec 9.2.9.
         */
        struct Ieee80211ASFTuple
        {
            MACAddress address;
            int sequenceNumber;
            int fragmentNumber;
        };
        typedef std::list<Ieee80211ASFTuple*> Ieee80211ASFTupleList;

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

        /**
         * Number of frame retransmission attempts, this is a simpification of
         * SLRC and SSRC, see 9.2.4 in the spec
         */
        int retryCounter;
        int cwMinData;
        int cwMinBroadcast;

        /**
         * A list of last sender, sequence and fragment number tuples to identify
         * duplicates, see spec 9.2.9.
         * TODO: this is not yet used
         */
        Ieee80211ASFTupleList asfTuplesList;

        virtual Ieee80211DataOrMgmtFrame *setDataFrameDuration(Ieee80211DataOrMgmtFrame *frameToSend);
        virtual Ieee80211ACKFrame *buildACKFrame(Ieee80211DataOrMgmtFrame *frameToACK);
        virtual Ieee80211DataOrMgmtFrame *buildBroadcastFrame(Ieee80211DataOrMgmtFrame *frameToSend);

        virtual simtime_t getEIFS() const;
        virtual simtime_t getPIFS() const;

        virtual Ieee80211Frame *setBasicBitrate(Ieee80211Frame *frame);

        /** @brief Returns true if message destination address is ours */
        virtual bool isForUs(Ieee80211Frame *msg) const;
        /** @brief Returns true if message is a broadcast message */
        virtual bool isBroadcast(Ieee80211Frame *msg) const;
        void handleMessage(cMessage *msg);
        virtual void frameExchangeFinished(Ieee80211FrameExchange *what, bool successful);
        void sendAck(Ieee80211DataOrMgmtFrame *frame);
        Ieee80211CTSFrame *buildCtsFrame(Ieee80211RTSFrame *frame);
        void sendCts(Ieee80211RTSFrame *frame);

    protected:
        void initializeQueueModule();

    public:
        double computeFrameDuration(Ieee80211Frame *msg); // TODO
        double computeFrameDuration(int bits, double bitrate); // TODO
        virtual simtime_t getSIFS() const; // TODO
        virtual simtime_t getDIFS() const; // TODO
        void upperFrameReceived(Ieee80211DataOrMgmtFrame *frame);
        void lowerFrameReceived(Ieee80211Frame *frame);
        void transmissionFinished(); // callback for MAC

        Ieee80211UpperMac(Ieee80211NewMac *mac);
        ~Ieee80211UpperMac();
};

} /* namespace inet */

#endif /* IEEE80211UPPERMAC_H_ */

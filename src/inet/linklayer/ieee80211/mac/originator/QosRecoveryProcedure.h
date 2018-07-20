//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_QOSRECOVERYPROCEDURE_H
#define __INET_QOSRECOVERYPROCEDURE_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/common/AccessCategory.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"
#include "inet/linklayer/ieee80211/mac/common/SequenceControlField.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecoveryProcedure.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

//
// References: 9.19.2.6 Retransmit procedures (IEEE 802.11-2012 STD)
//             802.11 Reference Design: Recovery Procedures and Retransmit Limits
//             (https://warpproject.org/trac/wiki/802.11/MAC/Lower/Retransmissions)
/*
 * TODO: stationRetryCounter
 */
class INET_API QosRecoveryProcedure : public cSimpleModule, public IRecoveryProcedure
{

    protected:
        ICwCalculator *cwCalculator = nullptr;

        std::map<std::pair<Tid, SequenceControlField>, int> shortRetryCounter; // SRC
        std::map<std::pair<Tid, SequenceControlField>, int> longRetryCounter; // LRC

        int stationLongRetryCounter = 0; // QLRC
        int stationShortRetryCounter = 0; // QSRC

        int shortRetryLimit = -1;
        int longRetryLimit = -1;
        int rtsThreshold = -1;

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;

        void incrementCounter(const Ptr<const Ieee80211DataHeader>& header, std::map<std::pair<Tid, SequenceControlField>, int>& retryCounter);
        void incrementStationSrc();
        void incrementStationLrc();
        void resetStationSrc() { stationShortRetryCounter = 0; }
        void resetStationLrc() { stationLongRetryCounter = 0; }
        void resetContentionWindow();
        int doubleCw(int cw);
        int getRc(Packet *packet, const Ptr<const Ieee80211DataHeader>& header, std::map<std::pair<Tid, SequenceControlField>, int>& retryCounter);
        bool isMulticastFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header);

    public:
        virtual void multicastFrameTransmitted();

        virtual void ctsFrameReceived();
        virtual void ackFrameReceived(Packet *packet, const Ptr<const Ieee80211DataHeader>& ackedHeader);
        virtual void blockAckFrameReceived();

        virtual void rtsFrameTransmissionFailed(const Ptr<const Ieee80211DataHeader>& protectedHeader);
        virtual void dataFrameTransmissionFailed(Packet *packet, const Ptr<const Ieee80211DataHeader>& failedHeader);

        virtual bool isRetryLimitReached(Packet *packet, const Ptr<const Ieee80211DataHeader>& failedHeader);
        virtual int getRetryCount(Packet *packet, const Ptr<const Ieee80211DataHeader>& header);
        virtual bool isRtsFrameRetryLimitReached(Packet *packet, const Ptr<const Ieee80211DataHeader>& protectedHeader);

        virtual void retryLimitReached(Packet *packet, const Ptr<const Ieee80211DataHeader>& header);

        virtual int getLongRetryLimit() { return longRetryLimit; }
        virtual int getShortRetryLimit() { return shortRetryLimit; }

};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_RECOVERYPROCEDURE_H

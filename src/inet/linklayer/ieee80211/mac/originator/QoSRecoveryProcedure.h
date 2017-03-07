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
class INET_API QoSRecoveryProcedure : public cSimpleModule, public IRecoveryProcedure
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

        void incrementCounter(Ieee80211DataFrame* frame, std::map<std::pair<Tid, SequenceControlField>, int>& retryCounter);
        void incrementStationSrc();
        void incrementStationLrc();
        void resetStationSrc() { stationShortRetryCounter = 0; }
        void resetStationLrc() { stationLongRetryCounter = 0; }
        void resetContentionWindow();
        int doubleCw(int cw);
        int getRc(Ieee80211DataFrame* frame, std::map<std::pair<Tid, SequenceControlField>, int>& retryCounter);
        bool isMulticastFrame(Ieee80211Frame *frame);

    public:
        virtual void multicastFrameTransmitted();

        virtual void ctsFrameReceived();
        virtual void ackFrameReceived(Ieee80211DataFrame *ackedFrame);
        virtual void blockAckFrameReceived();

        virtual void rtsFrameTransmissionFailed(Ieee80211DataFrame *protectedFrame);
        virtual void dataFrameTransmissionFailed(Ieee80211DataFrame *failedFrame);

        virtual bool isRetryLimitReached(Ieee80211DataFrame* failedFrame);
        virtual int getRetryCount(Ieee80211DataFrame* frame);
        virtual bool isRtsFrameRetryLimitReached(Ieee80211DataFrame* protectedFrame);

        virtual void retryLimitReached(Ieee80211DataFrame *frame);

        virtual int getLongRetryLimit() { return longRetryLimit; }
        virtual int getShortRetryLimit() { return shortRetryLimit; }

};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_RECOVERYPROCEDURE_H

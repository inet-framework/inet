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

#ifndef __INET_NONQOSRECOVERYPROCEDURE_H
#define __INET_NONQOSRECOVERYPROCEDURE_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/common/AccessCategory.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"
#include "inet/linklayer/ieee80211/mac/common/SequenceControlField.h"
#include "inet/linklayer/ieee80211/mac/common/StationRetryCounters.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecoveryProcedure.h"

namespace inet {
namespace ieee80211 {

//
// References: 9.19.2.6 Retransmit procedures (IEEE 802.11-2012 STD)
//             802.11 Reference Design: Recovery Procedures and Retransmit Limits
//             (https://warpproject.org/trac/wiki/802.11/MAC/Lower/Retransmissions)
class INET_API NonQosRecoveryProcedure : public cSimpleModule, public IRecoveryProcedure
{

    protected:
        ICwCalculator *cwCalculator = nullptr;

        std::map<SequenceControlField, int> shortRetryCounter; // SRC
        std::map<SequenceControlField, int> longRetryCounter; // LRC

        int shortRetryLimit = -1;
        int longRetryLimit = -1;
        int rtsThreshold = -1;

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;

        virtual void incrementCounter(const Ptr<const Ieee80211DataOrMgmtHeader>& header, std::map<SequenceControlField, int>& retryCounter);
        virtual void incrementContentionWindow();
        virtual void resetContentionWindow();
        virtual int getRc(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header, std::map<SequenceControlField, int>& retryCounter);
        virtual bool isMulticastFrame(const Ptr<const Ieee80211MacHeader>& header);
        virtual void incrementStationSrc(StationRetryCounters *stationCounters);
        virtual void incrementStationLrc(StationRetryCounters *stationCounters);

    public:
        virtual void multicastFrameTransmitted(StationRetryCounters *stationCounters);

        virtual void ctsFrameReceived(StationRetryCounters *stationCounters);
        virtual void ackFrameReceived(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& ackedHeader, StationRetryCounters *stationCounters);

        virtual void rtsFrameTransmissionFailed(const Ptr<const Ieee80211DataOrMgmtHeader>& protectedHeader, StationRetryCounters *stationCounters);
        virtual void dataOrMgmtFrameTransmissionFailed(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& failedHeader, StationRetryCounters *stationCounters);
        virtual int getRetryCount(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header);
        virtual int getShortRetryCount(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader);
        virtual int getLongRetryCount(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader);

        virtual bool isRetryLimitReached(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& failedHeader);
        virtual bool isRtsFrameRetryLimitReached(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& protectedHeader);

        virtual void retryLimitReached(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header);

        virtual int getLongRetryLimit() { return longRetryLimit; }
        virtual int getShortRetryLimit() { return shortRetryLimit; }

};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_NONQOSRECOVERYPROCEDURE_H

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

#ifndef __INET_BASICSTATISTICS_H
#define __INET_BASICSTATISTICS_H

#include "inet/linklayer/ieee80211/mac/contract/IStatistics.h"

namespace inet {
namespace ieee80211 {

/**
 * A basic implementation of statistics collection (IStatistics).
 */
class INET_API BasicStatistics : /*public IStatistics, */public cSimpleModule
{
//    private:
//        MacUtils *utils = nullptr;
//        IRateControl *rateControl = nullptr; //TODO maybe there should be a listener list instead of a direct pointer here
//
//        long numRetry;
//        long numSentWithoutRetry;
//        long numGivenUp;
//        long numCollision;
//        long numSent;
//        long numSentBroadcast;
//
//        long numReceivedUnicast;
//        long numReceivedMulticast;
//        long numReceivedBroadcast;
//        long numReceivedNotForUs;
//        long numReceivedErroneous;
//
//    protected:
//        virtual void initialize() override;
//        virtual void finish() override;
//        virtual void resetStatistics();
//
//    public:
//        virtual void setMacUtils(MacUtils *utils) override;
//        virtual void setRateControl(IRateControl *rateControl) override;
//        virtual void frameTransmissionSuccessful(const Ptr<const Ieee80211DataOrMgmtHeader>& header, int retryCount) override;
//        virtual void frameTransmissionUnsuccessful(const Ptr<const Ieee80211DataOrMgmtHeader>& header, int retryCount) override;
//        virtual void frameTransmissionUnsuccessfulGivingUp(const Ptr<const Ieee80211DataOrMgmtHeader>& header, int retryCount) override;
//        virtual void frameTransmissionGivenUp(const Ptr<const Ieee80211DataOrMgmtHeader>& header) override;
//        virtual void frameReceived(const Ptr<const Ieee80211MacHeader>& header) override;
//        virtual void erroneousFrameReceived() override;
};

}  // namespace ieee80211
}  // namespace inet

#endif

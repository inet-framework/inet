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

#ifndef __INET_BASICSTATISTICS_H
#define __INET_BASICSTATISTICS_H

#include "IStatistics.h"

namespace inet {
namespace ieee80211 {

/**
 * A basic implementation of statistics collection (IStatistics).
 */
class INET_API BasicStatistics : public IStatistics, public cSimpleModule
{
    private:
        MacUtils *utils = nullptr;
        IRateControl *rateControl = nullptr; //TODO maybe there should be a listener list instead of a direct pointer here

        long numRetry;
        long numSentWithoutRetry;
        long numGivenUp;
        long numCollision;
        long numSent;
        long numSentBroadcast;

        long numReceivedUnicast;
        long numReceivedMulticast;
        long numReceivedBroadcast;
        long numReceivedNotForUs;
        long numReceivedErroneous;

    protected:
        virtual void initialize() override;
        virtual void finish() override;
        virtual void resetStatistics();

    public:
        virtual void setMacUtils(MacUtils *utils) override;
        virtual void setRateControl(IRateControl *rateControl) override;
        virtual void frameTransmissionSuccessful(Ieee80211DataOrMgmtFrame *frame, int retryCount) override;
        virtual void frameTransmissionUnsuccessful(Ieee80211DataOrMgmtFrame *frame, int retryCount) override;
        virtual void frameTransmissionUnsuccessfulGivingUp(Ieee80211DataOrMgmtFrame *frame, int retryCount) override;
        virtual void frameTransmissionGivenUp(Ieee80211DataOrMgmtFrame *frame) override;
        virtual void frameReceived(Ieee80211Frame *frame) override;
        virtual void erroneousFrameReceived() override;
};

}  // namespace ieee80211
}  // namespace inet

#endif

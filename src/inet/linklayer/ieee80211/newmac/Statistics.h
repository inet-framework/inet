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

#ifndef __INET_STATISTICS_H
#define __INET_STATISTICS_H

#include "IStatistics.h"

namespace inet {
namespace ieee80211 {

class MacUtils;
class IRateControl;

class INET_API BasicStatistics : public IStatistics
{
    private:
        MacUtils *utils;
        IRateControl *rateControl = nullptr; //TODO maybe there should be a listener list instead of a direct pointer here

    public:
        BasicStatistics(MacUtils *utils) : utils(utils) {}
        virtual void setRateControl(IRateControl *rateControl) override;
        virtual void frameTransmissionSuccessful(Ieee80211DataOrMgmtFrame *frame, int retryCount) override;
        virtual void frameTransmissionUnsuccessful(Ieee80211DataOrMgmtFrame *frame, int retryCount) override;
        virtual void frameTransmissionUnsuccessfulGivingUp(Ieee80211DataOrMgmtFrame *frame, int retryCount) override;
        virtual void frameTransmissionGivenUp(Ieee80211DataOrMgmtFrame *frame) override;
        virtual void frameReceived(Ieee80211Frame *frame) override;
};

}  // namespace ieee80211
}  // namespace inet

#endif

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

#ifndef __INET_ISTATISTICS_H
#define __INET_ISTATISTICS_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace ieee80211 {

class MacUtils;
class IRateControl;
class Ieee80211Frame;
class Ieee80211DataOrMgmtFrame;

/**
 * Abstract interface for statistics collection within the 802.11 MAC.
 * IStatistics is notified of various events that occur in the MAC,
 * and it is up to the concrete IStatistics implementation to decide
 * what statistics to collect from them.
 *
 * Note that dynamic rate control algorithms (IRateControl) also plug into
 * IStatistics, because due to the nature of its input.
 */
class INET_API IStatistics
{
    public:
        ~IStatistics() {}
        virtual void setMacUtils(MacUtils *utils) = 0;
        virtual void setRateControl(IRateControl *rateControl) = 0;  // is interested in the statistics

        // events to compute statistics from; TODO there should be many more, e.g. about the contention, queueing time, etc
        virtual void frameTransmissionSuccessful(Ieee80211DataOrMgmtFrame *frame, int retryCount) = 0;
        virtual void frameTransmissionUnsuccessful(Ieee80211DataOrMgmtFrame *frame, int retryCount) = 0;
        virtual void frameTransmissionUnsuccessfulGivingUp(Ieee80211DataOrMgmtFrame *frame, int retryCount) = 0;
        virtual void frameTransmissionGivenUp(Ieee80211DataOrMgmtFrame *frame) = 0;
        virtual void frameReceived(Ieee80211Frame *frame) = 0;
        virtual void erroneousFrameReceived() = 0;
};

}  // namespace ieee80211
}  // namespace inet

#endif

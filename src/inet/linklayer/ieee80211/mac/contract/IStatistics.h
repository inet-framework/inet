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

#ifndef __INET_ISTATISTICS_H
#define __INET_ISTATISTICS_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

/**
 * Abstract interface for statistics collection within the 802.11 MAC.
 * IStatistics is notified of various events that occur in the MAC,
 * and it is up to the concrete IStatistics implementation to decide
 * what statistics to collect from them.
 */
class INET_API IStatistics
{
    public:
        ~IStatistics() { }

        // events to compute statistics from; TODO there should be many more, e.g. about the contention, queueing time, etc
        virtual void frameTransmissionSuccessful(const Ptr<const Ieee80211DataOrMgmtHeader>& header, int retryCount) = 0;
        virtual void frameTransmissionUnsuccessful(const Ptr<const Ieee80211DataOrMgmtHeader>& header, int retryCount) = 0;
        virtual void frameTransmissionUnsuccessfulGivingUp(const Ptr<const Ieee80211DataOrMgmtHeader>& header, int retryCount) = 0;
        virtual void frameTransmissionGivenUp(const Ptr<const Ieee80211DataOrMgmtHeader>& header) = 0;
        virtual void frameReceived(const Ptr<const Ieee80211MacHeader>& header) = 0;
        virtual void erroneousFrameReceived() = 0;
};

}  // namespace ieee80211
}  // namespace inet

#endif // #ifndef __INET_ISTATISTICS_H

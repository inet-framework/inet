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

#ifndef __INET_IMACPARAMETERS_H
#define __INET_IMACPARAMETERS_H

#include "AccessCategory.h"
#include "inet/linklayer/common/MACAddress.h"

namespace inet {
namespace ieee80211 {

/**
 * Frame exchange classes and other parts of UpperMac access channel access
 * parameters and other parts of the MAC configuration via this interface.
 */
class INET_API IMacParameters
{
    public:
        IMacParameters() {}
        virtual ~IMacParameters() {}

        virtual const MACAddress& getAddress() const = 0;

        // timing parameters
        virtual bool isEdcaEnabled() const = 0;
        virtual simtime_t getSlotTime() const = 0;
        virtual simtime_t getSifsTime() const = 0;
        virtual simtime_t getAifsTime(AccessCategory ac) const = 0;
        virtual simtime_t getEifsTime(AccessCategory ac) const = 0;
        virtual simtime_t getPifsTime() const = 0;
        virtual simtime_t getRifsTime() const = 0;
        virtual int getCwMin(AccessCategory ac) const = 0;
        virtual int getCwMax(AccessCategory ac) const = 0;
        virtual int getCwMulticast(AccessCategory ac) const = 0;
        virtual simtime_t getTxopLimit(AccessCategory ac) const = 0;

        // other parameters
        virtual int getShortRetryLimit() const = 0;
        virtual int getLongRetryLimit() const = 0;
        virtual int getRtsThreshold() const = 0;
        virtual simtime_t getPhyRxStartDelay() const = 0;
        virtual bool getUseFullAckTimeout() const = 0;
};

} // namespace ieee80211
} // namespace inet

#endif


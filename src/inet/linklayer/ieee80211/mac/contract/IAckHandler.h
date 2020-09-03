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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef __INET_IACKHANDLER_H
#define __INET_IACKHANDLER_H

namespace inet {
namespace ieee80211 {

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

class INET_API IAckHandler
{
    public:
        virtual ~IAckHandler() { }

        virtual bool isEligibleToTransmit(const Ptr<const Ieee80211DataOrMgmtHeader>& header) = 0;
        virtual bool isOutstandingFrame(const Ptr<const Ieee80211DataOrMgmtHeader>& header) = 0;
        virtual void frameGotInProgress(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif


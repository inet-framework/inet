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

#ifndef __INET_IRECIPIENTMACDATASERVICE_H
#define __INET_IRECIPIENTMACDATASERVICE_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class INET_API IRecipientMacDataService
{
    public:
        virtual std::vector<Ieee80211Frame *> dataFrameReceived(Ieee80211DataFrame *dataFrame) = 0;
        virtual std::vector<Ieee80211Frame *> controlFrameReceived(Ieee80211Frame *controlFrame) = 0;
        virtual std::vector<Ieee80211Frame *> managementFrameReceived(Ieee80211ManagementFrame *mgmtFrame) = 0;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_IRECIPIENTMACDATASERVICE_H

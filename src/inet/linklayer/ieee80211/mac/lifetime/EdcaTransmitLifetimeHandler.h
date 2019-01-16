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

#ifndef __INET_EDCATRANSMITLIFETIMEHANDLER_H
#define __INET_EDCATRANSMITLIFETIMEHANDLER_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/common/AccessCategory.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"
#include "inet/linklayer/ieee80211/mac/contract/ITransmitLifetimeHandler.h"

namespace inet {
namespace ieee80211 {

class INET_API EdcaTransmitLifetimeHandler : public ITransmitLifetimeHandler
{
    public:
        simtime_t msduLifetime[4];
        std::map<SequenceNumber, simtime_t> lifetimes;

    protected:
        AccessCategory mapTidToAc(int tid); // TODO: copy

    public:
        EdcaTransmitLifetimeHandler(simtime_t bkLifetime, simtime_t beLifetime, simtime_t viLifetime, simtime_t voLifetime);

        virtual void frameGotInProgess(const Ptr<const Ieee80211DataHeader>& header);
        virtual void frameTransmitted(const Ptr<const Ieee80211DataHeader>& header);

        virtual bool isLifetimeExpired(const Ptr<const Ieee80211DataHeader>& header);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef EDCATRANSMITLIFETIMEHANDLER_H

//
// Copyright (C) 2015 OpenSim Ltd.
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_IEEE80211MACBACKOFF_H
#define __INET_IEEE80211MACBACKOFF_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/ieee80211/thenewmac/reception/Ieee80211MacChannelState.h"

namespace inet {
namespace ieee80211 {

class INET_API IIeee80211MacBackoff : public cSimpleModule
{
    public:
        enum BkofSignal
        {
            BKOF_BACKOFF,
            BKOF_BKDONE,
            BKOF_CANCEL
        };
    public:
        virtual void handleBkofSignal(BkofSignal signal) = 0;
        virtual void handleFwdCsSignal(IIeee80211MacChannelState::ChannelState channelState) = 0;
};

class INET_API Ieee80211MacBackoff : public IIeee80211MacBackoff
{
    protected:
        void handleMessage(cMessage *msg) override;
        void initialize(int stage) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    public:
        // Signal handlers
        void handleBkofSignal(BkofSignal signal) override;
        void handleFwdCsSignal(IIeee80211MacChannelState::ChannelState channelState) override;
};

} /* namespace inet */
} /* namespace ieee80211 */

#endif // ifndef __INET_IEEE80211MACBACKOFF_H

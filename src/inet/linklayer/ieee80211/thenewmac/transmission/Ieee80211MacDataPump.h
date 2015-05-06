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

#ifndef __INET_IEEE80211MACDATAPUMP_H
#define __INET_IEEE80211MACDATAPUMP_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/ieee80211/thenewmac/reception/Ieee80211MacChannelState.h"

namespace inet {
namespace ieee80211 {

class INET_API IIeee80211MacDataPump : public cSimpleModule
{
    public:
        virtual void handleFromCsSignal(IIeee80211MacChannelState::ChannelState channelState) = 0;
        virtual void handleTxrqSignal() = 0;
};

class INET_API Ieee80211MacDataPump : public IIeee80211MacDataPump
{
    protected:
        void handleMessage(cMessage *msg) override;
        void initialize(int stage) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    public:
        // Signal handlers
        void handleFromCsSignal(IIeee80211MacChannelState::ChannelState channelState);
        virtual void handleTxrqSignal();
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211MACDATAPUMP_H

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

#ifndef __INET_IEEE80211MACCHANNELSTATE_H
#define __INET_IEEE80211MACCHANNELSTATE_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/ieee80211/thenewmac/reception/Ieee80211MacReception.h"

namespace inet {
namespace ieee80211 {

class INET_API IIeee80211MacChannelState : public cSimpleModule
{
    public:
        enum ChannelState
        {
            CHANNEL_STATE_BUSY,
            CHANNEL_STATE_IDLE,
            CHANNEL_STATE_SLOT
        };
};

class INET_API Ieee80211MacChannelState : public IIeee80211MacChannelState
{
    protected:
        void handleMessage(cMessage *msg) override;
        void initialize(int stage) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }

        // Signal emitters
        void emitChannelStateSignalToDataPump(ChannelState state) const;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211MACCHANNELSTATE_H

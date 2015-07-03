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
#include "inet/linklayer/ieee80211/thenewmac/signals/Ieee80211MacSignals_m.h"
#include "inet/linklayer/ieee80211/thenewmac/reception/Ieee80211MacReception.h"
#include "inet/linklayer/ieee80211/thenewmac/base/Ieee80211MacMacProcessBase.h"
#include "inet/linklayer/ieee80211/thenewmac/macsorts/Ieee80211MacMacsorts.h"
#include "inet/linklayer/ieee80211/thenewmac/macmib/Ieee80211MacMacmib.h"
#include "inet/linklayer/ieee80211/thenewmac/macsorts/Ieee80211MacEnumeratedPhyTypes.h"

namespace inet {
namespace ieee80211 {

class INET_API IIeee80211MacChannelState
{
    protected:
        virtual void handleResetMac() = 0;
        virtual void handleTifs() = 0;
        virtual void handleTslot() = 0;
        virtual void handleTnav() = 0;
        virtual void handleRtsTimeout() = 0;
        virtual void handleClearNav(Ieee80211MacSignalClearNav *clearNav) = 0;
        virtual void handleChangeNav(Ieee80211MacSignalChangeNav *changeNav) = 0;
        virtual void handleUseEifs(Ieee80211MacSignalUseEifs *useEifs) = 0;
        virtual void handleUseDifs(Ieee80211MacSignalUseDifs *useDifs) = 0;
        virtual void handlePhyCcaIndication(Ieee80211MacSignalPhyCcaIndication *phyCcaIndication) = 0;
        virtual void handleSetNav(Ieee80211MacSignalSetNav *setNav) = 0;

        virtual void emitIdle() = 0;
        virtual void emitSlot() = 0;
        virtual void emitBusy() = 0;
        virtual void emitPhyCcarstRequest() = 0;
};

class INET_API Ieee80211MacChannelState : public IIeee80211MacChannelState, public Ieee80211MacMacProcessBase
{
    protected:
        enum ChannelStateState
        {
            CHANNEL_STATE_STATE_START,
            CHANNEL_STATE_STATE_BUSY,
            CHANNEL_STATE_STATE_IDLE,
            CHANNEL_STATE_STATE_SLOT,
            CHANNEL_STATE_STATE_CS_NO_NAV,
            CHANNEL_STATE_STATE_WAIT_IFS,
            CHANNEL_STATE_STATE_CS_NAV,
            CHANNEL_STATE_STATE_NO_CS_NO_NAV,
            CHANNEL_STATE_STATE_NO_CS_NAV
        };

    protected:
        ChannelStateState state = CHANNEL_STATE_STATE_START;
        simtime_t tNavEnd = SIMTIME_ZERO;
        cMessage *tIfs = nullptr;
        cMessage *tNav = nullptr;
        cMessage *tSlot = nullptr;

        Ieee80211MacMacmibPackage *macmib = nullptr;
        Ieee80211MacMacsorts *macsorts = nullptr;

        CcaStatus cs;
        int rxtx;
        int slot;
        int sifs;

        simtime_t dDifs = SIMTIME_ZERO;
        simtime_t dEifs = SIMTIME_ZERO;
        simtime_t dIfs = SIMTIME_ZERO;
        simtime_t dNav = SIMTIME_ZERO;
        simtime_t dRxTx = SIMTIME_ZERO;
        simtime_t dSifs = SIMTIME_ZERO;
        simtime_t dSlot = SIMTIME_ZERO;

        simtime_t tNew = SIMTIME_ZERO;
        simtime_t tRef = SIMTIME_ZERO;
        simtime_t tRxEnd = SIMTIME_ZERO;

        NavSrc newSrc;
        NavSrc curSrc;

    protected:
        void handleMessage(cMessage *msg) override;
        void initialize(int stage) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }

        virtual void handleTifs();
        virtual void handlePhyCcaIndication(Ieee80211MacSignalPhyCcaIndication *phyCcaIndication);
        virtual void handleSetNav(Ieee80211MacSignalSetNav *setNav);
        virtual void handleTslot();
        virtual void handleTnav();
        virtual void handleUseEifs(Ieee80211MacSignalUseEifs *useEifs);
        virtual void handleUseDifs(Ieee80211MacSignalUseDifs *useDifs);
        virtual void handleChangeNav(Ieee80211MacSignalChangeNav *changeNav);
        virtual void handleRtsTimeout();
        virtual void handleClearNav(Ieee80211MacSignalClearNav *clearNav);

        virtual void handleResetMac();
        virtual void emitIdle();
        virtual void emitSlot();
        virtual void emitBusy();
        virtual void emitPhyCcarstRequest();
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211MACCHANNELSTATE_H

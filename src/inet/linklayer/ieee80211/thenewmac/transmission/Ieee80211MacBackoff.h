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
#include "inet/linklayer/ieee80211/thenewmac/base/Ieee80211MacMacProcessBase.h"
#include "inet/linklayer/ieee80211/thenewmac/macsorts/Ieee80211MacMacsorts.h"
#include "inet/linklayer/ieee80211/thenewmac/signals/Ieee80211MacSignals_m.h"

namespace inet {
namespace ieee80211 {

class INET_API IIeee80211MacBackoff
{
    protected:
        virtual void handleResetMac() = 0;
        virtual void handleBackoff(Ieee80211MacSignalBackoff *backoff) = 0;
        virtual void handleIdle() = 0;
        virtual void handleBusy() = 0;
        virtual void handleSlot() = 0;
        virtual void handleCancel() = 0;

        virtual void emitBkDone(int slotCount) = 0;

    public:
        virtual void emitResetMac() = 0;
};

/* This process performs the Backoff Procedure (see 9.2.5.2),
 * returning Done(-1) when Tx may begin, or Done(n>=0) if cancelled.
 * Backoff(cw,-1) starts new random backoff. Backoff(x,n>=0) resumes
 * cancelled backoff. Backoff(0,0) sends Done(-1) when WM idle.
 */
class INET_API Ieee80211MacBackoff : public IIeee80211MacBackoff, public Ieee80211MacMacProcessBase
{
    public:
        enum BackoffState
        {
            BACKOFF_STATE_NO_BACKOFF,
            BACKOFF_STATE_CHANNEL_BUSY,
            BACKOFF_STATE_CHANNEL_IDLE
        };

    protected:
        BackoffState state = BACKOFF_STATE_NO_BACKOFF;
        Ieee80211MacMacsorts *macsorts = nullptr;

        int slotCnt = -1;
        int cw = -1;
        int cnt = -1;
        cGate *source = nullptr;

    protected:
        void handleMessage(cMessage *msg) override;
        void initialize(int stage) override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    protected:
        virtual void handleResetMac() override;
        virtual void handleBackoff(Ieee80211MacSignalBackoff *backoff);
        virtual void handleIdle() override;
        virtual void handleBusy() override;
        virtual void handleSlot() override;
        virtual void handleCancel() override;
        virtual void done();

        virtual void emitBkDone(int signalPar);

    public:
        virtual void emitResetMac() override { Ieee80211MacMacProcessBase::emitResetMac(); }
};

} /* namespace inet */
} /* namespace ieee80211 */

#endif // ifndef __INET_IEEE80211MACBACKOFF_H

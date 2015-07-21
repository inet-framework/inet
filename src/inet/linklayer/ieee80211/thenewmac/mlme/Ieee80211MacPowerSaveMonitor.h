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

#ifndef __INET_IEEE80211MACPOWERSAVEMONITOR_H
#define __INET_IEEE80211MACPOWERSAVEMONITOR_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/ieee80211/thenewmac/signals/Ieee80211MacSignals_m.h"
#include "inet/linklayer/ieee80211/thenewmac/macsorts/Ieee80211MacEnumeratedMacStaTypes.h"
#include "inet/linklayer/ieee80211/thenewmac/macmib/Ieee80211MacMacmib.h"
#include "inet/linklayer/ieee80211/thenewmac/base/Ieee80211MacMacProcessBase.h"
#include <set>

namespace inet {
namespace ieee80211 {

class INET_API IIeee80211MacPowerSaveMonitor
{
    protected:
        virtual void handlePsIndicate(Ieee80211MacSignalPsIndicate *psIndicate) = 0;
        virtual void handleStaState(Ieee80211MacSignalStaState *staState) = 0;
        virtual void handlePsInquiry(Ieee80211MacSignalPsInquiry *psInquiry) = 0;
        virtual void handleSsInquiry(Ieee80211MacSignalSsInquiry *ssInquiry) = 0;
        virtual void handleResetMac() = 0;

        virtual void emitPsChange(MACAddress sta, PsMode psMode) = 0;
        virtual void emitPsResponse(MACAddress sta, PsMode psMode) = 0;
        virtual void emitSsResponse(MACAddress sta, StationState sst, StationState asst) = 0;
};


class INET_API Ieee80211MacPowerSaveMonitor : public IIeee80211MacPowerSaveMonitor, public Ieee80211MacMacProcessBase
{
    protected:
        enum PowerSaveMonitorState
        {
            POWER_SAVE_MONITOR_STATE_START,
            POWER_SAVE_MONITOR_STATE_MONITOR_IDLE
        };
    /* Each of these sets holds MAC addresses of
    stations with a particular operational state.
    Stations are added to and removed from sets
    due to MLME requests, received management
    frames, and bits in received MAC headers.
    Sets are not aged, as there is no requirement
    for periodic activity, but aging to expunge
    addresses of inactive stations is permitted.
    */
    protected:
        PowerSaveMonitorState state = POWER_SAVE_MONITOR_STATE_START;

        std::set<MACAddress> awake, asleep, authOs, authKey, asoc;
        PsMode psm;
        bool psquery;
        StationState sst, asst;
        MACAddress sta;
        Ieee80211MacMacsorts *macsorts = nullptr;
        Ieee80211MacMacmibPackage *macmib = nullptr;

    protected:
        void handleMessage(cMessage *msg) override;
        void initialize(int stage) override;
        void processSignal(cMessage *msg);

    protected:
        void grpAddr();
        void function1(); // TODO: rename

        void handleResetMac() override;
        virtual void handlePsIndicate(Ieee80211MacSignalPsIndicate *psIndicate) override;
        virtual void handleStaState(Ieee80211MacSignalStaState *staState) override;
        virtual void handlePsInquiry(Ieee80211MacSignalPsInquiry *psInquiry) override;
        virtual void handleSsInquiry(Ieee80211MacSignalSsInquiry *ssInquiry) override;

        virtual void emitPsChange(MACAddress sta, PsMode psMode) override;
        virtual void emitPsResponse(MACAddress sta, PsMode psMode) override;
        virtual void emitSsResponse(MACAddress sta, StationState sst, StationState asst) override;
};

} /* namespace inet */
} /* namespace ieee80211 */

#endif // ifndef __INET_IEEE80211MACPOWERSAVEMONITOR_H

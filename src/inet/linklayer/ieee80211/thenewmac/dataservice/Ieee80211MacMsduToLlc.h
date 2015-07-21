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

#ifndef __INET_IEEE80211MACMSDUTOLLC_H
#define __INET_IEEE80211MACMSDUTOLLC_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/ieee80211/thenewmac/signals/Ieee80211MacSignals_m.h"
#include "inet/linklayer/ieee80211/thenewmac/base/Ieee80211MacMacProcessBase.h"

namespace inet {
namespace ieee80211 {

class INET_API IIeee80211MacMsduToLlc
{
    protected:
        virtual void handleMsduIndicate(Ieee80211MacSignalMsduIndicate *msduIndicate, Ieee80211NewFrame *frame) = 0;
        virtual void emitMaUnitDataIndication(MACAddress sa, MACAddress da, Routing routing, cPacket *llcData, RxStatus rxStat, CfPriority cf, ServiceClass srv) = 0;
};

class INET_API Ieee80211MacMsduToLlc : public IIeee80211MacMsduToLlc, public Ieee80211MacMacProcessBase
{
    protected:
        enum MsduToLlcState
        {
            MSDU_TO_LLC_STATE_START,
            MSDU_TO_LLC_STATE_TO_LLC,
        };

    protected:
        MsduToLlcState state = MSDU_TO_LLC_STATE_START;
        MACAddress da, sa;
        Ieee80211NewFrame *sdu = nullptr;
        cPacket *llcData = nullptr;
        ServiceClass srv;
        CfPriority cf;

    protected:
        void initialize(int stage) override;
        void processSignal(cMessage *msg);
        virtual void handleMsduIndicate(Ieee80211MacSignalMsduIndicate *msduIndicate, Ieee80211NewFrame *frame) override;
        virtual void emitMaUnitDataIndication(MACAddress sa, MACAddress da, Routing routing, cPacket *llcData, RxStatus rxStat, CfPriority cf, ServiceClass srv) override;
};

} /* namespace inet */
} /* namespace ieee80211 */

#endif // ifndef __INET_IEEE80211MACMSDUTOLLC_H

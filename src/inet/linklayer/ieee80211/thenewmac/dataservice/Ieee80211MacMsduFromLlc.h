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

#ifndef __INET_IEEE80211MACMSDUFROMLLC_H
#define __INET_IEEE80211MACMSDUFROMLLC_H

#include "inet/common/INETDefs.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ieee80211/thenewmac/macsorts/Ieee80211MacMacsorts.h"
#include "inet/linklayer/ieee80211/thenewmac/base/Ieee80211MacMacProcessBase.h"
#include "inet/linklayer/ieee80211/thenewmac/macmib/Ieee80211MacMacmib.h"
#include "inet/linklayer/ieee80211/thenewmac/signals/Ieee80211MacSignals_m.h"

namespace inet {
namespace ieee80211 {

class INET_API IIeee80211MacMsduFromLlc
{
    protected:
        virtual void handleMaUnitDataRequest(Ieee80211MacSignalMaUnitDataRequest *signal) = 0;
        virtual void handleMsduConfirm(Ieee80211MacSignalMsduConfirm *signal) = 0;
        virtual void emitMsduRequest(cPacket *sdu, CfPriority priority) = 0;
        virtual void emitMaUnitDataStatusIndication(MACAddress sa, MACAddress da, TxStatus stat, CfPriority cf, ServiceClass srv) = 0;
};

class INET_API Ieee80211MacMsduFromLlc : public IIeee80211MacMsduFromLlc, public Ieee80211MacMacProcessBase
{
    protected:
        enum MsduFromLlcState {
            MSDU_FROM_LCC_STATE_FROM_LLC
        };

    protected:
        MsduFromLlcState state = MSDU_FROM_LCC_STATE_FROM_LLC;

        Ieee80211MacMacsorts *macsorts = nullptr;
        Ieee80211MacMacmibPackage *macmib = nullptr;

        CfPriority cf;
        cPacket llcData;
        Routing rt;
        MACAddress da;
        MACAddress sa;
        TxStatus stat;
        ServiceClass srv;

    protected:
        void handleMessage(cMessage *msg) override;
        void initialize(int stage) override;

        void handleMaUnitDataRequest(Ieee80211MacSignalMaUnitDataRequest *signal);
        void emitMaUnitDataStatusIndication(MACAddress sa, MACAddress da, TxStatus stat, CfPriority cf, ServiceClass srv);
        void handleMsduConfirm(Ieee80211MacSignalMsduConfirm *signal);
        void makeMsdu();
        void emitMsduRequest(cPacket *sdu, CfPriority priority);
};

} /* namespace inet */
} /* namespace ieee80211 */

#endif // ifndef __INET_IEEE80211MACMSDUFROMLLC_H

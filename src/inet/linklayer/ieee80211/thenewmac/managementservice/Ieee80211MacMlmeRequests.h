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

#ifndef __INET_IEEE80211MACMLMEREQUESTS_H
#define __INET_IEEE80211MACMLMEREQUESTS_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/ieee80211/thenewmac/macsorts/Ieee80211MacMacsorts.h"
#include "inet/linklayer/ieee80211/thenewmac/base/Ieee80211MacMacProcessBase.h"

namespace inet {
namespace ieee80211 {

class INET_API IIeee80211MacMlmeRequests
{

};

class INET_API Ieee80211MacMlmeRequests : public IIeee80211MacMlmeRequests, public Ieee80211MacMacProcessBase
{
    protected:
        enum MlmeRequestsState
        {
            MLME_REQUESTS_STATE_IDLE,
        };

    protected:
        MlmeRequestsState state = MLME_REQUESTS_STATE_IDLE;
        Ieee80211MacMacsorts *macsorts = nullptr;

        AuthType alg;

        std::string bRate, oRate, ss;
        BssDscr bss;
//        dcl bssSet BssDscrSet ;
        Capability cap;
        CfParms cfpm;
//        dcl chlist Intstring ;
        int dtp, li;
        simtime_t dly;
//        dcl ibpm IbssParms ;
//        PhyParms phpm;
        PwrSave ps;
//        ReasonCode rs;
        ScanType scan;
        MACAddress sta, bid;
        MlmeStatus sts;
        simtime_t tBcn, tmax, tmin, tmot;
//        BssTypeSet typeSet;
        bool wake, rdtm;


    protected:
        void handleMessage(cMessage *msg) override;
        void initialize(int stage) override;

        void restart();
};
;
} /* namespace inet */
} /* namespace ieee80211 */

#endif // ifndef __INET_IEEE80211MACMLMEREQUESTS_H

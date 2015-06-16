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

#ifndef __INET_IEEE80211MACPREPAREMPDU_H
#define __INET_IEEE80211MACPREPAREMPDU_H

#include "inet/common/INETDefs.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ieee80211/thenewmac/macsorts/Ieee80211MacMacsorts.h"
#include "inet/linklayer/ieee80211/thenewmac/base/Ieee80211MacMacProcessBase.h"
#include "inet/linklayer/ieee80211/thenewmac/macmib/Ieee80211MacMacmib.h"
#include "inet/linklayer/ieee80211/thenewmac/signals/Ieee80211MacSignals_m.h"

namespace inet {
namespace ieee80211 {

class INET_API IIeee80211MacPrepareMpdu
{
    public:
        typedef Ieee80211MacFragmentationSupport::FragSdu FragSdu;

    protected:
        virtual void handleMsduRequest(Ieee80211MacSignalMsduRequest *msduRequest) = 0;
        virtual void handleResetMac() = 0;
        virtual void handleMmRequest(Ieee80211MacSignalMmRequest *mmRequest) = 0;
        virtual void handleFragConfirm(Ieee80211MacSignalFragConfirm *fragConfirm) = 0;

        virtual void emitMsduConfirm(cPacket *sdu, CfPriority priority, TxStatus txStatus) = 0;
        virtual void emitFragRequest(FragSdu *fsdu) = 0;
        virtual void emitMmConfirm(cPacket *rsdu, TxResult txResult) = 0;
};


class INET_API Ieee80211MacPrepareMpdu : public IIeee80211MacPrepareMpdu, public Ieee80211MacMacProcessBase
{
    protected:
        enum PrepareMpduState
        {
            PREPARE_MPDU_STATE_START,
            PREPARE_MPDU_STATE_NO_BSS,
            PREPARE_MPDU_STATE_PREPARE_BSS,
            PREPARE_MDPU_STATE_PREPARE_IBSS,
            PREPARE_MPDU_STATE_PREPARE_AP
        };

    protected:
        PrepareMpduState state = PREPARE_MPDU_STATE_START;
        Ieee80211MacMacmibPackage *macmib = nullptr;

        bool bcmc;
        bool keyOk;
        bool useWep = false;
        int f;
        FragSdu *fsdu = nullptr;
        int mpduOvhd;
        int pduSize;
        int thld;
        CfPriority pri;
        TxResult rrsl;
        cPacket *sdu;
        cPacket *rsdu;

        Ieee80211MacMacsorts *macsorts = nullptr;

        cGate *msdu = nullptr;
        cGate *fragMsdu = nullptr;

    protected:
        void handleMessage(cMessage *msg) override;
        void initialize(int stage) override;

        void fragment();
        void makePdus();

        void handleMsduRequest(Ieee80211MacSignalMsduRequest *msduRequest);
        void emitMsduConfirm(cPacket *sdu, CfPriority priority, TxStatus txStatus);
        void emitFragRequest(FragSdu *fsdu);
        void handleResetMac();
        void handleMmRequest(Ieee80211MacSignalMmRequest *mmRequest);
        void handleFragConfirm(Ieee80211MacSignalFragConfirm *fragConfirm);
        void emitMmConfirm(cPacket *rsdu, TxResult txResult);
};

} /* namespace inet */
} /* namespace ieee80211 */

#endif // ifndef __INET_IEEE80211MACPREPAREMPDU_H

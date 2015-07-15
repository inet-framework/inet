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
#include "inet/linklayer/ieee80211/thenewmac/Ieee80211NewFrame_m.h"

namespace inet {
namespace ieee80211 {

class INET_API IIeee80211MacPrepareMpdu
{
    public:
        typedef Ieee80211MacFragmentationSupport::FragSdu FragSdu;

    protected:
        virtual void handleMsduRequest(Ieee80211MacSignalMsduRequest *msduRequest, Ieee80211NewFrame *frame, cGate *sender) = 0;
        virtual void handleResetMac() = 0;
        virtual void handleMmRequest(Ieee80211MacSignalMmRequest *mmRequest, Ieee80211NewFrame *frame, cGate *sender) = 0;
        virtual void handleFragConfirm(Ieee80211MacSignalFragConfirm *fragConfirm, FragSdu *fsdu) = 0;

        virtual void emitMsduConfirm(cPacket *sdu, CfPriority priority, TxStatus txStatus) = 0;
        virtual void emitFragRequest(FragSdu *fsdu) = 0;
        virtual void emitMmConfirm(cPacket *rsdu, TxStatus txStatus) = 0;
};


class INET_API Ieee80211MacPrepareMpdu : public IIeee80211MacPrepareMpdu, public Ieee80211MacMacProcessBase, public cListener
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
        Ieee80211NewFrame *sdu = nullptr;
        Ieee80211NewFrame *rsdu = nullptr;

        Ieee80211MacMacsorts *macsorts = nullptr;

    protected:
        void processSignal(cMessage *msg);
        void initialize(int stage) override;

        void receiveSignal(cComponent *source, int signalID, bool b) override;

        void fragment(cGate *sender);
        void makePdus(int sduLength);

        void handleMsduRequest(Ieee80211MacSignalMsduRequest *msduRequest, Ieee80211NewFrame *frame, cGate *sender);
        void emitMsduConfirm(cPacket *sdu, CfPriority priority, TxStatus txStatus);
        void emitFragRequest(FragSdu *fsdu);
        void handleResetMac();
        void handleMmRequest(Ieee80211MacSignalMmRequest *mmRequest, Ieee80211NewFrame *frame, cGate *sender);
        void handleFragConfirm(Ieee80211MacSignalFragConfirm *fragConfirm, FragSdu *fsdu);
        void emitMmConfirm(cPacket *rsdu, TxStatus txStatus);

        void processNoBssContinuousSignal1();
        bool isNoBssContinuoisSignal1Enabled();
        void processNoBssContinuousSignal2();
        bool isNoBssContinuoisSignal2Enabled();
        void processNoBssContinuousSignal3();
        bool isNoBssContinuoisSignal3Enabled();
        void processPrepareBssContinuousSignal1();
        bool isPrepareBssContinousSignal1Enabled();
        void processPrepareIbssContinuousSignal1();
        bool isPrepareIbssContinousSignal1Enabled();
        void processPrepareApContinuousSignal1();
        bool isPrepareApContinousSignal1Enabled();
};

} /* namespace inet */
} /* namespace ieee80211 */

#endif // ifndef __INET_IEEE80211MACPREPAREMPDU_H

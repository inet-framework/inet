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

#ifndef __INET_IEEE80211MACPMFILTERSTA_H
#define __INET_IEEE80211MACPMFILTERSTA_H

#include <deque>
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ieee80211/thenewmac/signals/Ieee80211MacSignals_m.h"
#include "inet/linklayer/ieee80211/thenewmac/macsorts/Ieee80211MacMacsorts.h"
#include "inet/linklayer/ieee80211/thenewmac/base/Ieee80211MacMacProcessBase.h"
#include "inet/linklayer/ieee80211/thenewmac/macmib/Ieee80211MacMacmib.h"

namespace inet {
namespace ieee80211 {

class INET_API IIeee80211MacPmFilterSta
{
    public:
        typedef Ieee80211MacFragmentationSupport::FragSdu FragSdu;

    protected:
        virtual void handleResetMac() = 0;
        virtual void handleFragRequest(Ieee80211MacSignalFragRequest *fragRequest, FragSdu *fsdu) = 0;
        virtual void handlePduConfirm(Ieee80211MacSignalPduConfirm *pduConfirm) = 0;
        virtual void handleCfPolled(Ieee80211MacSignalCfPolled *cfPolled) = 0;
        virtual void handleAtimW(Ieee80211MacSignalAtimW *awtimW) = 0;
        virtual void handlePsChange(Ieee80211MacSignalPsChange *psChange) = 0;
        virtual void handlePsResponse(Ieee80211MacSignalPsResponse *psResponse) = 0;

        virtual void emitPduRequest(FragSdu *fsdu) = 0;
        virtual void emitFragConfirm(FragSdu *fsdu, TxResult txResult) = 0;
        virtual void emitPsInquiry(MACAddress dst) = 0;
};

class INET_API Ieee80211MacPmFilterSta : public IIeee80211MacPmFilterSta, public Ieee80211MacMacProcessBase, public cListener
{
    protected:
        enum PmFilterStaStates
        {
            PM_FILTER_STA_STATE_START,
            PM_FILTER_STA_STATE_PM_IDLE,
            PM_FILTER_STA_STATE_PM_BSS,
            PM_FILTER_STA_STATE_PM_IBSS_DATA,
            PM_FILTER_STA_STATE_BSS_CFP,
            PM_FILTER_STA_WAIT_PS_RESPONSE,
            PM_FILTER_STA_PRE_ATIM,
            PM_FILTER_STA_IBSS_ATIM_W
        };

    protected:
        PmFilterStaStates state = PM_FILTER_STA_STATE_START;
        Ieee80211MacMacmibPackage *macmib = nullptr;
        Ieee80211MacMacsorts *macsorts = nullptr;

        bool atPend = false;
        bool fsPend = false;
        bool sentBcn = false;

        std::deque<FragSdu *> cfQ, txQ, anQ, psQ;
        PsMode dpsm;
        FragSdu *fsdu = nullptr;
        FragSdu *rsdu = nullptr;
        int k = -1;
        int n = -1;
        TxResult resl;
        MACAddress sta;

    protected:
        void processSignal(cMessage *msg);
        void initialize(int stage) override;
        void receiveSignal(cComponent *source, int signalID, cObject *obj);

    protected:
        void handleResetMac() override;
        void handleFragRequest(Ieee80211MacSignalFragRequest *fragRequest, FragSdu *fsdu) override;
        void handlePduConfirm(Ieee80211MacSignalPduConfirm *pduConfirm) override;
        void handleCfPolled(Ieee80211MacSignalCfPolled *cfPolled) override;
        void handleAtimW(Ieee80211MacSignalAtimW *awtimW) override;
        void handlePsChange(Ieee80211MacSignalPsChange *psChange) override;
        void handlePsResponse(Ieee80211MacSignalPsResponse *psResponse) override;

        void emitPduRequest(FragSdu *fsdu) override;
        void emitFragConfirm(FragSdu *fsdu, TxResult txResult) override;
        void emitPsInquiry(MACAddress dst) override;

        void processPmIdleContinuousSignal1();
        bool isPmIdleContinuoisSignal1Enabled();
        void processPmIdleContinuousSignal2();
        bool isPmIdleContinuoisSignal2Enabled();
        void processPmBssContinuousSignal1();
        bool isPmBssContinuousSignal1Enabled();
        void processPmBssContinuousSignal2();
        bool isPmBssContinuousSignal2Enabled();
        void processBssCfpContinuousSignal1();
        bool isBssCfpContinuousSignal1Enabled();
        void processPmIbssDataContinuousSignal1();
        bool isPmIbssDataContinuousSignal1Enabled();
        void processPmIbssDataContinuousSignal2();
        bool isPmIbssDataContinuousSignal2Enabled();
        void processPmIbssDataContinuousSignal3();
        bool isPmIbssDataContinuousSignal3Enabled();
        void processPmIbssAtimWContinuousSignal1();
        bool isPmIbssAtimWContinuousSignal1Enabled();
        void processPmIbssAtimWContinuousSignal2();
        bool isPmIbssAtimWContinuousSignal2Enabled();
        void processAllStatesContinuousSignal1();
        bool isAllStatesContinuousSignal1Enabled();
};

} /* namespace inet */
} /* namespace ieee80211 */

#endif // ifndef __INET_IEEE80211MACPMFILTERSTA_H

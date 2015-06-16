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

#ifndef __INET_IEEE80211MACTXCOORDINATIONSTA_H
#define __INET_IEEE80211MACTXCOORDINATIONSTA_H

#include "inet/linklayer/ieee80211/thenewmac/base/Ieee80211MacMacProcessBase.h"
#include "inet/linklayer/ieee80211/thenewmac/macsorts/Ieee80211MacMacsorts.h"
#include "inet/linklayer/ieee80211/thenewmac/macsorts/Ieee80211MacEnumeratedMacStaTypes.h"
#include "inet/linklayer/ieee80211/thenewmac/macmib/Ieee80211MacMacmib.h"
#include "inet/linklayer/ieee80211/thenewmac/macsorts/Ieee80211MacFrameTypes.h"
#include "inet/linklayer/ieee80211/thenewmac/Ieee80211NewFrame_m.h"
#include "inet/linklayer/ieee80211/thenewmac/signals/Ieee80211MacSignals_m.h"

namespace inet {
namespace ieee80211 {

class INET_API IIeee80211MacTxCoordinationSta
{
    public:
        typedef Ieee80211MacFragmentationSupport::FragSdu FragSdu;

    protected:
        virtual void handleResetMac() = 0;
        virtual void handlePduRequest(Ieee80211MacSignalPduRequest *pduRequest) = 0;
        virtual void handleBkDone(Ieee80211MacSignalBkDone *bkDone) = 0;
        virtual void handleCfPoll(Ieee80211MacSignalCfPoll *cfPoll) = 0;
        virtual void handleCfEnd() = 0;
        virtual void handleTbtt() = 0;
        virtual void handleTxConfirm() = 0;
        virtual void handleTxCfAck(Ieee80211MacSignalTxCfAck *txCfAck) = 0;
        virtual void handleTifs() = 0;
        virtual void handleAck(Ieee80211MacSignalAck *ack) = 0;
        virtual void handleTrsp() = 0;
        virtual void handleCts(Ieee80211MacSignalCts *cts) = 0;
        virtual void handleMmCancel() = 0;
        virtual void handleDoze() = 0;
        virtual void handleWake() = 0;
        virtual void handleTpdly() = 0;
        virtual void handleSwChnl(Ieee80211MacSignalSwChnl *swChnl) = 0;

        virtual void emitBackoff(int ccw, int par2) = 0;
        virtual void emitAtimW() = 0;
        virtual void emitTxRequest(cPacket *tpdu, bps txrate) = 0;
        virtual void emitPduConfirm(FragSdu *fsdu, TxResult txResult) = 0;
        virtual void emitCancel() = 0;
        virtual void emitPsmDone() = 0;
        virtual void emitChangeNav(int par1, NavSrc navSrc) = 0;
        virtual void emitCfPolled() = 0;
};

class INET_API Ieee80211MacTxCoordinationSta : public IIeee80211MacTxCoordinationSta, public Ieee80211MacMacProcessBase
{
    protected:
        enum TxCoordinationState {
            TX_COORDINATION_STATE_START,
            TX_COORDINATION_STATE_TXC_IDLE,
            TX_COORDINATION_STATE_TXC_BACKOFF,
            TX_COORDINATION_STATE_ATW_START,
            TX_COORDINATION_STATE_ATIM_WINDOW,
            TX_COORDINATION_STATE_WAIT_SIFS,
            TX_COORDINATION_STATE_WAIT_RTS_BACKOFF,
            TX_COORDINATION_STATE_WAIT_MPDU_BACKOFF,
            TX_COORDINATION_STATE_WAIT_PDU_SENT,
            TX_COORDINATION_STATE_WAIT_ACK,
            TX_COORDINATION_STATE_WAIT_CTS,
            TX_COORDINATION_STATE_WAIT_CTS_BACKOFF,
            TX_COORDINATION_STATE_WAIT_CTS_SIFS,
            TX_COORDINATION_STATE_WAIT_CTS_SENT,
            TX_COORDINATION_STATE_IBSS_WAIT_ATIM,
            TX_COORDINATION_STATE_IBSS_WAIT_BEACON,
            TX_COORDINATION_STATE_WAIT_BEACON_TRANSMIT,
            TX_COORDINATION_STATE_WAIT_BEACON_CANCEL,
            TX_COORDINATION_STATE_WAIT_ATIM_ACK,
            TX_COORDINATION_STATE_ASLEEP,
            TX_COORDINATION_STATE_WAKE_WAIT_PROBE_DELAY,
            TX_COORDINATION_STATE_WAIT_CHANNEL,
            TX_COORDINATION_STATE_TXC_CFP, // TODO: not import mCfp
            TX_COORDINATION_STATE_CF_RESPONSE,
            TX_COORDINATION_STATE_WAIT_CFP_SIFS,
            TX_COORDINATION_STATE_WAIT_CFP_TX_DONE,
            TX_COORDINATION_STATE_WAIT_CF_ACK
        };

    protected:
        TxCoordinationState state = TX_COORDINATION_STATE_START;
        Ieee80211MacMacsorts *macsorts = nullptr;
        Ieee80211MacMacmibPackage *macmib = nullptr;

        int ackctstime = -1;
        int atimcw = -1;
        int bstat = -1;
        int chan = -1;
        int dcfcnt = -1;
        int dcfcw = -1;
        int frametime = -1;
        int frametime2 = -1;

        int ccw = -1;
        short int curPm = -1; // type Bit
        bool doHop = false;
        bool psmChg = false;
        bool cont = false;

        simtime_t dSifsDelay = SIMTIME_ZERO;
        simtime_t endRx = SIMTIME_ZERO;

        int seqnum = 0;
        int ssrc = 0;
        int slrc = 0;
        int n = 0;

        Ieee80211NewFrame *tpdu = nullptr;
        FragSdu *fsdu = nullptr;
        bps txrate = bps(NaN); // type Rate

        // TODO: Mmrate must be selected from mBrates. Other
        // selection criteria are not specified.
        bps mmrate = bps(NaN);

        cGate *backoffProcedureGate = nullptr;
        cGate *dataPumpProcedureGate = nullptr;
        cGate *tdatGate = nullptr;
        cGate *tmgtGate = nullptr;

        cMessage *tifs = nullptr;
        cMessage *trsp = nullptr;
        cMessage *tpdly = nullptr;

        /*
         * temporary definitions
         */
        int aCWMin;
        int aCWMax;

    protected:
        void handleMessage(cMessage *msg) override;
        void initialize(int stage) override;

        void init();
        void txcReq();
        void txcReq2();
        void rxPoll();
        void chkRtsCts();
        void function1();
        void txSifs(); // Send frame at Sifs
        void txWait();
        void sendFrag();
        void sendRts();
        void sendMpdu();
        void sendTxReq();
        void endFx();
        void confirmPdu();
        void ackFail();
        void ctsFail();
        void atimAck();
        void ctsFail2();
        void atimWBullshit();
        void ibssAtimWBullshit();
        void atimLimit();
        void atimFail();
        TypeSubtype ftype(cPacket *packet);

        void handleWake() override;
        void handleTbtt() override;
        void handleBkDone(Ieee80211MacSignalBkDone *bkDone) override;
        void handleResetMac() override;
        void handlePduRequest(Ieee80211MacSignalPduRequest *pduRequest) override;
        void handleCfPoll(Ieee80211MacSignalCfPoll *cfPoll) override;
        void handleTxCfAck(Ieee80211MacSignalTxCfAck *txCfAck) override;
        void handleTifs() override;
        void handleTxConfirm() override;
        void handleAck(Ieee80211MacSignalAck *ack) override;
        void handleTrsp() override;
        void handleCts(Ieee80211MacSignalCts *cts) override;
        void handleMmCancel() override;
        void handleDoze() override;
        void handleTpdly() override;
        void handleSwChnl(Ieee80211MacSignalSwChnl *swChnl) override;
        virtual void handleCfEnd() override;

        void emitTxRequest(cPacket *tpdu, bps txrate) override;
        void emitBackoff(int ccw, int par2) override;
        void emitAtimW() override;
        void emitPduConfirm(FragSdu *fsdu, TxResult txResult) override;
        void emitCancel() override;
        void emitPsmDone() override;
        void emitChangeNav(int par1, NavSrc navSrc) override;
        void emitCfPolled() override;

    public:
        virtual void emitResetMac() override { Ieee80211MacMacProcessBase::emitResetMac(); }
};

} /* namespace inet */
} /* namespace ieee80211 */

#endif // ifndef __INET_IEEE80211MACTXCOORDINATIONSTA_H

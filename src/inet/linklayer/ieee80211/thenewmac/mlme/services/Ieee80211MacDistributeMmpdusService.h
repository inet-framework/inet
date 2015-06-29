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

#ifndef __INET_IEEE80211MACDISTRIBUTEMMPDUSSERVICE_H
#define __INET_IEEE80211MACDISTRIBUTEMMPDUSSERVICE_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/ieee80211/thenewmac/signals/Ieee80211MacSignals_m.h"
#include "inet/linklayer/ieee80211/thenewmac/macsorts/Ieee80211MacMacsorts.h"

namespace inet {
namespace ieee80211 {

class INET_API IIeee80211MacDistributeMmpdusService
{
    protected:
        virtual void handleXport() = 0;
        virtual void handleSst(Ieee80211MacSignalSst *sst) = 0;
        virtual void handleSend(Ieee80211MacSignalSend *send, cPacket *frame) = 0;
        virtual void handleMmConfirm(Ieee80211MacSignalMmConfirm *mmConfirm, cPacket *frame) = 0;
        virtual void handleMmIndicate(Ieee80211MacSignalMmIndicate *mmIndicate) = 0;

        virtual void emitStaState(MACAddress addr, StationState stationState) = 0;
        virtual void emitMmRequest(cPacket *mspdu, CfPriority mim, bps mRate) = 0;
        virtual void emitSent(cPacket *mspdu, TxStatus txStatus) = 0;
        virtual void emitCls2Err(MACAddress addr) = 0;
        virtual void emitCls3Err(MACAddress addr) = 0;
        virtual void emitCfend() = 0;
        virtual void emitAsocReq(cPacket *mrpdu) = 0;
        virtual void emitAsocRsp(cPacket *mrpdu) = 0;
        virtual void emitReasocReq(cPacket *mrpdu) = 0;
        virtual void emitReasocRsp(cPacket *mrpdu) = 0;
};

class INET_API Ieee80211MacDistributeMmpdusService : public IIeee80211MacDistributeMmpdusService, public cSimpleModule
{
    protected:
        enum DistributeMmpdusState
        {
            DISTRIBUTE_MMPDUS_STATE_MMPDU_IDLE,
        };

    protected:
        DistributeMmpdusState state = DISTRIBUTE_MMPDUS_STATE_MMPDU_IDLE;

        MACAddress mAdr;
        Imed mIm;
        CfPriority pri;
        bps mRate;
        cPacket *mRpdu = nullptr;
        cPacket *mSpdu = nullptr;
        StateErr mSerr;
        StationState mSst;
        simtime_t mtE, mtT;
        TxStatus mTxstat;

    protected:
        void handleMessage(cMessage *msg) override;
        void initialize(int stage) override;

    protected:
        virtual void handleXport() override;
        virtual void handleSst(Ieee80211MacSignalSst *sst) override;
        virtual void handleSend(Ieee80211MacSignalSend *send, cPacket *frame) override;
        virtual void handleMmConfirm(Ieee80211MacSignalMmConfirm *mmConfirm, cPacket *frame) override;
        virtual void handleMmIndicate(Ieee80211MacSignalMmIndicate *mmIndicate) override;

        virtual void emitStaState(MACAddress addr, StationState stationState);
        virtual void emitMmRequest(cPacket *mspdu, CfPriority mim, bps mRate);
        virtual void emitSent(cPacket *mspdu, TxStatus txStatus);
        virtual void emitCls2Err(MACAddress addr);
        virtual void emitCls3Err(MACAddress addr);

        void reExport();
        void chkSigtype();
};

} /* namespace inet */
} /* namespace ieee80211 */

#endif // ifndef __INET_IEEE80211MACDISTRIBUTEMMPDUSSERVICE_H

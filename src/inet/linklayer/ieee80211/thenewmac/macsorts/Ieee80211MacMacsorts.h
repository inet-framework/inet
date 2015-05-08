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

#ifndef __INET_IEEE80211MACMACSORTS_H
#define __INET_IEEE80211MACMACSORTS_H

#include "inet/common/INETDefs.h"
#include "inet/common/INETMath.h"
#include "inet/common/Units.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/linklayer/ieee80211/thenewmac/macsorts/Ieee80211MacEnumeratedMacStaTypes.h"
#include <vector>

using namespace inet::units::values;

namespace inet {
namespace ieee80211 {

/*
 * Intra-MAC remote variables (names of form mXYZ)
 */
class Ieee80211MacMacsortsIntraMacRemoteVariables
{
    protected:
        bool mActingAsAp; /* =true if STA started BSS */
        int mAId; /* AID assigned to STA by AP */
        bool mAssoc; /* =true if STA associated w/BSS */
        bool mAtimW; /* =true if ATIM window in prog */
        bool mBkIP; /* =true if backoff in prog */
        // TODO: mBrates; /* basic rate set for this sta */
        MACAddress mBssId; /* identifier of current (I)BSS */
        std::string mCap; /* capability info from MlmeJoin */
        bool mCfp; /* =true if CF period in progress */
        bool mDisable; /* =true if not in any BSS; then */
        /* TX only sends probe_req; RX only accepts beacon, probe_rsp */
        int mDtimCount; /* =0 at Tbtt of Beacon with DTIM */
        bool mFxIP; /* =true during frame exchange seq */
        bool mIbss; /* =true if STA is member of IBSS */
        int mListenInt; /* beacons between wake up @TBTT */
        simtime_t mNavEnd; /* NAV end Time, <=now when idle */
        simtime_t mNextBdry; /* next boundary Time; =0 if none */
        simtime_t mNextTbtt; /* Time next beacon due to occur */
        bool mPcAvail; /* =true if point coord in BSS */
        bool mPcDlvr; /* =true if CF delivery only */
        bool mPcPoll; /* =true if CF delivery & polling */
        simtime_t mPdly; /* probe delay from start or join */
        // TODO: remote mPss PsState; /* power save state of STA */
        bool mReceiveDTIMs; /* =true if DTIMs received */
        bool mRxA; /* =true if RX indicated by PHY */
        std::string mSsId; /* name of the current (I)BSS */
        // TIDI? remote procedure TSF; /* read & update 64-bit TSF timer */

    public:
        bool isActingAsAp() const { return mActingAsAp; }
        void setActingAsAp(bool actingAsAp) { mActingAsAp = actingAsAp; }
        int getAId() const { return mAId; }
        void setAId(int aId) { mAId = aId; }
        bool isAssoc() const { return mAssoc; }
        void setAssoc(bool assoc) { mAssoc = assoc; }
        bool isAtimW() const { return mAtimW; }
        void setAtimW(bool atimW) { mAtimW = atimW; }
        bool isBkIp() const { return mBkIP; }
        void setBkIp(bool bkIp) { mBkIP = bkIp; }
        const MACAddress& getBssId() const { return mBssId; }
        void setBssId(const MACAddress& bssId) { mBssId = bssId; }
        const std::string& getCap() const { return mCap; }
        void setCap(const std::string& cap) { mCap = cap; }
        bool isCfp() const { return mCfp; }
        void setCfp(bool cfp) { mCfp = cfp; }
        bool isDisable() const { return mDisable; }
        void setDisable(bool disable) { mDisable = disable; }
        int getDtimCount() const { return mDtimCount; }
        void setDtimCount(int dtimCount) { mDtimCount = dtimCount; }
        bool isFxIp() const { return mFxIP; }
        void setFxIp(bool fxIp) { mFxIP = fxIp; }
        bool isIbss() const { return mIbss; }
        void setIbss(bool ibss) { mIbss = ibss; }
        int getListenInt() const { return mListenInt; }
        void setListenInt(int listenInt) { mListenInt = listenInt; }
        const simtime_t& getNavEnd() const { return mNavEnd; }
        void setNavEnd(const simtime_t& navEnd) { mNavEnd = navEnd; }
        const simtime_t& getNextBdry() const { return mNextBdry; }
        void setNextBdry(const simtime_t& nextBdry) { mNextBdry = nextBdry; }
        const simtime_t& getNextTbtt() const { return mNextTbtt; }
        void setNextTbtt(const simtime_t& nextTbtt) { mNextTbtt = nextTbtt; }
        bool isPcAvail() const { return mPcAvail; }
        void setPcAvail(bool pcAvail) { mPcAvail = pcAvail; }
        bool isPcDlvr() const { return mPcDlvr; }
        void setPcDlvr(bool pcDlvr) { mPcDlvr = pcDlvr; }
        bool isPcPoll() const { return mPcPoll; }
        void setPcPoll(bool pcPoll) { mPcPoll = pcPoll; }
        const simtime_t& getPdly() const { return mPdly; }
        void setPdly(const simtime_t& pdly) { mPdly = pdly; }
        bool isReceiveDtiMs() const { return mReceiveDTIMs; }
        void setReceiveDtiMs(bool receiveDtiMs) { mReceiveDTIMs = receiveDtiMs; }
        bool isRxA() const { return mRxA; }
        void setRxA(bool rxA) { mRxA = rxA; }
        const std::string& getSsId() const { return mSsId; }
        void setSsId(const std::string& ssId) { mSsId = ssId; }
};

/*
 * Named static int data values (names of form sXYZ)
 */
class Ieee80211MacNamedStaticIntDataValues
{
    public:
        static int sMaxMsduLng;
        static int sMacHdrLng;
        static int sWepHdrLng;
        static int sWepAddLng;
        static int sWdsAddLng;
        static int sCrcLng;
        static int sMaxMpduLng;
        // syntype FrameIndexRange = constants 0 : sMaxMpduLng endsyntype FrameIndexRange; /* index range for octets in MPDU */
        static int sTsOctet;
        static int sMinFragLng;
        static int sMaxFragNum;
        static int sAckCtsLng;
};

/*
 * Fragmentation support sorts
 */
class Ieee80211MacFragmentationSupport
{
    public:
        /* Array to hold up to FragNum fragments of an Msdu/Mmpdu */
        typedef std::vector<cPacket *> FragArray;
        /* FragSdu structure is for OUTGOING MSDUs/MMPDUs (called SDUs) */
        /* Each SDU, even if not fragmented, is held in an instance of */
        /* this structure awaiting its (re)transmission attempt(s). */
        /* Transmit queue(s) are ordered lists of FragSdu instances. */
        struct FragSdu
        {
            int fTot; /* number of fragments in pdus FragArray */
            int fCur; /* next fragment number to send */
            int fAnc; /* next fragment to announce in ATIM or TIM when fAnc > fCur, pdus(fCur)+ may be sent */
            simtime_t eol; /* set to (now + dUsec(aMaxTxMsduLifetime)) when the entry is created */
            int sqf; /* SDU sequence number, set at 1st Tx attempt */
            int src; /* short retry counter for this SDU */
            int lrc; /* long retry counter for this SDU */
            MACAddress dst; /* destinaton address */
            bool grpa; /* =true if RA (not DA) is a group address */
            bool psm; /* =true if RA (not DA) may be in pwr_save */
            bool resume; /* =true if fragment burst being resumed */
            cGate *cnfTo; /* address to which confirmation is sent */
            bps rate; /* data rate used for initial fragment */
            CfPriority cf; /* requested priority (from LLC) */
            FragArray pdus; /* array of Frame to hold fragments */
        };
};

class INET_API Ieee80211MacMacsorts : public cSimpleModule
{
    protected:
        Ieee80211MacMacsortsIntraMacRemoteVariables *intraMacRemoteVariables;

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        void handleMessage(cMessage *msg) override;
        void initialize(int stage) override;

    public:
        Ieee80211MacMacsortsIntraMacRemoteVariables* getIntraMacRemoteVariables() const { return intraMacRemoteVariables; }
};

} /* namespace inet */
} /* namespace ieee80211 */

#endif // ifndef __INET_IEEE80211MACMACSORTS_H

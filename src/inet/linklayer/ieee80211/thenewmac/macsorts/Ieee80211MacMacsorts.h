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
#include "inet/linklayer/ieee80211/thenewmac/Ieee80211NewFrame_m.h"
#include <vector>

using namespace inet::units::values;

namespace inet {
namespace ieee80211 {

/*
 * Discrete microsecond and Time Unit sorts
 */
typedef int Usec;
typedef int Tu; // Time Unit -- 1*TU = 1024*Usec
static simtime_t usecToSimtime(Usec usec) { return simtime_t(usec * 1E-6); }
static simtime_t tuToSimtime(Tu tu)  { return simtime_t(1024 * tu * 1E-6); }

class Ieee80211MacMacsortsIntraMacRemoteVariables;

class INET_API Ieee80211MacMacsorts : public cSimpleModule
{
    public:
        static simsignal_t intraMacRemoteVariablesChanged;

    protected:
        Ieee80211MacMacsortsIntraMacRemoteVariables *intraMacRemoteVariables = nullptr;

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        void handleMessage(cMessage *msg) override;
        void initialize(int stage) override;

    public:
        void emitIntraMacRemoteVariablesChangedSignal();
        Ieee80211MacMacsortsIntraMacRemoteVariables* getIntraMacRemoteVariables() const { return intraMacRemoteVariables; }
};

/*
 * Intra-MAC remote variables (names of form mXYZ)
 */
class Ieee80211MacMacsortsIntraMacRemoteVariables
{
    protected:
        Ieee80211MacMacsorts *macsortsModule = nullptr;

    protected:
        bool mActingAsAp; /* =true if STA started BSS */
        int mAId; /* AID assigned to STA by AP */
        bool mAssoc = true; /* =true if STA associated w/BSS */ // TODO: hack
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
        bool mIbss = true; /* =true if STA is member of IBSS */ // TODO: hack
        int mListenInt; /* beacons between wake up @TBTT */
        simtime_t mNavEnd; /* NAV end Time, <=now when idle */
        simtime_t mNextBdry; /* next boundary Time; =0 if none */
        simtime_t mNextTbtt; /* Time next beacon due to occur */
        bool mPcAvail; /* =true if point coord in BSS */
        bool mPcDlvr; /* =true if CF delivery only */
        bool mPcPoll; /* =true if CF delivery & polling */
        Usec mPdly; /* probe delay from start or join */
        // TODO: remote mPss PsState; /* power save state of STA */
        bool mReceiveDTIMs; /* =true if DTIMs received */
        bool mRxA; /* =true if RX indicated by PHY */
        std::string mSsId; /* name of the current (I)BSS */
        // TIDI? remote procedure TSF; /* read & update 64-bit TSF timer */

    public:
        Ieee80211MacMacsortsIntraMacRemoteVariables(Ieee80211MacMacsorts *macsortsModule) :
            macsortsModule(macsortsModule) {}

        bool isActingAsAp() const { return mActingAsAp; }
        void setActingAsAp(bool actingAsAp) { mActingAsAp = actingAsAp; macsortsModule->emitIntraMacRemoteVariablesChangedSignal(); }
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
        void setIbss(bool ibss) { mIbss = ibss; macsortsModule->emitIntraMacRemoteVariablesChangedSignal(); }
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
        Usec getPdly() const { return mPdly; }
        void setPdly(const Usec pdly) { mPdly = pdly; }
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
        typedef std::vector<Ieee80211NewFrame *> FragArray;
        /* FragSdu structure is for OUTGOING MSDUs/MMPDUs (called SDUs) */
        /* Each SDU, even if not fragmented, is held in an instance of */
        /* this structure awaiting its (re)transmission attempt(s). */
        /* Transmit queue(s) are ordered lists of FragSdu instances. */
        class FragSdu : public cPacket
        {
            public:
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

//newtype CfParms inherits Octetstring operators all;
//adding operators
//cfpCount : CfParms -> Integer; /* CfpCount field (1) */
//setCfpCount : CfParms, Integer -> CfParms;
//cfpPeriod : CfParms -> Integer; /* CfpPeriod field (1) */
//setCfpPeriod : CfParms, Integer -> CfParms;
//cfpMaxDur : CfParms -> TU; /* CfpMaxDuration field (2) */
//setCfpMaxDur : CfParms, TU -> CfParms;
//cfpDurRem : CfParms -> TU; /* CfpDurRemaining field (2) */
//setCfpDurRem : CfParms, TU -> CfParms;
//axioms for all cf in CfParms( for all i in Integer( for all u in TU(
//cfpCount(cf) == octetVal(cf(0));
//setCfpCount(cf, i) == mkOS(i, 1) // Tail(cf);
//cfpPeriod(cf) == octetVal(cf(1));
//setCfpPeriod(cf, i) == cf(0) // mkOS(i, 1) // SubStr(cf,2,4);
//cfpMaxDur(cf) == octetVal(cf(2)) + (octetVal(cf(3)) * 256);
//setCfpMaxDur(cf, u) == SubStr(cf, 0, 2) // mkOS(u mod 256, 1)
//// mkOS(u / 256, 1) // SubStr(cf, 4, 2);
//cfpDurRem(cf) == octetVal(cf(4)) + (octetVal(cf(5)) * 256);
//setCfpDurRem(cf, u) == SubStr(cf, 0, 4) // mkOS(u mod 256, 1)
//// mkOS(u / 256, 1); )));
//endnewtype CfParms;
struct CfParms
{

};

struct BssDscr
{
    MACAddress bdBssId;
    std::string bdSsId; /* 1 <= length <= 32 */
    BssType bdType;
    Tu bdBcnPer; /* beacon period in Time Units */
    int bdDtimPer; /* DTIM period in beacon periods */
    std::string bdTstamp; /* 8 Octets from ProbeRsp/Beacon */
    std::string bdStartTs; /* 8 Octets TSF when rx Tstamp */
    std::string bdPhyParms; /* empty if not needed by PHY */
    CfParms bdCfParms; /* empty if not CfPollable/no PCF */
//    IbssParms bdIbssParms; /* empty if infrastructure BSS */
    Capability bdCap; /* capability information */
//    Ratestring bdBrates; /* BSS basic rate set */
};

} /* namespace inet */
} /* namespace ieee80211 */

#endif // ifndef __INET_IEEE80211MACMACSORTS_H

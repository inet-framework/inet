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

cplusplus {{
    #include "inet/common/INETDefs.h"
    #include "inet/linklayer/ieee80211/thenewmac/macsorts/Ieee80211MacMacsorts.h"
    #include "inet/linklayer/ieee80211/thenewmac/macsorts/Ieee80211MacEnumeratedMacStaTypes.h"
    #include "inet/linklayer/ieee80211/thenewmac/macsorts/Ieee80211MacEnumeratedPhyTypes.h"
    #include "inet/linklayer/ieee80211/thenewmac/Ieee80211NewFrame_m.h"
}}

namespace inet::ieee80211;

class noncobject Ieee80211MacFragmentationSupport::FragSdu;
class noncobject TxResult;
class noncobject NavSrc;
class noncobject CfPriority;
class noncobject Routing;
class noncobject ServiceClass;
class noncobject TxStatus;
class noncobject cPacket;
class noncobject MACAddress;
class noncobject PsMode;
class noncobject StationState;
class noncobject StateErr;
class noncobject Imed;
class noncobject MibStatus;
class noncobject Ieee80211NewFrame;
class noncobject MlmeStatus;
class noncobject CcaStatus;
class noncobject NavSrc;
class noncobject RxStatus;

enum Ieee80211MacSignalType
{
    TX_REQUEST = 0;
    BACKOFF = 1;
    PDU_CONFIRM = 2;
    CANCEL = 3;
    CHANGE_NAV = 4;
    BKDONE = 5;
    MA_UNIT_DATA_REQUEST = 6;
    MSDU_CONFIRM = 7;
    FRAG_REQUEST = 8;
    PDU_REQUEST = 9;
    FRAG_CONFIRM = 10;
    CF_POLLED = 11;
    PS_INQUIRY = 12;
    ATIM_W = 13;
    PS_CHANGE = 14;
    PS_RESPONSE = 15;
    CF_POLL = 16;
    TBTT = 17;
    CF_END = 18;
    TX_CONFIRM = 19;
    TX_CF_ACK = 20;
    TIFS = 21;
    IDLE = 22;
    BUSY = 23;
    SLOT = 24;
    PHY_TX_START_REQUEST = 25;
    PHY_TX_START_CONFIRM = 26;
    PHY_DATA_CONFIRM = 27;
    PHY_TX_END_CONFIRM = 28;
    ACK = 29;
    CTS = 30;
    MM_CANCEL = 31;
    DOZE = 32;
    WAKE = 33;
    TPDLY = 34;
    SW_CHNL = 35;
    MSDU_REQUEST = 36;
    MM_REQUEST = 37;
    XPORT = 38;
    SST = 39;
    SEND = 40;
    MM_CONFIRM = 41;
    MM_INDICATE = 42;
    MLME_GET_REQUEST = 43;
    MLME_SET_REQUEST = 44;
    MLME_GET_CONFIRM = 45;
    MLME_SET_CONFIRM = 46;
    MLME_RESET_REQUEST = 47;
    SW_DONE = 48;
    PLME_SET_CONFIRM = 49;
    PLME_SET_REQUEST = 50;
    MA_UNIT_DATA_STATUS_INDICATION = 51;
    MLME_RESET_CONFIRM = 52;
    PHY_CCA_INDICATION = 53;
    SET_NAV = 54;
    USE_EIFS = 55;
    USE_DIFS = 56;
    RTS_TIMEOUT = 57;
    CLEAR_NAV = 58;
    PHY_CCARST_REQUEST = 59;
    TRSP = 60;
    PS_INDICATE = 61;
    STA_STATE = 62;
    SS_INQUIRY = 63;
    SS_RESPONSE = 64;
    MSDU_INDICATE = 65;
    MA_UNIT_DATA_INDICATION = 66;
    TRTS = 67;
    PHY_RX_START_INDICATION = 68;
}

class Ieee80211MacSignal
{
    short signalType @enum(Ieee80211MacSignalType);
}

class Ieee80211MacSignalTxRequest extends Ieee80211MacSignal
{
    // tpdu as encapsulated class
    signalType = 0;
    double txrate;
}

class Ieee80211MacSignalBackoff extends Ieee80211MacSignal
{
    signalType = 1;
    int cw;
    int cnt;
}

class Ieee80211MacSignalPduConfirm extends Ieee80211MacSignal
{
    signalType = 2;
    TxResult txResult;
}

class Ieee80211MacSignalCancel extends Ieee80211MacSignal
{
    signalType = 3;
}

class Ieee80211MacSignalChangeNav extends Ieee80211MacSignal
{
    signalType = 4;
    simtime_t time;
    simtime_t duration;
    NavSrc navSrc;
}

class Ieee80211MacSignalBkDone extends Ieee80211MacSignal
{
    signalType = 5;
    int slotCount;
}

class Ieee80211MacSignalMaUnitDataRequest extends Ieee80211MacSignal
{
    signalType = 6;
    MACAddress senderAddres; // sa?
    MACAddress destinationAddress; // da
    Routing routing;
//    cPacket llcData;
    CfPriority priority;
    ServiceClass serviceClass;
}

class Ieee80211MacSignalMsduConfirm extends Ieee80211MacSignal
{
    signalType = 7;
//    cPacket sdu;
    CfPriority priority;
    TxStatus txStatus;
}

class Ieee80211MacSignalFragRequest extends Ieee80211MacSignal
{
    signalType = 8;
//    Ieee80211MacFragmentationSupport::FragSdu fsdu;
}

class Ieee80211MacSignalPduRequest extends Ieee80211MacSignal
{
    signalType = 9;
//    Ieee80211MacFragmentationSupport::FragSdu fsdu;
}

class Ieee80211MacSignalFragConfirm extends Ieee80211MacSignal
{
    signalType = 10;
//    Ieee80211MacFragmentationSupport::FragSdu fsdu;
    TxResult txResult;
    CfPriority priority;
}

class Ieee80211MacSignalCfPolled extends Ieee80211MacSignal
{
    signalType = 11;
}

class Ieee80211MacSignalPsInquiry extends Ieee80211MacSignal
{
    signalType = 12;
    MACAddress macAddress;
}

class Ieee80211MacSignalAtimW extends Ieee80211MacSignal
{
    signalType = 13;
}

class Ieee80211MacSignalPsChange extends Ieee80211MacSignal
{
    signalType = 14;
    MACAddress addr;
    PsMode psMode;
}

class Ieee80211MacSignalPsResponse extends Ieee80211MacSignal
{
    signalType = 15;
    MACAddress adr;
    PsMode psMode;
}

class Ieee80211MacSignalCfPoll extends Ieee80211MacSignal
{
    signalType = 16;
    simtime_t time;
    double rate;
}

class Ieee80211MacSignalTbtt extends Ieee80211MacSignal
{
    signalType = 17;
}

class Ieee80211MacSignalCfEnd extends Ieee80211MacSignal
{
    signalType = 18;
}

class Ieee80211MacSignalTxConfirm extends Ieee80211MacSignal
{
    signalType = 19;
}

class Ieee80211MacSignalTxCfAck extends Ieee80211MacSignal
{
    signalType = 20;
    simtime_t time;
    double rate;
}

class Ieee80211MacSignalTifs extends Ieee80211MacSignal
{
    signalType = 21;
}

class Ieee80211MacSignalIdle extends Ieee80211MacSignal
{
    signalType = 22;
}

class Ieee80211MacSignalBusy extends Ieee80211MacSignal
{
    signalType = 23;
}

class Ieee80211MacSignalSlot extends Ieee80211MacSignal
{
    signalType = 24;
}

class Ieee80211MacSignalPhyTxStartRequest extends Ieee80211MacSignal
{
    signalType = 25;
}

class Ieee80211MacSignalPhyTxStartConfirm extends Ieee80211MacSignal
{
    signalType = 26;
}

class Ieee80211MacSignalPhyDataConfirm extends Ieee80211MacSignal
{
    signalType = 27;
}

class Ieee80211MacSignalPhyTxEndConfirm extends Ieee80211MacSignal
{
    signalType = 28;
}

class Ieee80211MacSignalAck extends Ieee80211MacSignal
{
    signalType = 29;
    simtime_t endRx;
    double txRate;
}

class Ieee80211MacSignalCts extends Ieee80211MacSignal
{
    signalType = 30;
    simtime_t endRx;
    double txRate;
}

class Ieee80211MacSignalMmCancel extends Ieee80211MacSignal
{
    signalType = 31;
}

class Ieee80211MacSignalDoze extends Ieee80211MacSignal
{
    signalType = 32;
}

class Ieee80211MacSignalWake extends Ieee80211MacSignal
{
    signalType = 33;
}

class Ieee80211MacSignalTpdly extends Ieee80211MacSignal
{
    signalType = 34;
}

class Ieee80211MacSignalSwChnl extends Ieee80211MacSignal
{
    signalType = 35;
    int chan;
    bool doBkoff;
}

class Ieee80211MacSignalMsduRequest extends Ieee80211MacSignal
{
    signalType = 36;
//    Ieee80211NewFrame sdu;
    CfPriority priority;
}


class Ieee80211MacSignalMmRequest extends Ieee80211MacSignal
{
    signalType = 37;
//    Ieee80211NewFrame sdu;
    Imed imed;
    CfPriority priority; // TODO??
    double rate;
}

class Ieee80211MacSignalXport extends Ieee80211MacSignal
{
    signalType = 38;
}

class Ieee80211MacSignalSst extends Ieee80211MacSignal
{
    signalType = 39;
    MACAddress addr;
    StationState stationState;
}

class Ieee80211MacSignalSend extends Ieee80211MacSignal
{
    signalType = 40;
//    cPacket frame;
    Imed priority; // ???
}

class Ieee80211MacSignalMmConfirm extends Ieee80211MacSignal
{
    signalType = 41;
//    cPacket frame;
    TxStatus status;
}

class Ieee80211MacSignalMmIndicate extends Ieee80211MacSignal
{
    signalType = 42;
//    cPacket frame;
    simtime_t time1;
    simtime_t time2;
    StateErr stateErr;
}

class Ieee80211MacSignalMlmeGetRequest extends Ieee80211MacSignal
{
    signalType = 43;
    string mibAtrib;
}

class Ieee80211MacSignalMlmeSetRequest extends Ieee80211MacSignal
{
    signalType = 44;
    string mibAtrib;
    int mibValue;
}

class Ieee80211MacSignalMlmeGetConfirm extends Ieee80211MacSignal
{
    signalType = 45;
    MibStatus mibStatus;
    string mibAtrib;
    int mibValue;
}

class Ieee80211MacSignalMlmeSetConfirm extends Ieee80211MacSignal
{
    signalType = 46;
    MibStatus mibStatus;
    string mibAtrib;
}

class Ieee80211MacSignalMlmeResetRequest extends Ieee80211MacSignal
{
    signalType = 47;
    MACAddress addr;
    bool dflt;
}

class Ieee80211MacSignalSwDone extends Ieee80211MacSignal
{
    signalType = 48;
}

class Ieee80211MacSignalPlmeSetConfirm extends Ieee80211MacSignal
{
    signalType = 49;
    MibStatus status;
    string attrib; // MibAtrib
}


class Ieee80211MacSignalPlmeSetRequest extends Ieee80211MacSignal
{
    signalType = 50;
    string atrrib; // MibAtrib
    int mibValue; // MibValue
}

class Ieee80211MacSignalMaUnitDataStatusIndication extends Ieee80211MacSignal
{
    signalType = 51;
    MACAddress sa;
    MACAddress da;
    TxStatus stat; 
    CfPriority priority; 
    ServiceClass srv;
}

class Ieee80211MacSignalMlmeResetConfirm extends Ieee80211MacSignal
{
    signalType = 52;
    MlmeStatus mlmeStatus;
}

class Ieee80211MacSignalPhyCcaIndication extends Ieee80211MacSignal
{
    signalType = 53;
    CcaStatus status;
}

class Ieee80211MacSignalSetNav extends Ieee80211MacSignal
{
    signalType = 54;
    simtime_t time;
    simtime_t duration;
    NavSrc navSrc;
}

class Ieee80211MacSignalUseEifs extends Ieee80211MacSignal
{
    signalType = 55;
    simtime_t time;
}

class Ieee80211MacSignalUseDifs extends Ieee80211MacSignal
{
    signalType = 56;
    simtime_t time;
}

class Ieee80211MacSignalRtsTimeout extends Ieee80211MacSignal
{
    signalType = 57;
}

class Ieee80211MacSignalClearNav extends Ieee80211MacSignal
{
    signalType = 58;
    NavSrc navSrc;
}
class Ieee80211MacSignalPhyCcarstRequest extends Ieee80211MacSignal
{
    signalType = 59;
}

class Ieee80211MacSignalTrsp extends Ieee80211MacSignal
{
    signalType = 60;
}

class Ieee80211MacSignalPsIndicate extends Ieee80211MacSignal
{
    signalType = 61;
    MACAddress sta;
    PsMode psm;
}

class Ieee80211MacSignalStaState extends Ieee80211MacSignal
{
    signalType = 62;
    MACAddress sta;
    StationState sst;
}

class Ieee80211MacSignalSsInquiry extends Ieee80211MacSignal
{
    signalType = 63;
    MACAddress sta;
}

class Ieee80211MacSignalSsResponse extends Ieee80211MacSignal
{
    signalType = 64;
    MACAddress sta;
    StationState sst;
    StationState asst;
}

class Ieee80211MacSignalMsduIndicate extends Ieee80211MacSignal
{
    signalType = 65;
    CfPriority cf;
}

class Ieee80211MacSignalMaUnitDataIndication extends Ieee80211MacSignal
{
    signalType = 66;
    MACAddress sa;
    MACAddress da;
    Routing routing;
    RxStatus rxStat;
    CfPriority cf;
    ServiceClass srv;
}

class Ieee80211MacSignalTrts extends Ieee80211MacSignal
{
    signalType = 67;
}

class Ieee80211MacSignalPhyRxStartIndication extends Ieee80211MacSignal
{
    signalType = 68;
    int rxLength;
    double rxRate;
}

class Ieee80211MacSignalPhyRxDataIndication extends Ieee80211MacSignal
{
    signalType = 68;
    int rxLength;
    double rxRate;
}

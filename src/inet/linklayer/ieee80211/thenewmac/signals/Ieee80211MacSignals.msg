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
}

packet Ieee80211MacSignal
{
    short signalType @enum(Ieee80211MacSignalType);
}

packet Ieee80211MacSignalTxRequest extends Ieee80211MacSignal
{
    // tpdu as encapsulated packet
    signalType = 0;
    double txrate;
}

packet Ieee80211MacSignalBackoff extends Ieee80211MacSignal
{
    signalType = 1;
    int cw;
    int cnt;
}

packet Ieee80211MacSignalPduConfirm extends Ieee80211MacSignal
{
    signalType = 2;
    Ieee80211MacFragmentationSupport::FragSdu fsdu;
    TxResult txResult;
}

packet Ieee80211MacSignalCancel extends Ieee80211MacSignal
{
    signalType = 3;
}

packet Ieee80211MacSignalChangeNav extends Ieee80211MacSignal
{
    signalType = 4;
    int par1;
    NavSrc navSrc;
}

packet Ieee80211MacSignalBkDone extends Ieee80211MacSignal
{
    signalType = 5;
    int slotCount;
}

packet Ieee80211MacSignalMaUnitDataRequest extends Ieee80211MacSignal
{
    signalType = 6;
    MACAddress senderAddres; // sa?
    MACAddress destinationAddress; // da
    Routing routing;
    cPacket llcData;
    CfPriority priority;
    ServiceClass serviceClass;
}

packet Ieee80211MacSignalMsduConfirm extends Ieee80211MacSignal
{
    signalType = 7;
    cPacket sdu;
    CfPriority priority;
    TxStatus txStatus;
}

packet Ieee80211MacSignalFragRequest extends Ieee80211MacSignal
{
    signalType = 8;
    Ieee80211MacFragmentationSupport::FragSdu fsdu;
}

packet Ieee80211MacSignalPduRequest extends Ieee80211MacSignal
{
    signalType = 9;
    Ieee80211MacFragmentationSupport::FragSdu fsdu;
}

packet Ieee80211MacSignalFragConfirm extends Ieee80211MacSignal
{
    signalType = 10;
    Ieee80211MacFragmentationSupport::FragSdu fsdu;
    TxResult txResult;
    CfPriority priority;
}

packet Ieee80211MacSignalCfPolled extends Ieee80211MacSignal
{
    signalType = 11;
}

packet Ieee80211MacSignalPsInquiry extends Ieee80211MacSignal
{
    signalType = 12;
    MACAddress macAddress;
}

packet Ieee80211MacSignalAtimW extends Ieee80211MacSignal
{
    signalType = 13;
}

packet Ieee80211MacSignalPsChange extends Ieee80211MacSignal
{
    signalType = 14;
    MACAddress addr;
    PsMode psMode;
}

packet Ieee80211MacSignalPsResponse extends Ieee80211MacSignal
{
    signalType = 15;
    MACAddress adr;
    PsMode psMode;
}

packet Ieee80211MacSignalCfPoll extends Ieee80211MacSignal
{
    signalType = 16;
    simtime_t time;
    double rate;
}

packet Ieee80211MacSignalTbtt extends Ieee80211MacSignal
{
    signalType = 17;
}

packet Ieee80211MacSignalCfEnd extends Ieee80211MacSignal
{
    signalType = 18;
}

packet Ieee80211MacSignalTxConfirm extends Ieee80211MacSignal
{
    signalType = 19;
}

packet Ieee80211MacSignalTxCfAck extends Ieee80211MacSignal
{
    signalType = 20;
    simtime_t time;
    double rate;
}

packet Ieee80211MacSignalTifs extends Ieee80211MacSignal
{
    signalType = 21;
}

packet Ieee80211MacSignalIdle extends Ieee80211MacSignal
{
    signalType = 22;
}

packet Ieee80211MacSignalBusy extends Ieee80211MacSignal
{
    signalType = 23;
}

packet Ieee80211MacSignalSlot extends Ieee80211MacSignal
{
    signalType = 24;
}

packet Ieee80211MacSignalPhyTxStartRequest extends Ieee80211MacSignal
{
    signalType = 25;
}

packet Ieee80211MacSignalPhyTxStartConfirm extends Ieee80211MacSignal
{
    signalType = 26;
}

packet Ieee80211MacSignalPhyDataConfirm extends Ieee80211MacSignal
{
    signalType = 27;
}

packet Ieee80211MacSignalPhyTxEndConfirm extends Ieee80211MacSignal
{
    signalType = 28;
}

packet Ieee80211MacSignalAck extends Ieee80211MacSignal
{
    signalType = 29;
    simtime_t endRx;
    double txRate;
}

packet Ieee80211MacSignalCts extends Ieee80211MacSignal
{
    signalType = 30;
    simtime_t endRx;
    double txRate;
}

packet Ieee80211MacSignalMmCancel extends Ieee80211MacSignal
{
    signalType = 31;
}

packet Ieee80211MacSignalDoze extends Ieee80211MacSignal
{
    signalType = 32;
}

packet Ieee80211MacSignalWake extends Ieee80211MacSignal
{
    signalType = 33;
}

packet Ieee80211MacSignalTpdly extends Ieee80211MacSignal
{
    signalType = 34;
}

packet Ieee80211MacSignalSwChnl extends Ieee80211MacSignal
{
    signalType = 35;
    int chan;
    bool doBkoff;
}

packet Ieee80211MacSignalMsduRequest extends Ieee80211MacSignal
{
    signalType = 36;
    Ieee80211NewFrame sdu;
    CfPriority priority;
}

packet Ieee80211MacSignalMmRequest extends Ieee80211MacSignal
{
    signalType = 37;
    Ieee80211NewFrame sdu;
    Imed imed;
    CfPriority priority; // TODO??
    double rate;
}

packet Ieee80211MacSignalXport extends Ieee80211MacSignal
{
    signalType = 38;
}

packet Ieee80211MacSignalSst extends Ieee80211MacSignal
{
    signalType = 39;
    MACAddress addr;
    StationState stationState;
}

packet Ieee80211MacSignalSend extends Ieee80211MacSignal
{
    signalType = 40;
    cPacket frame;
    Imed priority; // ???
}

packet Ieee80211MacSignalMmConfirm extends Ieee80211MacSignal
{
    signalType = 41;
    cPacket frame;
    TxStatus status;
}

packet Ieee80211MacSignalMmIndicate extends Ieee80211MacSignal
{
    signalType = 42;
    cPacket frame;
    simtime_t time1;
    simtime_t time2;
    StateErr stateErr;
}

packet Ieee80211MacSignalMlmeGetRequest extends Ieee80211MacSignal
{
    signalType = 43;
    string mibAtrib;
}

packet Ieee80211MacSignalMlmeSetRequest extends Ieee80211MacSignal
{
    signalType = 44;
    string mibAtrib;
    int mibValue;
}

packet Ieee80211MacSignalMlmeGetConfirm extends Ieee80211MacSignal
{
    signalType = 45;
    MibStatus mibStatus;
    string mibAtrib;
    int mibValue;
}

packet Ieee80211MacSignalMlmeSetConfirm extends Ieee80211MacSignal
{
    signalType = 46;
    MibStatus mibStatus;
    string mibAtrib;
}

packet Ieee80211MacSignalMlmeResetRequest extends Ieee80211MacSignal
{
    signalType = 47;
    MACAddress addr;
    bool dflt;
}

packet Ieee80211MacSignalSwDone extends Ieee80211MacSignal
{
    signalType = 48;
}

packet Ieee80211MacSignalPlmeSetConfirm extends Ieee80211MacSignal
{
    signalType = 49;
    MibStatus status;
    string attrib; // MibAtrib
}


packet Ieee80211MacSignalPlmeSetRequest extends Ieee80211MacSignal
{
    signalType = 50;
    string atrrib; // MibAtrib
    int mibValue; // MibValue
}

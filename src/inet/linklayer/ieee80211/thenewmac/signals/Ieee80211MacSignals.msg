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
    ServiceClass serviceClass;
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

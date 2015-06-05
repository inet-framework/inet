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

enum Ieee80211MacSignalType
{
    TX_REQUEST = 0;
    BACKOFF = 1;
    PDU_CONFIRM = 2;
    CANCEL = 3;
    CHANGE_NAV = 4;
    BKDONE = 5;
    MA_UNIT_DATA_REQUEST = 6;
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
    int par1;
    int par2;
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
    MACAddress senderAddres; // sa?
    MACAddress destinationAddress; // da
    Routing routing;
    cPacket llcData;
    CfPriority priority;
    ServiceClass serviceClass;
}

packet Ieee80211MacSignalMsduConfirm extends Ieee80211MacSignal
{
    cPacket sdu;
    ServiceClass serviceClass;
    TxStatus txStatus;
}

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

enum Ieee80211MacTxCoordinationSignalType
{
    TX_REQUEST = 0;
    BACKOFF = 1;
    PDU_CONFIRM = 2;
    CANCEL = 3;
    CHANGE_NAV = 4;
}

packet Ieee80211MacTxCoordinationSignal
{
    short signalType @enum(Ieee80211MacTxCoordinationSignalType);
}

packet Ieee80211MacTxCoordinationTxRequest extends Ieee80211MacTxCoordinationSignal
{
    // tpdu as encapsulated packet
    signalType = 0;
    double txrate;
}

packet Ieee80211MacTxCoordinationBackoff extends Ieee80211MacTxCoordinationSignal
{
    signalType = 1;
    int par1;
    int par2;
}

packet Ieee80211MacTxCoordinationPduConfirm extends Ieee80211MacTxCoordinationSignal
{
    signalType = 2;
    Ieee80211MacFragmentationSupport::FragSdu fsdu;
    TxResult txResult;
}

packet Ieee80211MacTxCoordinationCancel extends Ieee80211MacTxCoordinationSignal
{
    signalType = 3;
}

packet Ieee80211MacTxCoordinationChangeNav extends Ieee80211MacTxCoordinationSignal
{
    signalType = 4;
    int par1;
    NavSrc navSrc;
}
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
#include "inet/linklayer/ieee80211/thenewmac/macsorts/Ieee80211MacFrameTypes.h"
#include "inet/linklayer/ieee80211/thenewmac/macsorts/Ieee80211MacEnumeratedMacStaTypes.h"
#include "inet/linklayer/common/MACAddress.h"
}}

class noncobject TypeSubtype;
class noncobject BasicType;
class noncobject StatusCode;
class noncobject AuthType;
class noncobject Capability;
class noncobject MACAddress;

namespace inet::ieee80211;

//
// FRAME sort (the basic definition of fields in MAC frames)
//
packet Ieee80211NewFrame
{
    int protocolVer; // Protocol version (2 bits)
    BasicType basetype; // Type field (2 bits)
    TypeSubtype ftype; // Type & Subtype (6 bits)
    bool toDs; // To DS bit (1 bit)
    bool frDs; // From DS bit (1 bit)
    bool moreFrag; // More Fragments bit (1 bit)
    bool retryBit; // Retry bit (1 bit)
    bool pwrMgt; // Power Management bit (1 bit)
    bool moreData; // More Data bit (1 bit)
    bool wepBit; // WEP bit (1 bit)
    bool orderBit; // {strictly}Order{ed} (1 bit)
    int durId; // Duration/ID field (2)
    MACAddress addr1; // Address 1 [DA/RA] field (6)
    MACAddress addr2; // Address 2 [SA/TA] field (6)
    MACAddress addr3; // Address 3 [Bss/DA/SA] field
    MACAddress addr4; // Address 4 [WDS-SA] field (6)
    int seq; // Sequence Number (12 bits)
    int frag; // Fragment Number (4 bits)
    simtime_t ts; // Timestamp field (8)
    StatusCode status; // Status Code field (2)
    string reason; // Reason Code field (2)
    StatusCode authStat; // Status Code in Auth frame
    int authSeqNum; // Auth Sequence Number (2)
    AuthType authAlg; // Auth Algorithm field (2)
    simtime_t beaconInt; // Beacon Interval field (2)
    simtime_t listenInt; // Listen Interval field (2)
    int aid; // Association ID field (2)
    MACAddress curApAddr; // Current AP Addr field (6)
    Capability capA; // Capability (Re)Asoc
    Capability capB; // Capability Bcn/Probe
    int keyId; // Key ID subfield (2 bits)
}

//
// Copyright (C) 2006 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

//
// Required for MACAddress declarations
//
cplusplus
{{
#include "Ieee80211MgmtFrames_m.h"
}};
class noncobject MACAddress;
class noncobject Ieee80211CapabilityInformation;
class noncobject Ieee80211SupportedRatesElement;

enum AuthenticationType
{
    AUTHTYPE_OPENSYSTEM = 1;
    AUTHTYPE_SHAREDKEY = 2;
};

//
// Base class
//
class Ieee80211Prim
{
};

//
// Losely based on MLME-SCAN.request
//
class Ieee80211Prim_ScanRequest extends Ieee80211Prim
{
    fields:
        bool BSSType;
        MACAddress BSSID;
        string SSID;
        bool activeScan;
        double probeDelay;
        int channelList[];
        double minChannelTime;
        double maxChannelTime;
};

class Ieee80211Prim_BSSDescription
{
    fields:
        MACAddress BSSID;
        string SSID;
        Ieee80211SupportedRatesElement supportedRates;
        Ieee80211CapabilityInformation capabilityInfo;
        unsigned short beaconInterval;
        unsigned short DSChannel;
};

//
// Loosely based on MLME-SCAN.confirm
//
class Ieee80211Prim_ScanConfirm extends Ieee80211Prim
{
    fields:
        Ieee80211Prim_BSSDescription bssList[];
};

//
// Losely based on MLME-AUTHENTICATE.request
//
class Ieee80211Prim_AuthenticateRequest extends Ieee80211Prim
{
    fields:
        MACAddress address;
        int authType enum(AuthenticationType);
        double timeout;
};

//
// Losely based on MLME-AUTHENTICATE.confirm
//
class Ieee80211Prim_AuthenticateConfirm extends Ieee80211Prim
{
    fields:
        MACAddress address;
        int authType enum(AuthenticationType);
        int result; //=0 OK
};

//
// Losely based on MLME-ASSOCIATE.request
//
class Ieee80211Prim_AssociateRequest extends Ieee80211Prim
{
    fields:
        MACAddress address;
        double timeout;
        Ieee80211CapabilityInformation capablityInfo;
        int listenInterval; // unsupported by MAC
};

//
// Losely based on MLME-ASSOCIATE.confirm
//
class Ieee80211Prim_AssociateConfirm extends Ieee80211Prim
{
    fields:
        int result; // SUCCESS, INVALID_PARAMETERS, TIMEOUT, REFUSED
};


// TODO: deauthenticate, disassociate, reassociate, ...



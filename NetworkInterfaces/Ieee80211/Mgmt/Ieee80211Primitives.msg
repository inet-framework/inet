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

enum Ieee80211AuthenticationType
{
    AUTHTYPE_OPENSYSTEM = 1;
    AUTHTYPE_SHAREDKEY = 2;
};

enum Ieee80211BSSType
{
    BSSTYPE_ANY = 0;
    BSSTYPE_INFRASTRUCTURE = 1;
    BSSTYPE_INDEPENDENT = 2;
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
        int BSSType enum(Ieee80211BSSType); // determines type of BSS's to include in the scan
        MACAddress BSSID; // specific BSSID to scan for (default: any)
        string SSID;      // SSID to scan for SSID (default: any)
        bool activeScan;  // whether to perform active or passive scanning
        double probeDelay; // delay (in ìs) to be used prior to transmitting a Probe frame during active scanning
        int channelList[]; // list of channels to scan (default: all channels)
        double minChannelTime; // minimum time to spend on each channel when scanning
        double maxChannelTime; // maximum time to spend on each channel when scanning
};

class Ieee80211Prim_BSSDescription
{
    fields:
        int channel;
        MACAddress BSSID;
        string SSID;
        Ieee80211SupportedRatesElement supportedRates;
        Ieee80211CapabilityInformation capabilityInfo;
        double beaconInterval;

        double rxPower; // received power from AP; not part of the standard
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
        int authType enum(Ieee80211AuthenticationType);
        double timeout;
};

//
// Losely based on MLME-AUTHENTICATE.confirm
//
class Ieee80211Prim_AuthenticateConfirm extends Ieee80211Prim
{
    fields:
        MACAddress address;
        int authType enum(Ieee80211AuthenticationType);
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
        Ieee80211CapabilityInformation capabilityInfo;
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



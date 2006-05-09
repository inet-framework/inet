//
// Copyright (C) 2006 Andras Varga
// Copyright (C) 2001 Eric Wu and Steve Woon, Monash University, Melbourne, Australia
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
#include "MACAddress.h"
#include "Ieee80211Frame_m.h"
}};
class noncobject MACAddress;
message Ieee80211ManagementFrame;

//
// Capability Information field format
//
struct Ieee80211CapabilityInformation
{
    fields:
        bool ESS;
        bool IBSS;
        bool CFPollable;
        bool CFPollRequest;
        bool privacy;
};

//
// Additional parameters for handover decisions
// (Not part of standard, for experimental purposes)
//
struct Ieee80211HandoverParameters
{
    fields:
        double avgBackoffTime;
        double avgWaitTime;
        double avgErrorRate;
        double estAvailBW;
};

enum Ieee80211ReasonCode
{
    RC_UNSPECIFIED = 1;
    RC_PREV_AUTH_EXPIRED = 2;
    RC_DEAUTH_MS_LEAVING = 3;
    RC_DISASS_INACTIVITY = 4;
    RC_DISASS_TOO_MANY_MS = 5;
    RC_NONAUTH_CLASS2 = 6;
    RC_NONASS_CLASS3 = 7;
    RC_DIASS_MS_LEAVING = 8;
    RC_NONAUTH_ASS_REQUEST = 9;
};

enum Ieee80211StatusCode
{
    SC_SUCCESSFUL = 0;
    SC_UNSPECIFIED = 1;
    SC_UNSUP_CAP = 2;
    SC_RESASS_DENIED = 3;
    SC_ASS_DENIED_UNKNOWN = 4;
    SC_AUTH_ALG0_UNSUP = 5;
    SC_AUTH_OUT_OF_SEQ = 6;
    SC_CHALLANGE_FAILURE = 7;
    SC_AUTH_TIMEOUT = 8;
    SC_ASS_TOO_MANY_MS = 9;
    SC_DATARATE_SUP = 10;
};

//
// Structure to store a single supported rate information
//
struct Ieee80211SupportedRatesElement
{
    fields:
        bool supported; // true=supported, false=unsupported
        double rate;    // in Mbit/sec
};

//
// Frame body base class used to hide various frame body types
//
class Ieee80211FrameBody
{
    fields:
        unsigned short length; //XXX what's this?
};

//
// Authentication frame body format
//
class Ieee80211AuthenticationFrameBody extends Ieee80211FrameBody
{
    fields:
        unsigned short sequenceNumber;
        int statusCode enum(Ieee80211StatusCode);
};

//
// De-authentication frame body format
//
class Ieee80211DeauthenticationFrameBody extends Ieee80211FrameBody
{
    fields:
        int reasonCode enum(Ieee80211ReasonCode);
};

//
// Dis-association frame body format (same as De-authentication)
//
class Ieee80211DisassociationFrameBody extends Ieee80211FrameBody
{
    fields:
        int reasonCode enum(Ieee80211ReasonCode);
};

//
// Probe request frame body format
//
class Ieee80211ProbeRequestFrameBody extends Ieee80211FrameBody
{
    fields:
        string SSID;
        Ieee80211SupportedRatesElement supportedRates[];
};

//
// Association request frame body format
//
class Ieee80211AssociationRequestFrameBody extends Ieee80211FrameBody
{
    fields:
        string SSID;
        Ieee80211SupportedRatesElement supportedRates[];
        Ieee80211CapabilityInformation capabilityInformation;
};

//
// Re-association request frame body format (same as association)
//
class Ieee80211ReassociationRequestFrameBody extends Ieee80211AssociationRequestFrameBody
{
    fields:
        MACAddress currentAP;
};

//
// Association response frame body format
//
class Ieee80211AssociationResponseFrameBody extends Ieee80211FrameBody
{
    fields:
        //XXX ???? unsigned short sequenceNumber;
        int statusCode enum(Ieee80211StatusCode);
        Ieee80211CapabilityInformation capabilityInformation;
        Ieee80211SupportedRatesElement supportedRates[];
};

//
// Re-association response frame body format
//
class Ieee80211ReassociationResponseFrameBody extends Ieee80211AssociationResponseFrameBody
{
    fields:
};

//
// Beacon frame body format
//
class Ieee80211BeaconFrameBody extends Ieee80211FrameBody
{
    fields:
        string SSID;
        Ieee80211SupportedRatesElement supportedRates[];
        Ieee80211CapabilityInformation capabilityInformation;
        double timestamp;
        unsigned short beaconInterval;
        unsigned short DSChannel;
        Ieee80211HandoverParameters handoverParameters;
};

//
// Probe response frame body format (same as Beacon)
//
class Ieee80211ProbeResponseFrameBody extends Ieee80211BeaconFrameBody
{
    fields:
};


//--------------------------------------------------------------


//
// Authentication frame format (management frame + body)
//
message Ieee80211AuthenticationFrame extends Ieee80211ManagementFrame
{
    fields:
        Ieee80211AuthenticationFrameBody body;
};

//
// De-authentication frame format (management frame + body)
//
message Ieee80211DeauthenticationFrame extends Ieee80211ManagementFrame
{
    fields:
        Ieee80211DeauthenticationFrameBody body;
};

//
// Dis-association frame format (management frame + body) (same as De-authentication)
//
message Ieee80211DisassociationFrame extends Ieee80211ManagementFrame
{
    fields:
        Ieee80211DisassociationFrameBody body;
};

//
// Probe request frame format (management frame + body)
//
message Ieee80211ProbeRequestFrame extends Ieee80211ManagementFrame
{
    fields:
        Ieee80211ProbeRequestFrameBody body;
};

//
// Association request frame format (management frame + body)
//
message Ieee80211AssociationRequestFrame extends Ieee80211ManagementFrame
{
    fields:
        Ieee80211AssociationRequestFrameBody body;
};

//
// Re-association request frame format (management frame + body) (same as association)
//
message Ieee80211ReassociationRequestFrame extends Ieee80211ManagementFrame
{
    fields:
        Ieee80211ReassociationRequestFrameBody body;
};

//
// Association response frame format (management frame + body)
//
message Ieee80211AssociationResponseFrame extends Ieee80211ManagementFrame
{
    fields:
        Ieee80211AssociationResponseFrameBody body;
};

//
// Re-association response frame format (management frame + body)
//
message Ieee80211ReassociationResponseFrame extends Ieee80211ManagementFrame
{
    fields:
        Ieee80211ReassociationResponseFrameBody body;
};

//
// Beacon frame format (management frame + body)
//
message Ieee80211BeaconFrame extends Ieee80211ManagementFrame
{
    fields:
        Ieee80211BeaconFrameBody body;
};

//
// Probe response frame format (management frame + body); same as Beacon
//
message Ieee80211ProbeResponseFrame extends Ieee80211ManagementFrame
{
    fields:
        Ieee80211ProbeResponseFrameBody body;
};

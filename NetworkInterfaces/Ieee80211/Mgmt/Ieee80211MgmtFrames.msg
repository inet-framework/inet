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
message W80211ManagementFrame;

//
// Capability Information field format
//
struct W80211CapabilityInformation
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
struct W80211HandoverParameters
{
    fields:
        double avgBackoffTime;
        double avgWaitTime;
        double avgErrorRate;
        double estAvailBW;
};

enum W80211ReasonCode
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

enum W80211StatusCode
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
struct W80211SupportedRatesElement
{
    fields:
        bool supported; // true=supported, false=unsupported
        double rate;    // in Mbit/sec
};

//
// Frame body base class used to hide various frame body types
//
class W80211FrameBody
{
    fields:
        unsigned short length; //XXX what's this?
};

//
// Authentication frame body format
//
class W80211AuthenticationFrameBody extends W80211FrameBody
{
    fields:
        unsigned short sequenceNumber;
        int statusCode enum(W80211StatusCode);
};

//
// De-authentication frame body format
//
class W80211DeauthenticationFrameBody extends W80211FrameBody
{
    fields:
        int reasonCode enum(W80211ReasonCode);
};

//
// Dis-association frame body format (same as De-authentication)
//
class W80211DisassociationFrameBody extends W80211FrameBody
{
    fields:
        int reasonCode enum(W80211ReasonCode);
};

//
// Probe request frame body format
//
class W80211ProbeRequestFrameBody extends W80211FrameBody
{
    fields:
        string SSID;
        W80211SupportedRatesElement supportedRates[];
};

//
// Association request frame body format
//
class W80211AssociationRequestFrameBody extends W80211FrameBody
{
    fields:
        string SSID;
        W80211SupportedRatesElement supportedRates[];
        W80211CapabilityInformation capabilityInformation;
};

//
// Re-association request frame body format (same as association)
//
class W80211ReassociationRequestFrameBody extends W80211AssociationRequestFrameBody
{
    fields:
        MACAddress currentAP;
};

//
// Association response frame body format
//
class W80211AssociationResponseFrameBody extends W80211FrameBody
{
    fields:
        //XXX ???? unsigned short sequenceNumber;
        int statusCode enum(W80211StatusCode);
        W80211CapabilityInformation capabilityInformation;
        W80211SupportedRatesElement supportedRates[];
};

//
// Re-association response frame body format
//
class W80211ReassociationResponseFrameBody extends W80211AssociationResponseFrameBody
{
    fields:
};

//
// Beacon frame body format
//
class W80211BeaconFrameBody extends W80211FrameBody
{
    fields:
        string SSID;
        W80211SupportedRatesElement supportedRates[];
        W80211CapabilityInformation capabilityInformation;
        double timestamp;
        unsigned short beaconInterval;
        unsigned short DSChannel;
        W80211HandoverParameters handoverParameters;
};

//
// Probe response frame body format (same as Beacon)
//
class W80211ProbeResponseFrameBody extends W80211BeaconFrameBody
{
    fields:
};


//--------------------------------------------------------------


//
// Authentication frame format (management frame + body)
//
message W80211AuthenticationFrame extends W80211ManagementFrame
{
    fields:
        W80211AuthenticationFrameBody body;
};

//
// De-authentication frame format (management frame + body)
//
message W80211DeauthenticationFrame extends W80211ManagementFrame
{
    fields:
        W80211DeauthenticationFrameBody body;
};

//
// Dis-association frame format (management frame + body) (same as De-authentication)
//
message W80211DisassociationFrame extends W80211ManagementFrame
{
    fields:
        W80211DisassociationFrameBody body;
};

//
// Probe request frame format (management frame + body)
//
message W80211ProbeRequestFrame extends W80211ManagementFrame
{
    fields:
        W80211ProbeRequestFrameBody body;
};

//
// Association request frame format (management frame + body)
//
message W80211AssociationRequestFrame extends W80211ManagementFrame
{
    fields:
        W80211AssociationRequestFrameBody body;
};

//
// Re-association request frame format (management frame + body) (same as association)
//
message W80211ReassociationRequestFrame extends W80211ManagementFrame
{
    fields:
        W80211ReassociationRequestFrameBody body;
};

//
// Association response frame format (management frame + body)
//
message W80211AssociationResponseFrame extends W80211ManagementFrame
{
    fields:
        W80211AssociationResponseFrameBody body;
};

//
// Re-association response frame format (management frame + body)
//
message W80211ReassociationResponseFrame extends W80211ManagementFrame
{
    fields:
        W80211ReassociationResponseFrameBody body;
};

//
// Beacon frame format (management frame + body)
//
message W80211BeaconFrame extends W80211ManagementFrame
{
    fields:
        W80211BeaconFrameBody body;
};

//
// Probe response frame format (management frame + body); same as Beacon
//
message W80211ProbeResponseFrame extends W80211ManagementFrame
{
    fields:
        W80211ProbeResponseFrameBody body;
};

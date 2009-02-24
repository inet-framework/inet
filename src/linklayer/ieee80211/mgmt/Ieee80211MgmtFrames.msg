//
// Copyright (C) 2006 Andras Varga
// Copyright (C) 2001 Eric Wu and Steve Woon, Monash University, Melbourne, Australia
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
// Required for MACAddress declarations
//
cplusplus {{
#include "MACAddress.h"
#include "Ieee80211Frame_m.h"
}}

class noncobject MACAddress;

packet Ieee80211ManagementFrame;


//
// Capability Information field format. This field is not used
// by the model (and is omitted from management frame formats too),
// because its fields are related to features not supported by this model
// (PCF, privacy, and beacons in ad-hoc mode). It is only provided here
// for completeness.
//
struct Ieee80211CapabilityInformation
{
    bool ESS;
    bool IBSS;
    bool CFPollable;
    bool CFPollRequest;
    bool privacy;
}

//
// Additional parameters for handover decisions
// (Not part of standard, for experimental purposes)
//
struct Ieee80211HandoverParameters
{
    double avgBackoffTime;
    double avgWaitTime;
    double avgErrorRate;
    double estAvailBW;
}

//
// 802.11 reason code; values correspond to the standard.
//
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
}

//
// 802.11 status code; values correspond to the standard.
//
enum Ieee80211StatusCode
{

    SC_SUCCESSFUL = 0;
    SC_UNSPECIFIED = 1;
    SC_UNSUP_CAP = 10;
    SC_REASS_DENIED = 11;
    SC_ASS_DENIED_UNKNOWN = 12;
    SC_AUTH_ALG0_UNSUP = 13;
    SC_AUTH_OUT_OF_SEQ = 14;
    SC_AUTH_CHALLENGE_FAIL = 15;
    SC_AUTH_TIMEOUT = 16;
    SC_ASS_TOO_MANY_MS = 17;
    SC_DATARATE_UNSUP = 18;
}

//
// Structure to store supported rates
//
struct Ieee80211SupportedRatesElement
{
    short numRates; // number of rates (max 8)
    double rate[8]; // in Mbit/s; should be multiple of 500 kbit/s
}

//
// Frame body base class used to hide various frame body types
//
class Ieee80211FrameBody
{
    short bodyLength; // not an actual frame field
}

//
// Authentication frame body format.
//
// Note: the "isLast" field is not part of the standard.
// The authentication procedure is simulated by this model by exchanging
// a number of "dummy" authentication frames without real contents;
// the "isLast" field gets set by the AP to signal the STA that
// the authentication procedure has completed and it does not need to
// send any more Authentication frames.
//
class Ieee80211AuthenticationFrameBody extends Ieee80211FrameBody
{
    bodyLength = 6; // add 2+len bytes for challenge text if present
    unsigned short sequenceNumber;
    int statusCode enum(Ieee80211StatusCode);
    bool isLast; // not part of the standard; see above
}

//
// De-authentication frame body format
//
class Ieee80211DeauthenticationFrameBody extends Ieee80211FrameBody
{
    bodyLength = 2;
    int reasonCode enum(Ieee80211ReasonCode);
}

//
// Dis-association frame body format (same as De-authentication)
//
class Ieee80211DisassociationFrameBody extends Ieee80211FrameBody
{
    bodyLength = 2;
    int reasonCode enum(Ieee80211ReasonCode);
}

//
// Probe request frame body format
//
class Ieee80211ProbeRequestFrameBody extends Ieee80211FrameBody
{
    bodyLength = 12; // assuming a 8-character SSID
    string SSID;
    Ieee80211SupportedRatesElement supportedRates;
}

//
// Association request frame body format
//
class Ieee80211AssociationRequestFrameBody extends Ieee80211FrameBody
{
    bodyLength = 16; // assuming a 8-character SSID
    string SSID;
    Ieee80211SupportedRatesElement supportedRates;
}

//
// Re-association request frame body format (same as association)
//
class Ieee80211ReassociationRequestFrameBody extends Ieee80211AssociationRequestFrameBody
{
    bodyLength = 26; // assuming a 8-character SSID and 4 supported datarates
    MACAddress currentAP;
}

//
// Association response frame body format
//
class Ieee80211AssociationResponseFrameBody extends Ieee80211FrameBody
{
    bodyLength = 26; // assuming a 8-character SSID and 4 supported datarates
    int statusCode enum(Ieee80211StatusCode);
    short aid;
    Ieee80211SupportedRatesElement supportedRates;
}

//
// Re-association response frame body format (same as AssociationResponse)
//
class Ieee80211ReassociationResponseFrameBody extends Ieee80211AssociationResponseFrameBody
{
}

//
// Beacon frame body format.
//
class Ieee80211BeaconFrameBody extends Ieee80211FrameBody
{
    bodyLength = 50; // assuming a 8-character SSID, 4 supported datarate plus TIM
    string SSID;
    Ieee80211SupportedRatesElement supportedRates;
    simtime_t beaconInterval;
    int channelNumber;
    Ieee80211HandoverParameters handoverParameters;
}

//
// Probe response frame body format (same as Beacon)
//
class Ieee80211ProbeResponseFrameBody extends Ieee80211BeaconFrameBody
{
}


//--------------------------------------------------------------


//
// Authentication frame format (management frame + body)
//
packet Ieee80211AuthenticationFrame extends Ieee80211ManagementFrame
{
    type = ST_AUTHENTICATION;
    byteLength = 28+getBody().getBodyLength();
    Ieee80211AuthenticationFrameBody body;
}

//
// De-authentication frame format (management frame + body)
//
packet Ieee80211DeauthenticationFrame extends Ieee80211ManagementFrame
{
    type = ST_DEAUTHENTICATION;
    byteLength = 28+getBody().getBodyLength();
    Ieee80211DeauthenticationFrameBody body;
}

//
// Dis-association frame format (management frame + body) (same as De-authentication)
//
packet Ieee80211DisassociationFrame extends Ieee80211ManagementFrame
{
    type = ST_DISASSOCIATION;
    byteLength = 28+getBody().getBodyLength();
    Ieee80211DisassociationFrameBody body;
}

//
// Probe request frame format (management frame + body)
//
packet Ieee80211ProbeRequestFrame extends Ieee80211ManagementFrame
{
    type = ST_PROBEREQUEST;
    byteLength = 28+getBody().getBodyLength();
    Ieee80211ProbeRequestFrameBody body;
}

//
// Association request frame format (management frame + body)
//
packet Ieee80211AssociationRequestFrame extends Ieee80211ManagementFrame
{
    type = ST_ASSOCIATIONREQUEST;
    byteLength = 28+getBody().getBodyLength();
    Ieee80211AssociationRequestFrameBody body;
}

//
// Re-association request frame format (management frame + body) (same as association)
//
packet Ieee80211ReassociationRequestFrame extends Ieee80211ManagementFrame
{
    type = ST_REASSOCIATIONREQUEST;
    byteLength = 28+getBody().getBodyLength();
    Ieee80211ReassociationRequestFrameBody body;
}

//
// Association response frame format (management frame + body)
//
packet Ieee80211AssociationResponseFrame extends Ieee80211ManagementFrame
{
    type = ST_ASSOCIATIONRESPONSE;
    byteLength = 28+getBody().getBodyLength();
    Ieee80211AssociationResponseFrameBody body;
}

//
// Re-association response frame format (management frame + body)
//
packet Ieee80211ReassociationResponseFrame extends Ieee80211ManagementFrame
{
    type = ST_REASSOCIATIONRESPONSE;
    byteLength = 28+getBody().getBodyLength();
    Ieee80211ReassociationResponseFrameBody body;
}

//
// Beacon frame format (management frame + body)
//
packet Ieee80211BeaconFrame extends Ieee80211ManagementFrame
{
    type = ST_BEACON;
    byteLength = 28+getBody().getBodyLength();
    Ieee80211BeaconFrameBody body;
}

//
// Probe response frame format (management frame + body); same as Beacon
//
packet Ieee80211ProbeResponseFrame extends Ieee80211ManagementFrame
{
    type = ST_PROBERESPONSE;
    byteLength = 28+getBody().getBodyLength();
    Ieee80211ProbeResponseFrameBody body;
}


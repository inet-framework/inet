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
#define PREQElemLen 11
#define PERRElemLen 13
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
    bool DelayedBlockAck;
    bool InmediateBlockAck;
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
    RC_MESH_PEERING_CANCELLED = 52;
    RC_MESH_MAX_PEERS = 53;
    RC_MESH_CONFIGURATION_POLICY_VIOLATION = 54;
    RC_MESH_CLOSE_RCVD = 55;
    RC_MESH_MAX_RETRIES = 56;
    RC_MESH_CONFIRM_TIMEOUT = 57;
    RC_MESH_INVALID_GTK = 58;
    RC_MESH_INCONSISTENT_PARAMETERS = 59;
    RC_MESH_INVALID_SECURITY_CAPABILITY = 60;
    RC_MESH_PATH_ERROR_NO_PROXY_INFORMATION = 61;
    RC_MESH_PATH_ERROR_NO_FORWARDING_INFORMATION = 60;
    RC_MESH_PATH_ERROR_DESTINATION_UNREACHABLE = 63;
    RC_MAC_ADDRESS_ALREADY_EXISTS_IN_MBSS = 64;
    RC_MESH_CHANNEL_SWITCH_REGULATORY_REQUIREMENTS = 65;
    RC_MESH_CHANNEL_SWITCH_UNSPECIFIED = 66;
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
    SC_AUTH_REJECT_ANTI_CLOGING = 76;
    SC_AUTH_REJECT_FINITE_CYC_GROUP_UNS = 77;
    SC_TBTT_REQUEST = 78;
}

enum Ieee80211HWMPCode
{
    IE11S_GANN = 125;
    IE11S_RANN = 126;
    IE11S_PREQ = 130;
    IE11S_PREP = 131;
    IE11S_PERR = 132;
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

class Ieee80211ActionFrameBody extends Ieee80211FrameBody
{
}

class Ieee80211ActionHWMPFrameBody extends Ieee80211ActionFrameBody
{
     int id enum(Ieee80211HWMPCode);
     //unsigned char length; 
     unsigned char flags = 0;
     unsigned char hopsCount = 0;
     unsigned char TTL = 32;
 }


struct PREQElem
{
     bool TO = true;
     bool USN = false;
     MACAddress targetAddress;
     unsigned int targetSeqNumber;
}

struct PERRElem
{
     bool RC;
     bool AE;
     MACAddress destAddress;
     unsigned int destSeqNumber;
     unsigned int destExternalAddress;
     unsigned short reasonCode;
}

     
class Ieee80211ActionRANNFrameBody extends Ieee80211ActionHWMPFrameBody
{
     bodyLength = 23;
     id = IE11S_RANN;
     MACAddress root;
     unsigned int seqNumber;
     unsigned int interval;
     unsigned int metric = 0;
}

class  Ieee80211ActionPREQFrameBody extends Ieee80211ActionHWMPFrameBody
{
     bodyLength = 26;
     id = IE11S_PREQ;
     unsigned int pathDiscoveryId;
     MACAddress originator;
     unsigned int originatorSeqNumber;
     MACAddress originatorExternalAddr;
     unsigned int lifeTime;
     unsigned int metric = 0;
     unsigned char targetCount;
     PREQElem preqElem[];
}

class  Ieee80211ActionPREPFrameBody extends Ieee80211ActionHWMPFrameBody
{
     bodyLength = 37;
     id = IE11S_PREP;
     MACAddress target;
     unsigned int targetSeqNumber;
     MACAddress tagetExternalAddr;
     unsigned int lifeTime;
     unsigned int metric = 0;
     MACAddress originator;
     unsigned int originatorSeqNumber;
}

class  Ieee80211ActionPERRFrameBody extends Ieee80211ActionHWMPFrameBody
{
     bodyLength = 2;
     id = IE11S_PERR;
     unsigned char numDestination;
     PERRElem perrElem[];
}

class  Ieee80211ActionGANNFrameBody extends Ieee80211ActionHWMPFrameBody
{
    bodyLength = 15;
    id = IE11S_GANN;
    MACAddress meshGateAddress;
    unsigned int meshGateSeqNumber;
    
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

//
// Action frames
//
packet Ieee80211ActionFrame extends Ieee80211ManagementFrame
{
    type = ST_ACTION;
    byteLength = 28+getBody().getBodyLength();
    Ieee80211ActionFrameBody body;
}

//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//
// Required for MacAddress declarations
//
import inet.common.INETDefs;
import inet.linklayer.ieee80211.mgmt.Ieee80211MgmtFrame;
import inet.linklayer.common.MacAddress;

namespace inet::ieee80211;

//
// IEEE 802.11 command codes, sent by the agent to the management entity.
// These constants should be set as message kind on the messages.
//
// @see ~Ieee80211PrimConfirmCode, ~Ieee80211MgmtSta, ~Ieee80211AgentSta
//
enum Ieee80211PrimRequestCode
{

    PR_SCAN_REQUEST = 1;
    PR_AUTHENTICATE_REQUEST = 2;
    PR_DEAUTHENTICATE_REQUEST = 3;
    PR_ASSOCIATE_REQUEST = 4;
    PR_REASSOCIATE_REQUEST = 5;
    PR_DISASSOCIATE_REQUEST = 6;
}

//
// IEEE 802.11 confirm codes, sent by the management entity to the agent,
// in response to the agents's requests.
// These constants should be set as message kind on the messages.
//
// @see ~Ieee80211PrimRequestCode, ~Ieee80211MgmtSta, ~Ieee80211AgentSta
//
enum Ieee80211PrimConfirmCode
{

    PR_SCAN_CONFIRM = 1;
    PR_AUTHENTICATE_CONFIRM = 2;
    PR_DEAUTHENTICATE_CONFIRM = 3;
    PR_ASSOCIATE_CONFIRM = 4;
    PR_REASSOCIATE_CONFIRM = 5;
    PR_DISASSOCIATE_CONFIRM = 6;
}

enum Ieee80211BssType
{

    BSSTYPE_ANY = 0;
    BSSTYPE_INFRASTRUCTURE = 1;
    BSSTYPE_INDEPENDENT = 2;
}

//
// Values for the resultCode field in confirm primitives, sent from
// the management entity to the agent.
//
// @see ~Ieee80211PrimConfirmCode, ~Ieee80211MgmtSta, ~Ieee80211AgentSta
//
enum Ieee80211PrimResultCode
{

    PRC_SUCCESS = 0;
    PRC_INVALID_PARAMETERS = 1;
    PRC_TIMEOUT = 2;
    PRC_TOO_MANY_SIMULTANEOUS_REQUESTS = 3;
    PRC_REFUSED = 4;
}

//
// Base class for request primitives
//
class Ieee80211PrimRequest extends cObject
{
}

//
// Base class for confirm primitives
//
class Ieee80211PrimConfirm extends cObject
{
    Ieee80211PrimResultCode resultCode;
}

//
// Losely based on MLME-SCAN.request.
//
class Ieee80211Prim_ScanRequest extends Ieee80211PrimRequest
{
    Ieee80211BssType BSSType; // determines type of BSS's to include in the scan
    MacAddress BSSID; // specific BSSID to scan for (default: any)
    string SSID;      // SSID to scan for SSID (default: any)
    bool activeScan;  // whether to perform active or passive scanning
    simtime_t probeDelay; // delay (in �s) to be used prior to transmitting a Probe frame during active scanning
    int channelList[]; // list of channels to scan (default: all channels)
    simtime_t minChannelTime; // minimum time to spend on each channel when scanning
    simtime_t maxChannelTime; // maximum time to spend on each channel when scanning
}

class Ieee80211Prim_BssDescription extends cObject
{
    int channelNumber;
    MacAddress BSSID;
    string SSID;
    Ieee80211SupportedRatesElement supportedRates;
    simtime_t beaconInterval;

    double rxPower; // received power from AP; not part of the standard
}

//
// Losely based on MLME-SCAN.confirm
// Possible result codes: SUCCESS, INVALID_PARAMETERS
//
class Ieee80211Prim_ScanConfirm extends Ieee80211PrimConfirm
{
    Ieee80211Prim_BssDescription bssList[];
}

//
// Losely based on MLME-AUTHENTICATE.request.
//
// Note: the "authType" parameter (Open System, Shared Key, etc) is omitted.
// The authentication procedure is simulated by this model by exchanging
// a number of "dummy" authentication frames without real contents,
// and it is configured in the AP how many authentication steps it requires.
//
class Ieee80211Prim_AuthenticateRequest extends Ieee80211PrimRequest
{
    MacAddress address;
    simtime_t timeout;
}

//
// Losely based on MLME-AUTHENTICATE.confirm.
// Possible result codes: SUCCESS, INVALID_PARAMETERS, TIMEOUT, TOO_MANY_SIMULTANEOUS_REQUESTS, REFUSED
//
class Ieee80211Prim_AuthenticateConfirm extends Ieee80211PrimConfirm
{
    MacAddress address;
}

//
// Losely based on MLME-DEAUTHENTICATE.request
//
// NOTE: there is no Deauthenticate Confirm, because Deauthenticate Request
// takes place immediately, and nothing can go wrong (there is no reply
// from the AP to wait for)
//
class Ieee80211Prim_DeauthenticateRequest extends Ieee80211PrimRequest
{
    MacAddress address;
    Ieee80211ReasonCode reasonCode;
}

//
// Losely based on MLME-ASSOCIATE.request
//
class Ieee80211Prim_AssociateRequest extends Ieee80211PrimRequest
{
    MacAddress address;
    simtime_t timeout;
}

//
// Losely based on MLME-ASSOCIATE.confirm
// Possible result codes: SUCCESS, INVALID_PARAMETERS, TIMEOUT, REFUSED
//
class Ieee80211Prim_AssociateConfirm extends Ieee80211PrimConfirm
{
    MacAddress address;
}

//
// MLME-REASSOCIATE.request is very similar to MLME-ASSOCIATE.request
//
class Ieee80211Prim_ReassociateRequest extends Ieee80211Prim_AssociateRequest
{
}

//
// MLME-REASSOCIATE.confirm is very similar to MLME-ASSOCIATE.confirm
//
class Ieee80211Prim_ReassociateConfirm extends Ieee80211Prim_AssociateConfirm
{
}

//
// Losely based on MLME-DISASSOCIATE.request.
//
// NOTE: there is no Disassociate Confirm, because Disassociate Request
// takes place immediately, and nothing can go wrong (there is no reply
// from the AP to wait for)
//
class Ieee80211Prim_DisassociateRequest extends Ieee80211PrimRequest
{
    MacAddress address;
    Ieee80211ReasonCode reasonCode;
}

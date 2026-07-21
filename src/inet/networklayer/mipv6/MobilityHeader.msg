//
// Copyright (C) 2007
// Faqir Zarrar Yousaf
// Communication Networks Institute, Dortmund University of Technology (TU Dortmund), Germany.
// Christian Bauer
// Institute of Communications and Navigation, German Aerospace Center (DLR)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

import inet.common.INETDefs;
import inet.common.TlvOptions;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.contract.ipv6.Ipv6Address;
import inet.networklayer.ipv6.Ipv6Header;
import inet.networklayer.ipv6.Ipv6ExtensionHeaders;

namespace inet;

enum MobilityHeaderType
{
    BINDING_REFRESH_REQUEST = 0;
    HOME_TEST_INIT = 1;
    CARE_OF_TEST_INIT = 2;
    HOME_TEST = 3;
    CARE_OF_TEST = 4;
    BINDING_UPDATE = 5;
    BINDING_ACKNOWLEDGEMENT = 6;
    BINDING_ERROR = 7;
}

class MobilityHeader extends FieldsChunk  // extends Ipv6ExtensionHeader // TODO check how to define MobilityHeader as subclass of Ipv6ExtensionHeader
{
    MobilityHeaderType mobilityHeaderType;
}

class BindingUpdate extends MobilityHeader
{
    unsigned int lifetime; // in seconds (serialized in RFC 6275 units of 4 seconds)
    unsigned int sequence;
    bool ackFlag;                //A-Flag (Acknowledge Flag)
    bool homeRegistrationFlag;        //H-Flag (Home Registration Flag)
    bool linkLocalAddressCompatibilityFlag;    //L-Flag (Link Local Address Compatibility Flag)
    bool keyManagementFlag = false;        //K-Flag (Key Management Flag)
    //Mobility Options not defined
    Ipv6Address homeAddressMN;        //As BU is supposed to be sent with Home Address Option, therefore i make this part of BU Msg.

    // for CN communication; value different from 0 shows that it is used
    int bindingAuthorizationData = 0; // 28.08.07 - CB

    //
    // Proxy Mobile IPv6 (RFC 5213). When proxyRegistrationFlag (the P-flag) is
    // set, this Binding Update is a Proxy Binding Update (PBU) sent by a Mobile
    // Access Gateway on behalf of an unmodified mobile node, and it carries the
    // proxy mobility options below. The options are serialized on the wire only
    // when the P-flag is set, so plain MIPv6 BUs are byte-identical to before.
    //
    bool proxyRegistrationFlag = false;        // P-flag (RFC 5213 Section 8.1)
    string mobileNodeIdentifier;               // Mobile Node Identifier option (NAI), RFC 4283
    Ipv6Address homeNetworkPrefix;             // Home Network Prefix option, RFC 5213 Section 8.3
    uint8_t homeNetworkPrefixLength = 0;
    uint8_t handoffIndicator = 0;              // Handoff Indicator option, RFC 5213 Section 8.4
    uint8_t accessTechnologyType = 0;          // Access Technology Type option, RFC 5213 Section 8.5
    uint64_t timestampValue = 0;               // Timestamp option, RFC 5213 Section 8.8 (0 = ordered by sequence number)
}

//Defining the status fields for the Binding Acknowledgement Message
enum BaStatus
{
    BINDING_UPDATE_ACCEPTED = 0;
    BU_ACCEPT_BUT_DISCOVER_PREFIX = 1;
    REASON_UNSPECIFIED = 128;
    ADMINISTRATIVELY_PROHIBITED = 129;
    INSUFFICIENT_RESOURCES = 130;
    HOME_REGISTRATION_NOT_SUPPORTED = 131;
    NOT_HOME_SUBNET = 132;
    NOT_HA_FOR_THIS_MN = 133;
    DAD_FAILED = 134;
    SEQUENCE_NUMBER_OUT_OF_WINDOW = 135;
    EXPIRED_HOME_NONCE_INDEX = 136;
    EXPIRED_CARE_OF_NONCE_INDEX = 137;
    EXPIRED_NONCES = 138;
    REGISTRATION_TYPE_CHANGE_DISALLOWED = 139;
    // Proxy Mobile IPv6 status codes (RFC 5213 Section 8.1.1)
    PROXY_REG_NOT_ENABLED = 152;
    NOT_LMA_FOR_THIS_MOBILE_NODE = 153;
    MAG_NOT_AUTHORIZED_FOR_PROXY_REG = 154;
    NOT_AUTHORIZED_FOR_HOME_NETWORK_PREFIX = 155;
    TIMESTAMP_MISMATCH = 156;
    TIMESTAMP_LOWER_THAN_PREV_ACCEPTED = 157;
    MISSING_HOME_NETWORK_PREFIX_OPTION = 159;
    BCE_PBU_PREFIX_SET_DO_NOT_MATCH = 160;
    MISSING_MN_IDENTIFIER_OPTION = 161;
    MISSING_HANDOFF_INDICATOR_OPTION = 162;
    MISSING_ACCESS_TECH_TYPE_OPTION = 163;
}

//==================Defining Binding Acknowledgement Message===================
class BindingAcknowledgement extends MobilityHeader
{
    BaStatus status;
    unsigned int sequenceNumber;
    unsigned int lifetime; // in seconds (serialized in RFC 6275 units of 4 seconds)
    bool keyManagementFlag = false;//K-Flag (Key Management Flag)
    //Mobility Options not defined

    // for CN communication; value different from 0 shows that it is used
    int bindingAuthorizationData = 0; // 28.08.07 - CB

    //
    // Proxy Mobile IPv6 (RFC 5213). When proxyRegistrationFlag (the P-flag) is
    // set, this is a Proxy Binding Acknowledgement (PBA) and carries the proxy
    // mobility options below (only serialized when the P-flag is set, so plain
    // MIPv6 BAs are byte-identical to before).
    //
    bool proxyRegistrationFlag = false;        // P-flag (RFC 5213 Section 8.1)
    string mobileNodeIdentifier;               // Mobile Node Identifier option (NAI), echoed back
    Ipv6Address homeNetworkPrefix;             // Home Network Prefix option (assigned/confirmed)
    uint8_t homeNetworkPrefixLength = 0;
    uint64_t timestampValue = 0;               // Timestamp option, RFC 5213 Section 8.8
}

//==================Defining Binding Error Status====================
enum BeStatus
{
    UNKNOWN_BINDING_FOR_HOME_ADDRESS_DEST_OPTION = 1;
    UNKNOWN_MH_TYPE = 2; //Unrecognised MH Type
}

//==================Defining Binding Error Message===================
class BindingError extends MobilityHeader
{
    BeStatus status;
    Ipv6Address homeAddress;
    //Mobility Options not defined
}

//=============Message definition of Home Test Init Message ==========
class HomeTestInit extends MobilityHeader
{
    uint64_t homeInitCookie; // 8 octets on the wire (RFC 6275)
}

//=============Message definition of Home Test Message ==========
class HomeTest extends MobilityHeader
{
    uint64_t homeInitCookie; // 8 octets on the wire (RFC 6275)
    uint64_t homeKeyGenToken; // 8 octets on the wire (RFC 6275)
    uint16_t homeNonceIndex;
}

//=============Message definition of Care-of Test Init Message ==========
class CareOfTestInit extends MobilityHeader
{
    uint64_t careOfInitCookie; // 8 octets on the wire (RFC 6275)
}

//=============Message definition of Care-of Test Message ==========
class CareOfTest extends MobilityHeader
{
    uint64_t careOfInitCookie; // 8 octets on the wire (RFC 6275)
    uint64_t careOfKeyGenToken; // 8 octets on the wire (RFC 6275)
    uint16_t careOfNonceIndex;
}

///////////////////////////////////////////
// 18.09.07 - CB
///////////////////////////////////////////
class BindingRefreshRequest extends MobilityHeader
{
}

///////////////////////////////////////////
// 17.10.07 - CB
///////////////////////////////////////////
class HomeAddressOption extends TlvOptionBase
{
    type = IPv6TLVOPTION_HOME_ADDRESS;
    Ipv6Address homeAddress;
}

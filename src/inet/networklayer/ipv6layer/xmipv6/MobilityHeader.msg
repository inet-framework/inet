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
    unsigned int lifetime;
    unsigned int sequence;
    bool ackFlag;                //A-Flag (Acknowledge Flag)
    bool homeRegistrationFlag;        //H-Flag (Home Registration Flag)
    bool linkLocalAddressCompatibilityFlag;    //L-Flag (Link Local Address Compatibility Flag)
    bool keyManagementFlag = false;        //K-Flag (Key Management Flag)
    //Mobility Options not defined
    Ipv6Address homeAddressMN;        //As BU is supposed to be sent with Home Address Option, therefore i make this part of BU Msg.

    // for CN communication; value different from 0 shows that it is used
    int bindingAuthorizationData = 0; // 28.08.07 - CB
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
}

//==================Defining Binding Acknowledgement Message===================
class BindingAcknowledgement extends MobilityHeader
{
    BaStatus status;
    unsigned int sequenceNumber;
    unsigned int lifetime;
    bool keyManagementFlag = false;//K-Flag (Key Management Flag)
    //Mobility Options not defined

    // for CN communication; value different from 0 shows that it is used
    int bindingAuthorizationData = 0; // 28.08.07 - CB
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
    unsigned int homeInitCookie;
}

//=============Message definition of Home Test Message ==========
class HomeTest extends MobilityHeader
{
    unsigned int homeInitCookie;
    unsigned int homeKeyGenToken;
    // home nonce index not modelled
}

//=============Message definition of Care-of Test Init Message ==========
class CareOfTestInit extends MobilityHeader
{
    unsigned int careOfInitCookie;
}

//=============Message definition of Care-of Test Message ==========
class CareOfTest extends MobilityHeader
{
    unsigned int careOfInitCookie;
    unsigned int careOfKeyGenToken;
    // care-of nonce index not modelled
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
class HomeAddressOption extends Ipv6DestinationOptionsHeader
{
    Ipv6Address homeAddress;
}

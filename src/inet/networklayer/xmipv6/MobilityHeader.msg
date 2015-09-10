//
// Copyright (C) 2007
// Faqir Zarrar Yousaf
// Communication Networks Institute, Dortmund University of Technology (TU Dortmund), Germany.
// Christian Bauer
// Institute of Communications and Navigation, German Aerospace Center (DLR)
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


cplusplus {{
#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/ipv6/IPv6Address.h"
#include "inet/networklayer/ipv6/IPv6Datagram.h" // added by CB
#include "inet/networklayer/ipv6/IPv6ExtensionHeaders.h" // 17.10.07 - CB
}}

namespace inet;

class noncobject IPv6Address;

class noncobject IPv6ExtensionHeader;

class noncobject IPv6DestinationOptionsHeader;

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

packet MobilityHeader // extends IPv6ExtensionHeader // TODO check how to define MobilityHeader as subclass of IPv6ExtensionHeader
{
    int mobilityHeaderType @enum(MobilityHeaderType);
}


packet BindingUpdate extends MobilityHeader
{
    unsigned int lifetime;
    unsigned int sequence;
    bool ackFlag;                //A-Flag (Acknowledge Flag)
    bool homeRegistrationFlag;        //H-Flag (Home Registration Flag)
    bool linkLocalAddressCompatibilityFlag;    //L-Flag (Link Local Address Compatibility Flag)
    bool keyManagementFlag = false;        //K-Flag (Key Management Flag)
    //Mobility Options not defined
    IPv6Address homeAddressMN;        //As BU is supposed to be sent with Home Address Option, therefore i make this part of BU Msg.

    // for CN communication; value different from 0 shows that it is used
    int bindingAuthorizationData = 0; // 28.08.07 - CB
}


//Defining the status fields for the Binding Acknowledgement Message

enum BAStatus
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

packet BindingAcknowledgement extends MobilityHeader
{
    int status @enum(BAStatus);
    unsigned int sequenceNumber;
    unsigned int lifetime;
    bool keyManagementFlag = false;//K-Flag (Key Management Flag)
    //Mobility Options not defined

    // for CN communication; value different from 0 shows that it is used
    int bindingAuthorizationData = 0; // 28.08.07 - CB
}

//==================Defining Binding Error Status====================

enum BEStatus
{

    UNKNOWN_BINDING_FOR_HOME_ADDRESS_DEST_OPTION = 1;
    UNKNOWN_MH_TYPE = 2; //Unrecognised MH Type
}

//==================Defining Binding Error Message===================

packet BindingError extends MobilityHeader
{
    int status @enum(BEStatus);
    IPv6Address homeAddress;
    //Mobility Options not defined

}

//=============Message definition of Home Test Init Message ==========

packet HomeTestInit extends MobilityHeader
{
    unsigned int homeInitCookie;
}

//=============Message definition of Home Test Message ==========

packet HomeTest extends MobilityHeader
{
    unsigned int homeInitCookie;
    unsigned int homeKeyGenToken;
    // home nonce index not modelled
}

//=============Message definition of Care-of Test Init Message ==========

packet CareOfTestInit extends MobilityHeader
{
    unsigned int careOfInitCookie;
}

//=============Message definition of Care-of Test Message ==========

packet CareOfTest extends MobilityHeader
{
    unsigned int careOfInitCookie;
    unsigned int careOfKeyGenToken;
    // care-of nonce index not modelled
}


///////////////////////////////////////////
// 18.09.07 - CB
///////////////////////////////////////////
packet BindingRefreshRequest extends MobilityHeader
{
}


///////////////////////////////////////////
// 17.10.07 - CB
///////////////////////////////////////////
packet HomeAddressOption extends IPv6DestinationOptionsHeader
{
    @customize(true);
    IPv6Address homeAddress;
}

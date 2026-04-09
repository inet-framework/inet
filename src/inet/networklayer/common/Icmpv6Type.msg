//
// Copyright (C) 2001 CTIE, Monash University
// Copyright (C) 2005 OpenSim Ltd.
// Copyright (C) 2005 Wei Yang, Ng
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;

namespace inet;

enum Icmpv6Type
{

    ICMPv6_UNSPECIFIED = 0;
    ICMPv6_DESTINATION_UNREACHABLE = 1;
    ICMPv6_PACKET_TOO_BIG = 2;
    ICMPv6_TIME_EXCEEDED = 3;
    ICMPv6_PARAMETER_PROBLEM = 4;
    ICMPv6_ECHO_REQUEST = 128;
    ICMPv6_ECHO_REPLY = 129;
    ICMPv6_MLD_QUERY = 130;
    ICMPv6_MLD_REPORT = 131;
    ICMPv6_MLD_DONE = 132;
    ICMPv6_ROUTER_SOL = 133;
    ICMPv6_ROUTER_AD = 134;
    ICMPv6_NEIGHBOUR_SOL = 135;
    ICMPv6_NEIGHBOUR_AD = 136;
    ICMPv6_REDIRECT = 137;
    ICMPv6_MLDv2_REPORT = 143;
    ICMPv6_EXPERIMENTAL_MOBILITY = 150;  //Zarrar Yousaf 02.08.07 (FMIPv6 Implementation)
}

//
// ICMPv6 "codes" for type ICMPv6_DESTINATION_UNREACHABLE
//
enum Icmpv6DestUnav
{

    NO_ROUTE_TO_DEST = 0;
    COMM_WITH_DEST_PROHIBITED = 1;
    //2 - NOT ASSIGNED
    ADDRESS_UNREACHABLE = 3;
    PORT_UNREACHABLE = 4;
}

//
// ICMPv6 "codes" for type ICMPv6_TIME_EXCEEDED
//
enum Icmpv6TimeEx
{

    ND_HOP_LIMIT_EXCEEDED = 0;
    ND_FRAGMENT_REASSEMBLY_TIME = 1;
}

//
// ICMPv6 "codes" for type ICMPv6_PARAMETER_PROBLEM
//
enum Icmpv6ParameterProblem
{

    ERROREOUS_HDR_FIELD = 0;
    UNRECOGNIZED_NEXT_HDR_TYPE = 1;
    UNRECOGNIZED_IPV6_OPTION = 2;
}

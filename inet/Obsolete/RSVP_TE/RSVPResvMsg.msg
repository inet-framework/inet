//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//


cplusplus {{
#include "IntServ_m.h"
#include "RSVPPacket.h"
}}

struct RsvpHopObj_t;
struct FlowDescriptor_t;
class noncobject IPADDR;

class RSVPPacket;


//
// \RSVP RESV message
//
// <code>
//  <Resv Message> ::= <Common Header> [ <INTEGRITY> ]
//                     <SESSION>  <RSVP_HOP>
//                     <TIME_VALUES>
//                     [ <RESV_CONFIRM> ]  [ <SCOPE> ]
//                     [ <POLICY_DATA> ... ]
//                     <STYLE> <flow descriptor list>
// </code>
//
message RSVPResvMsg extends RSVPPacket
{
    properties:
        customize=true;
    fields:
        RsvpHopObj_t hop;
        double refreshTime;
        int style; //Support FF style only
        FlowDescriptor_t flow_descriptor_list[InLIST_SIZE];
}


//
// \RSVP RESV TEAR message
//
// <code>
// <ResvTear Message> ::= <Common Header> [<INTEGRITY>]
//                        <SESSION> <RSVP_HOP>
//                        [ <SCOPE> ] <STYLE>
//                        <flow descriptor list>
// </code>
//
message RSVPResvTear extends RSVPPacket
{
    properties:
        customize=true;
    fields:
        RsvpHopObj_t hop;
        FlowDescriptor_t flow_descriptor_list[InLIST_SIZE];
};

//
// \RSVP RESV ERROR message
//
// <code>
//    <ResvErr Message> ::= <Common Header> [ <INTEGRITY> ]
//                          <SESSION>  <RSVP_HOP>
//                          <ERROR_SPEC>  [ <SCOPE> ]
//                          [ <POLICY_DATA> ...]
//                          <STYLE> [ <error flow descriptor>
// </code>
//
message RSVPResvError extends RSVPPacket
{
    properties:
        customize=true;
    fields:
        RsvpHopObj_t hop;
        IPADDR errorNode;
        int errorCode;
};



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
#include "intserv.h"
#include "RSVPPacket.h"
}}

struct RsvpHopObj_t;
struct SenderTemplateObj_t;
struct LabelRequestObj_t;
struct SenderDescriptor_t;
struct FlowDescriptor_t;
struct EroObj_t;

class RSVPPacket;


//
// RSVP PATH message
//
// <code>
//    <Path Message> ::=       <Common Header> [ <INTEGRITY> ]
//                             <SESSION> <RSVP_HOP>
//                             <TIME_VALUES>
//                             [ <EXPLICIT_ROUTE> ]
//                             <LABEL_REQUEST>
//                             [ <SESSION_ATTRIBUTE> ]
//                             [ <POLICY_DATA> ... ]
//                             <sender descriptor>
//
//    <sender descriptor> ::=  <SENDER_TEMPLATE> <SENDER_TSPEC>
//                             [ <ADSPEC> ]
//                             [ <RECORD_ROUTE> ]
// </code>
//
message RSVPPathMsg extends RSVPPacket
{
    properties:
        customize=true;
    fields:
        RsvpHopObj_t rsvp_hop;
        double refresh_time;
        EroObj_t ERO[MAX_ROUTE];
        LabelRequestObj_t label_request;
        SenderDescriptor_t sender_descriptor;
        bool hasERO;
};

//
// PATH TEAR message
//
// <code>
//   <PathTear Message> ::= <Common Header> [ <INTEGRITY> ]
//                          <SESSION> <RSVP_HOP>
//                          [ <sender descriptor> ]
// </code>
//
message RSVPPathTear extends RSVPPacket
{
    properties:
        customize=true;
    fields:
        RsvpHopObj_t rsvp_hop;
        SenderTemplateObj_t senderTemplate;
        //SenderDescriptor_t sender_descriptor;
};


//
// PATH ERROR message
//
// <code>
//   <PathErr message> ::= <Common Header> [ <INTEGRITY> ]
//                         <SESSION> <ERROR_SPEC>
//                         [ <POLICY_DATA> ...]
//                         [ <sender descriptor> ]
// </code>
//
message RSVPPathError extends RSVPPacket
{
    properties:
        customize=true;
    fields:
        int errorNode;
        int errorCode;
        SenderDescriptor_t sender_descriptor;
};


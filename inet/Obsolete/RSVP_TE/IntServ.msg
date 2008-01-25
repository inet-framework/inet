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


//
// This file defines data structures used in IntServ services
//

cplusplus {{
#include "IPAddress.h"

#define MAX_ROUTE 10
#define InLIST_SIZE 5

typedef uint32 IPADDR;  //FIXME this definition should go!!!  IPADDR should be IPAddress
//typedef IPAddress IPADDR;
}}
class noncobject IPADDR;

class noncobject IPAddress;

//
// Intserv/RSVP: Session Structure
//
struct SessionObj_t
{
  fields:
    IPADDR DestAddress;
    int Protocol_Id;
    int DestPort;
    int setupPri;
    int holdingPri;
    int Tunnel_Id;
    int Extended_Tunnel_Id;
};

//
// Intserv/RSVP: RSVP HOP Structure
//
struct RsvpHopObj_t
{
  fields:
    IPADDR Next_Hop_Address;
    int Logical_Interface_Handle;
};

//
// Intserv/RSVP: Sender Template Structure
//
struct SenderTemplateObj_t
{
  fields:
    IPADDR SrcAddress; //= 0; FIXME
    int SrcPort = 0;
    int Lsp_Id = -1;
};


//
// Intserv/RSVP: Sender Tspec Structure
//
struct SenderTspecObj_t
{
  fields:
    double req_bandwidth = 0;
    double link_delay = 0;
};

//
// Intserv/RSVP: Sender Tspec Structure
//
struct FlowSpecObj_t extends SenderTspecObj_t  {fields:}

//
// Intserv/RSVP: Sender Template Structure
//
struct FilterSpecObj_t extends SenderTemplateObj_t {fields:}

//
// Intserv/RSVP: Label Request Object Structure
//
struct LabelRequestObj_t
{
  fields:
    // request;
    int prot;
};

//
// Intserv/RSVP: Sender Descriptor Structure
//
struct SenderDescriptor_t
{
  fields:
    SenderTemplateObj_t Sender_Template_Object;
    SenderTspecObj_t Sender_Tspec_Object;
};


//
// Intserv/RSVP: Flow Descriptor Structure
//
struct FlowDescriptor_t
{
  fields:
    FlowSpecObj_t Flowspec_Object;
    FilterSpecObj_t Filter_Spec_Object;
    IPADDR RRO[MAX_ROUTE];
    int label;
};

//
// Intserv/RSVP: Explicit Routing Object Structure
//
struct EroObj_t
{
  fields:
    bool L = false;
    IPADDR node; // = 0;FIXME
};





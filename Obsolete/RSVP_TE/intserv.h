/*******************************************************************
*
*    This library is free software, you can redistribute it
*    and/or modify
*    it under  the terms of the GNU Lesser General Public License
*    as published by the Free Software Foundation;
*    either version 2 of the License, or any later version.
*    The library is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*    See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/

/*
*    File Name IntServ.h
*    RSVP-TE library
*    This file defines data structures used in IntServ services
**/
#ifndef __intserv_h__
#define __intserv_h__

#include "omnetpp.h"
#include <string.h>

#define MAX_ROUTE 10
#define InLIST_SIZE 5
#define ON     1
#define OFF     0

/**
 * Intserv/RSVP: Session Structure
 */
struct SessionObj_t
{
    int  DestAddress;
    int Protocol_Id;
    int DestPort;
    int setupPri;
    int holdingPri;
    int Tunnel_Id;
    int Extended_Tunnel_Id;
};

/**
 * Intserv/RSVP: RSVP HOP Structure
 */
struct RsvpHopObj_t
{
    int  Next_Hop_Address;
    int  Logical_Interface_Handle;
};

/**
 * Intserv/RSVP: Sender Template Structure
 */
struct SenderTemplateObj_t
{
    SenderTemplateObj_t(){SrcAddress =0; SrcPort=0; Lsp_Id =-1;}
    int  SrcAddress;
    int SrcPort;
    int Lsp_Id;
};


/**
 * Intserv/RSVP: Sender Tspec Structure
 */
struct SenderTspecObj_t
{
    SenderTspecObj_t(){req_bandwidth =0; link_delay=0;}
    double req_bandwidth;
    double link_delay;
};

/**
 * Intserv/RSVP: Sender Tspec Structure
 */
typedef SenderTspecObj_t FlowSpecObj_t;

/**
 * Intserv/RSVP: Sender Template Structure
 */
typedef SenderTemplateObj_t FilterSpecObj_t;

/**
 * Intserv/RSVP: Label Request Object Structure
 */
struct LabelRequestObj_t
{
    // request;
    int prot;
};

/**
 * Intserv/RSVP: Sender Descriptor Structure
 */
struct SenderDescriptor_t
{
  SenderTemplateObj_t Sender_Template_Object;
  SenderTspecObj_t Sender_Tspec_Object;
};


/**
 * Intserv/RSVP: Flow Descriptor Structure
 */
struct FlowDescriptor_t
{
  FlowSpecObj_t Flowspec_Object;
  FilterSpecObj_t Filter_Spec_Object;
  int RRO[MAX_ROUTE];
  int label;
};

/**
 * Intserv/RSVP: Explicit Routing Object Structure
 */
struct EroObj_t
{
    EroObj_t(){L=false;node =0;}
    bool L;
    int node;
};

#endif



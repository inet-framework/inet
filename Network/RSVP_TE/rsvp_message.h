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

#ifndef __rsvpmmmesg_h__
#define __rsvpmmmesg_h__

#include "intserv.h"
#include "IPAddress.h"
#include "RSVPPacket.h"
#include "RSVPPathMsg.h"
#include "RSVPResvMsg.h"


/**
 * RESV TEAR packet
 *
 * <code>
 * <ResvTear Message> ::= <Common Header> [<INTEGRITY>]
 *                                        <SESSION> <RSVP_HOP>
 *                                        [ <SCOPE> ] <STYLE>
 *                                        <flow descriptor list>
 * </code>
 */
class RSVPResvTear : public RSVPPacket
{
protected:
    RsvpHopObj_t rsvp_hop;
    FlowDescriptor_t flow_descriptor_list[InLIST_SIZE];

public:
    RSVPResvTear();
    inline FlowDescriptor_t* getFlowDescriptorList() {return flow_descriptor_list;}

    inline int getNHOP() {return rsvp_hop.Next_Hop_Address;}
    inline int getLIH() {return rsvp_hop.Logical_Interface_Handle;}

    inline FlowDescriptor_t* getFlowDescriptor() {return flow_descriptor_list;}

    inline void setFlowDescriptor(FlowDescriptor_t* f)
    {
        for(int i=0; i< InLIST_SIZE; i++)
            flow_descriptor_list[i]= *(f+i);
    }

    void setHop(RsvpHopObj_t* h);
    void print();
};


/**
 * RESV ERROR packet
 *
 * <code>
 *    <ResvErr Message> ::= <Common Header> [ <INTEGRITY> ]
 *                              <SESSION>  <RSVP_HOP>
 *                              <ERROR_SPEC>  [ <SCOPE> ]
 *                              [ <POLICY_DATA> ...]
 *                              <STYLE> [ <error flow descriptor>
 * </code>
 */
class RSVPResvError : public RSVPPacket
{
protected:
    RsvpHopObj_t rsvp_hop;
    int errorNode;
    int errorCode;

public:
    RSVPResvError();
    inline int getNHOP() {return rsvp_hop.Next_Hop_Address;}
    inline int getLIH() {return rsvp_hop.Logical_Interface_Handle;}

    inline int getErrorNode() {return errorNode;}
    inline void setErrorNode(int i) {errorNode =i;}
    inline int getErrorCode() {return errorCode;}
    inline void setErrorCode(int i) {errorCode =i;}

    void setHop(RsvpHopObj_t* h);
};

#endif




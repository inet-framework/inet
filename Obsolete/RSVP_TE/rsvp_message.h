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
 * PATH TEAR packet
 *
 * <code>
 *   <PathTear Message> ::= <Common Header> [ <INTEGRITY> ]
 *                           <SESSION> <RSVP_HOP>
 *                           [ <sender descriptor> ]
 * </code>
 */
class RSVPPathTear : public RSVPPacket
{
protected:
    RsvpHopObj_t rsvp_hop;
    SenderTemplateObj_t senderTemplate;
    //SenderDescriptor_t sender_descriptor;

public:
    RSVPPathTear();

    inline int getNHOP() {return rsvp_hop.Next_Hop_Address;}
    inline int getLIH() {return rsvp_hop.Logical_Interface_Handle;}
    inline int getSrcAddress() {return senderTemplate.SrcAddress;}
    inline int getSrcPort() {return senderTemplate.SrcPort;}

    inline SenderTemplateObj_t* getSenderTemplate() {return &senderTemplate;}

    void setSenderTemplate(SenderTemplateObj_t* s);
    bool equalST(SenderTemplateObj_t* s);

    void setHop(RsvpHopObj_t* h);
    void print();
};


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
 * PATH ERROR packet
 *
 * <code>
 *   <PathErr message> ::= <Common Header> [ <INTEGRITY> ]
 *                                     <SESSION> <ERROR_SPEC>
 *                                     [ <POLICY_DATA> ...]
 *                                     [ <sender descriptor> ]
 * </code>
 */
class RSVPPathError : public RSVPPacket
{
protected:
    int errorNode;
    int errorCode;
    SenderDescriptor_t sender_descriptor;

public:
    RSVPPathError();

    inline int getErrorNode() {return errorNode;}
    inline void setErrorNode(int i) {errorNode =i;}
    inline int getErrorCode() {return errorCode;}
    inline void setErrorCode(int i) {errorCode =i;}
    inline int getSrcAddress() {return sender_descriptor.Sender_Template_Object.SrcAddress;}
    inline int getSrcPort() {return sender_descriptor.Sender_Template_Object.SrcPort;}
    inline int getLspId() {return sender_descriptor.Sender_Template_Object.Lsp_Id;}
    inline double getDelay() {return sender_descriptor.Sender_Tspec_Object.link_delay;}
    inline double getBW() {return sender_descriptor.Sender_Tspec_Object.req_bandwidth;}
    inline SenderTemplateObj_t* getSenderTemplate() {return &sender_descriptor.Sender_Template_Object;}
    inline SenderTspecObj_t* getSenderTspec() {return &sender_descriptor.Sender_Tspec_Object;}
    bool equalST(SenderTemplateObj_t* s);
    bool equalSD(SenderDescriptor_t* s);
    void setSenderTemplate(SenderTemplateObj_t* s);
    void setSenderTspec(SenderTspecObj_t* s);
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




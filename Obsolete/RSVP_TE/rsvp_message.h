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
#include "RSVPPacket_m.h"


#define PATH_MESSAGE 1
#define RESV_MESSAGE 2
#define PTEAR_MESSAGE 3
#define RTEAR_MESSAGE 4
#define PERROR_MESSAGE 5
#define RERROR_MESSAGE 6

#define FF_STYLE    1
#define SF_STYLE    2



/**
 * RSVP generic packet
 */
class RSVPPacket: public RSVPPacket_Base
{
//protected:
//    SessionObj_t session;
//    int  _rsvpLength;
//    bool _checksumValid;

public:
    RSVPPacket(const char *name=NULL, int kind=0) : RSVPPacket_Base(name,kind) {}
    RSVPPacket(const RSVPPacket& other) : RSVPPacket_Base(other.name()) {operator=(other);}
    RSVPPacket& operator=(const RSVPPacket& other) {RSVPPacket_Base::operator=(other); return *this;}
    virtual cObject *dup() {return new RSVPPacket(*this);}

    inline int getDestAddress() {return getSession().DestAddress;}
    inline int getProtId()      {return getSession().Protocol_Id;}
    inline int getDestPort()    {return getSession().DestPort;}
    inline int getTunnelId()    {return getSession().Tunnel_Id;}
    inline int getExTunnelId()  {return getSession().Extended_Tunnel_Id;}
    inline int getSetupPri()    {return getSession().setupPri;}
    inline int getHoldingPri()  {return getSession().holdingPri;}
    //inline SessionObj_t* getSession() {return &session;}
    bool  isInSession(SessionObj_t* s);
    //void  setSession(SessionObj_t* s);

    //Overload setLength()
    virtual void setLength(int bitlength);

    //bool checksumValid() const {return _checksumValid;}
    //void setChecksumValidity(bool isValid) {_checksumValid = isValid;}
    //int RSVPLength() const {return _rsvpLength;}
    //void setRSVPLength(int byteLength);
};


/**
 * RSVP PATH message
 *
 * <code>
 *    <Path Message> ::=       <Common Header> [ <INTEGRITY> ]
 *                             <SESSION> <RSVP_HOP>
 *                             <TIME_VALUES>
 *                             [ <EXPLICIT_ROUTE> ]
 *                             <LABEL_REQUEST>
 *                             [ <SESSION_ATTRIBUTE> ]
 *                             [ <POLICY_DATA> ... ]
 *                             <sender descriptor>
 *
 *    <sender descriptor> ::=  <SENDER_TEMPLATE> <SENDER_TSPEC>
 *                             [ <ADSPEC> ]
 *                             [ <RECORD_ROUTE> ]
 * </code>
 */
class RSVPPathMsg : public RSVPPacket
{
protected:
    RsvpHopObj_t rsvp_hop;
    double refresh_time;
    EroObj_t ERO[MAX_ROUTE];
    LabelRequestObj_t label_request;
    SenderDescriptor_t sender_descriptor;
    bool useERO;

public:
    RSVPPathMsg();

    inline int getSrcAddress() {return sender_descriptor.Sender_Template_Object.SrcAddress;}
    inline int getSrcPort() {return sender_descriptor.Sender_Template_Object.SrcPort;}
    inline int getLspId() {return sender_descriptor.Sender_Template_Object.Lsp_Id;}
    inline int getNHOP() {return rsvp_hop.Next_Hop_Address;}
    inline int getLIH() {return rsvp_hop.Logical_Interface_Handle;}
    inline double getDelay() {return sender_descriptor.Sender_Tspec_Object.link_delay;}
    inline double getBW() {return sender_descriptor.Sender_Tspec_Object.req_bandwidth;}
    inline bool hasERO() {return useERO;}
    inline void addERO(bool b) {useERO = b;}
    inline void setLabelRequest(LabelRequestObj_t* l) {label_request.prot = l->prot;}
    inline LabelRequestObj_t* getLabelRequest() {return &label_request;}

    inline RsvpHopObj_t* getHop() {return &rsvp_hop;}
    inline SenderTemplateObj_t* getSenderTemplate() {return &sender_descriptor.Sender_Template_Object;}
    inline SenderTspecObj_t* getSenderTspec() {return &sender_descriptor.Sender_Tspec_Object;}
    inline EroObj_t* getERO() {return ERO;}
    inline void setERO(EroObj_t* ero)
    {
      for(int i=0; i< MAX_ROUTE; i++)
      {
            ERO[i].L= (*(ero+i)).L;
            ERO[i].node =(*(ero+i)).node;
      }
    }

    inline void setRTime(double t) {refresh_time=t;}

    bool equalST(SenderTemplateObj_t* s);
    bool equalSD(SenderDescriptor_t* s);

    void setHop(RsvpHopObj_t* h);
    void setSenderTemplate(SenderTemplateObj_t* s);
    void setSenderTspec(SenderTspecObj_t* s);
    void print();
    void setContent(RSVPPathMsg* pMsg);
};


/**
 * RSVP RESV packet
 *
 * <code>
 *  <Resv Message> ::= <Common Header> [ <INTEGRITY> ]
 *                             <SESSION>  <RSVP_HOP>
 *                             <TIME_VALUES>
 *                             [ <RESV_CONFIRM> ]  [ <SCOPE> ]
 *                             [ <POLICY_DATA> ... ]
 *                             <STYLE> <flow descriptor list>
 * </code>
 */
class RSVPResvMsg : public RSVPPacket
{
protected:
    RsvpHopObj_t rsvp_hop;
    double refresh_time;
    int resv_style; //Support FF style only
    FlowDescriptor_t flow_descriptor_list[InLIST_SIZE];

public:
    RSVPResvMsg();
    inline FlowDescriptor_t* getFlowDescriptorList() {return flow_descriptor_list;}

    inline int getNHOP() {return rsvp_hop.Next_Hop_Address;}
    inline int getLIH() {return rsvp_hop.Logical_Interface_Handle;}
    inline int getStyle() {return resv_style;}
    inline void setStyle(int style) {resv_style = style;}
    inline RsvpHopObj_t* getHop() {return &rsvp_hop;}
    inline FlowDescriptor_t* getFlowDescriptor() {return flow_descriptor_list;}
    inline void setRTime(double t) {refresh_time = t;}
    inline void setFlowDescriptor(FlowDescriptor_t* f)
    {
        for(int i=0; i< InLIST_SIZE; i++)
            flow_descriptor_list[i]= *(f+i);
    }

    void setHop(RsvpHopObj_t* h);
    void setContent(RSVPResvMsg* rMsg);
    void print();
};



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

    inline RsvpHopObj_t* getHop() {return &rsvp_hop;}
    inline SenderTemplateObj_t* getSenderTemplate() {return &senderTemplate;}

    void setSenderTemplate(SenderTemplateObj_t* s);
    bool equalST(SenderTemplateObj_t* s);

    void setHop(RsvpHopObj_t* h);
    void setContent(RSVPPathTear* pMsg);
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

    inline RsvpHopObj_t* getHop() {return &rsvp_hop;}
    inline FlowDescriptor_t* getFlowDescriptor() {return flow_descriptor_list;}

    inline void setFlowDescriptor(FlowDescriptor_t* f)
    {
        for(int i=0; i< InLIST_SIZE; i++)
            flow_descriptor_list[i]= *(f+i);
    }

    void setHop(RsvpHopObj_t* h);
    void setContent(RSVPResvTear* rMsg);
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
    void setContent(RSVPPathError* pMsg);
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

    inline RsvpHopObj_t* getHop() {return &rsvp_hop;}
    inline int getErrorNode() {return errorNode;}
    inline void setErrorNode(int i) {errorNode =i;}
    inline int getErrorCode() {return errorCode;}
    inline void setErrorCode(int i) {errorCode =i;}

    void setHop(RsvpHopObj_t* h);
    void setContent(RSVPResvError* pMsg);
};

#endif




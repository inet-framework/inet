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

#ifndef __RSVPPATHMSG_H__
#define __RSVPPATHMSG_H__

#include "RSVPPathMsg_m.h"


/**
 * RSVP PATH message
 *
 * This class adds convenience get() and set() methods to the generated
 * base class, but no extra data.
 */
class INET_API RSVPPathMsg : public RSVPPathMsg_Base
{
  public:
    RSVPPathMsg(const char *name=NULL, int kind=PATH_MESSAGE) : RSVPPathMsg_Base(name,kind) {}
    RSVPPathMsg(const RSVPPathMsg& other) : RSVPPathMsg_Base(other.name()) {operator=(other);}
    RSVPPathMsg& operator=(const RSVPPathMsg& other) {RSVPPathMsg_Base::operator=(other); return *this;}
    virtual RSVPPathMsg *dup() {return new RSVPPathMsg(*this);}

    inline IPADDR getSrcAddress() {return getSender_descriptor().Sender_Template_Object.SrcAddress;}
    inline int getSrcPort() {return getSender_descriptor().Sender_Template_Object.SrcPort;}
    inline int getLspId() {return getSender_descriptor().Sender_Template_Object.Lsp_Id;}
    inline IPADDR getNHOP() {return getHop().Next_Hop_Address;}
    inline int getLIH() {return getHop().Logical_Interface_Handle;}
    inline double getDelay() {return getSender_descriptor().Sender_Tspec_Object.link_delay;}
    inline double getBW() {return getSender_descriptor().Sender_Tspec_Object.req_bandwidth;}
    inline bool hasERO() {return getHasERO();}
    inline SenderTemplateObj_t& getSenderTemplate() {return getSender_descriptor().Sender_Template_Object;}
    inline void setSenderTemplate(const SenderTemplateObj_t& s) {getSender_descriptor().Sender_Template_Object = s;}
    inline SenderTspecObj_t& getSenderTspec() {return getSender_descriptor().Sender_Tspec_Object;}
    inline void setSenderTspec(const SenderTspecObj_t& s) {getSender_descriptor().Sender_Tspec_Object = s;}
    inline EroObj_t* getEROArrayPtr() {return ERO_var;} //FIXME
    inline void setEROArray(EroObj_t* ero) {for (int i=0; i<MAX_ROUTE; i++) setERO(i,ero[i]);}
};


/**
 * RSVP PATH TEAR message
 *
 * This class adds convenience get() and set() methods to the generated
 * base class, but no extra data.
 */
class INET_API RSVPPathTear : public RSVPPathTear_Base
{
  public:
    RSVPPathTear(const char *name=NULL, int kind=PTEAR_MESSAGE) : RSVPPathTear_Base(name,kind) {}
    RSVPPathTear(const RSVPPathTear& other) : RSVPPathTear_Base(other.name()) {operator=(other);}
    RSVPPathTear& operator=(const RSVPPathTear& other) {RSVPPathTear_Base::operator=(other); return *this;}
    virtual RSVPPathTear *dup() {return new RSVPPathTear(*this);}

    inline IPADDR getNHOP() {return getHop().Next_Hop_Address;}
    inline int getLIH() {return getHop().Logical_Interface_Handle;}
    inline IPADDR getSrcAddress() {return getSenderTemplate().SrcAddress;}
    inline int getSrcPort() {return getSenderTemplate().SrcPort;}
};


/**
 * RSVP PATH ERROR message
 *
 * This class adds convenience get() and set() methods to the generated
 * base class, but no extra data.
 */
class INET_API RSVPPathError : public RSVPPathError_Base
{
  public:
    RSVPPathError(const char *name=NULL, int kind=PERROR_MESSAGE) : RSVPPathError_Base(name,kind) {}
    RSVPPathError(const RSVPPathError& other) : RSVPPathError_Base(other.name()) {operator=(other);}
    RSVPPathError& operator=(const RSVPPathError& other) {RSVPPathError_Base::operator=(other); return *this;}
    virtual RSVPPathError *dup() {return new RSVPPathError(*this);}

    inline IPADDR getSrcAddress() {return getSender_descriptor().Sender_Template_Object.SrcAddress;}
    inline int getSrcPort() {return getSender_descriptor().Sender_Template_Object.SrcPort;}
    inline int getLspId() {return getSender_descriptor().Sender_Template_Object.Lsp_Id;}
    inline double getDelay() {return getSender_descriptor().Sender_Tspec_Object.link_delay;}
    inline double getBW() {return getSender_descriptor().Sender_Tspec_Object.req_bandwidth;}

    inline SenderTemplateObj_t& getSenderTemplate() {return getSender_descriptor().Sender_Template_Object;}
    inline void setSenderTemplate(const SenderTemplateObj_t& s) {getSender_descriptor().Sender_Template_Object = s;}
    inline SenderTspecObj_t& getSenderTspec() {return getSender_descriptor().Sender_Tspec_Object;}
    inline void setSenderTspec(const SenderTspecObj_t& s) {getSender_descriptor().Sender_Tspec_Object = s;}
};

#endif




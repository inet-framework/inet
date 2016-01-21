//
// (C) 2005 Vojtech Janota
// (C) 2003 Xuan Thang Nguyen
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

#ifndef __INET_RSVPPATHMSG_H
#define __INET_RSVPPATHMSG_H

#include "RSVPPathMsg_m.h"

namespace inet {

/**
 * RSVP PATH message
 *
 * This class adds convenience get() and set() methods to the generated
 * base class, but no extra data.
 */
class INET_API RSVPPathMsg : public RSVPPathMsg_Base
{
  public:
    RSVPPathMsg(const char *name = nullptr, int kind = PATH_MESSAGE) : RSVPPathMsg_Base(name, kind) {}
    RSVPPathMsg(const RSVPPathMsg& other) : RSVPPathMsg_Base(other) {}
    RSVPPathMsg& operator=(const RSVPPathMsg& other) { RSVPPathMsg_Base::operator=(other); return *this; }
    virtual RSVPPathMsg *dup() const override { return new RSVPPathMsg(*this); }

    inline IPv4Address getSrcAddress() { return getSender_descriptor().Sender_Template_Object.SrcAddress; }
    inline int getLspId() { return getSender_descriptor().Sender_Template_Object.Lsp_Id; }
    inline IPv4Address getNHOP() { return getHop().Next_Hop_Address; }
    inline IPv4Address getLIH() { return getHop().Logical_Interface_Handle; }
    inline double getBW() { return getSender_descriptor().Sender_Tspec_Object.req_bandwidth; }
    inline SenderTemplateObj_t& getSenderTemplate() { return getSender_descriptor().Sender_Template_Object; }
    inline void setSenderTemplate(const SenderTemplateObj_t& s) { getSender_descriptor().Sender_Template_Object = s; }
    inline SenderTspecObj_t& getSenderTspec() { return getSender_descriptor().Sender_Tspec_Object; }
    inline void setSenderTspec(const SenderTspecObj_t& s) { getSender_descriptor().Sender_Tspec_Object = s; }
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
    RSVPPathTear(const char *name = nullptr, int kind = PTEAR_MESSAGE) : RSVPPathTear_Base(name, kind) {}
    RSVPPathTear(const RSVPPathTear& other) : RSVPPathTear_Base(other) {}
    RSVPPathTear& operator=(const RSVPPathTear& other) { RSVPPathTear_Base::operator=(other); return *this; }
    virtual RSVPPathTear *dup() const override { return new RSVPPathTear(*this); }

    inline IPv4Address getNHOP() { return getHop().Next_Hop_Address; }
    inline IPv4Address getLIH() { return getHop().Logical_Interface_Handle; }
    inline IPv4Address getSrcAddress() { return getSenderTemplate().SrcAddress; }
    inline int getLspId() { return getSenderTemplate().Lsp_Id; }
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
    RSVPPathError(const char *name = nullptr, int kind = PERROR_MESSAGE) : RSVPPathError_Base(name, kind) {}
    RSVPPathError(const RSVPPathError& other) : RSVPPathError_Base(other) {}
    RSVPPathError& operator=(const RSVPPathError& other) { RSVPPathError_Base::operator=(other); return *this; }
    virtual RSVPPathError *dup() const override { return new RSVPPathError(*this); }

    inline IPv4Address getSrcAddress() { return getSender_descriptor().Sender_Template_Object.SrcAddress; }
    inline int getLspId() { return getSender_descriptor().Sender_Template_Object.Lsp_Id; }
    inline double getBW() { return getSender_descriptor().Sender_Tspec_Object.req_bandwidth; }

    inline SenderTemplateObj_t& getSenderTemplate() { return getSender_descriptor().Sender_Template_Object; }
    inline void setSenderTemplate(const SenderTemplateObj_t& s) { getSender_descriptor().Sender_Template_Object = s; }
    inline SenderTspecObj_t& getSenderTspec() { return getSender_descriptor().Sender_Tspec_Object; }
    inline void setSenderTspec(const SenderTspecObj_t& s) { getSender_descriptor().Sender_Tspec_Object = s; }
};

} // namespace inet

#endif // ifndef __INET_RSVPPATHMSG_H


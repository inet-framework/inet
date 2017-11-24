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
class INET_API RsvpPathMsg : public RsvpPathMsg_Base
{
  public:
    RsvpPathMsg(/* const char *name = nullptr, int kind = PATH_MESSAGE */) : RsvpPathMsg_Base(/* name, kind */) {}
    RsvpPathMsg(const RsvpPathMsg& other) : RsvpPathMsg_Base(other) {}
    RsvpPathMsg& operator=(const RsvpPathMsg& other) { RsvpPathMsg_Base::operator=(other); return *this; }
    virtual RsvpPathMsg *dup() const override { return new RsvpPathMsg(*this); }

    inline Ipv4Address getSrcAddress() { return getSender_descriptor().Sender_Template_Object.SrcAddress; }
    inline int getLspId() const { return getSender_descriptor().Sender_Template_Object.Lsp_Id; }
    inline Ipv4Address getNHOP() { return getHop().Next_Hop_Address; }
    inline Ipv4Address getLIH() { return getHop().Logical_Interface_Handle; }
    inline double getBW() { return getSender_descriptor().Sender_Tspec_Object.req_bandwidth; }
    inline SenderTemplateObj& getMutableSenderTemplate() { return getMutableSender_descriptor().Sender_Template_Object; }
    inline const SenderTemplateObj& getSenderTemplate() const { return getSender_descriptor().Sender_Template_Object; }
    inline void setSenderTemplate(const SenderTemplateObj& s) { getMutableSender_descriptor().Sender_Template_Object = s; }
    inline SenderTspecObj& getMutableSenderTspec() { return getMutableSender_descriptor().Sender_Tspec_Object; }
    inline const SenderTspecObj& getSenderTspec() { return getSender_descriptor().Sender_Tspec_Object; }
    inline void setSenderTspec(const SenderTspecObj& s) { getMutableSender_descriptor().Sender_Tspec_Object = s; }
};

/**
 * RSVP PATH TEAR message
 *
 * This class adds convenience get() and set() methods to the generated
 * base class, but no extra data.
 */
class INET_API RsvpPathTear : public RsvpPathTear_Base
{
  public:
    RsvpPathTear() : RsvpPathTear_Base() {}
    RsvpPathTear(const RsvpPathTear& other) : RsvpPathTear_Base(other) {}
    RsvpPathTear& operator=(const RsvpPathTear& other) { RsvpPathTear_Base::operator=(other); return *this; }
    virtual RsvpPathTear *dup() const override { return new RsvpPathTear(*this); }

    inline Ipv4Address getNHOP() { return getHop().Next_Hop_Address; }
    inline Ipv4Address getLIH() { return getHop().Logical_Interface_Handle; }
    inline Ipv4Address getSrcAddress() { return getSenderTemplate().SrcAddress; }
    inline int getLspId() const { return getSenderTemplate().Lsp_Id; }
};

/**
 * RSVP PATH ERROR message
 *
 * This class adds convenience get() and set() methods to the generated
 * base class, but no extra data.
 */
class INET_API RsvpPathError : public RsvpPathError_Base
{
  public:
    RsvpPathError() : RsvpPathError_Base(/* name, kind */) {}
    RsvpPathError(const RsvpPathError& other) : RsvpPathError_Base(other) {}
    RsvpPathError& operator=(const RsvpPathError& other) { RsvpPathError_Base::operator=(other); return *this; }
    virtual RsvpPathError *dup() const override { return new RsvpPathError(*this); }

    inline Ipv4Address getSrcAddress() { return getSender_descriptor().Sender_Template_Object.SrcAddress; }
    inline int getLspId() { return getSender_descriptor().Sender_Template_Object.Lsp_Id; }
    inline double getBW() { return getSender_descriptor().Sender_Tspec_Object.req_bandwidth; }

    inline SenderTemplateObj& getMutableSenderTemplate() { return getMutableSender_descriptor().Sender_Template_Object; }
    inline const SenderTemplateObj& getSenderTemplate() const { return getSender_descriptor().Sender_Template_Object; }
    inline void setSenderTemplate(const SenderTemplateObj& s) { getMutableSender_descriptor().Sender_Template_Object = s; }
    inline SenderTspecObj& getMutableSenderTspec() { return getMutableSender_descriptor().Sender_Tspec_Object; }
    inline const SenderTspecObj& getSenderTspec() const { return getSender_descriptor().Sender_Tspec_Object; }
    inline void setSenderTspec(const SenderTspecObj& s) { getMutableSender_descriptor().Sender_Tspec_Object = s; }
};

} // namespace inet

#endif // ifndef __INET_RSVPPATHMSG_H


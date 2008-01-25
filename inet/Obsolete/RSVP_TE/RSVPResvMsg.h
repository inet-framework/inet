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

#ifndef __RSVPRESVMSG_H__
#define __RSVPRESVMSG_H__

#include "RSVPResvMsg_m.h"


/**
 * RSVP RESV message
 *
 * This class adds convenience get() and set() methods to the generated
 * base class, but no extra data.
 */
class INET_API RSVPResvMsg : public RSVPResvMsg_Base
{
  public:
    RSVPResvMsg(const char *name=NULL, int kind=RESV_MESSAGE) : RSVPResvMsg_Base(name,kind) {}
    RSVPResvMsg(const RSVPResvMsg& other) : RSVPResvMsg_Base(other.name()) {operator=(other);}
    RSVPResvMsg& operator=(const RSVPResvMsg& other) {RSVPResvMsg_Base::operator=(other); return *this;}
    virtual RSVPResvMsg *dup() {return new RSVPResvMsg(*this);}

    inline IPADDR getNHOP() {return getHop().Next_Hop_Address;}
    inline int getLIH() {return getHop().Logical_Interface_Handle;}
    inline FlowDescriptor_t* getFlowDescriptorList() {return flow_descriptor_list_var;} //FIXME
    inline void setFlowDescriptor(FlowDescriptor_t* f) {for(int i=0; i<InLIST_SIZE; i++) setFlow_descriptor_list(i,f[i]);} // FIXME
};


/**
 * RESV TEAR message
 *
 * This class adds convenience get() and set() methods to the generated
 * base class, but no extra data.
 */
class INET_API RSVPResvTear : public RSVPResvTear_Base
{
  public:
    RSVPResvTear(const char *name=NULL, int kind=RTEAR_MESSAGE) : RSVPResvTear_Base(name,kind) {}
    RSVPResvTear(const RSVPResvTear& other) : RSVPResvTear_Base(other.name()) {operator=(other);}
    RSVPResvTear& operator=(const RSVPResvTear& other) {RSVPResvTear_Base::operator=(other); return *this;}
    virtual RSVPResvTear *dup() {return new RSVPResvTear(*this);}

    inline IPADDR getNHOP() {return getHop().Next_Hop_Address;}
    inline int getLIH() {return getHop().Logical_Interface_Handle;}

    inline FlowDescriptor_t* getFlowDescriptorList() {return flow_descriptor_list_var;} //FIXME
    inline void setFlowDescriptor(FlowDescriptor_t* f) {for(int i=0; i<InLIST_SIZE; i++) setFlow_descriptor_list(i,f[i]);}
};


/**
 * RESV ERROR message
 *
 * This class adds convenience get() and set() methods to the generated
 * base class, but no extra data.
 */
class INET_API RSVPResvError : public RSVPResvError_Base
{
  public:
    RSVPResvError(const char *name=NULL, int kind=RERROR_MESSAGE) : RSVPResvError_Base(name,kind) {}
    RSVPResvError(const RSVPResvError& other) : RSVPResvError_Base(other.name()) {operator=(other);}
    RSVPResvError& operator=(const RSVPResvError& other) {RSVPResvError_Base::operator=(other); return *this;}
    virtual RSVPResvError *dup() {return new RSVPResvError(*this);}

    inline IPADDR getNHOP() {return getHop().Next_Hop_Address;}
    inline int getLIH() {return getHop().Logical_Interface_Handle;}
};

#endif




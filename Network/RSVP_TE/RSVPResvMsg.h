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
 */
class RSVPResvMsg : public RSVPResvMsg_Base
{
  public:
    RSVPResvMsg(const char *name=NULL, int kind=RESV_MESSAGE) : RSVPResvMsg_Base(name,kind) {}
    RSVPResvMsg(const RSVPResvMsg& other) : RSVPResvMsg_Base(other.name()) {operator=(other);}
    RSVPResvMsg& operator=(const RSVPResvMsg& other) {RSVPResvMsg_Base::operator=(other); return *this;}
    virtual cObject *dup() {return new RSVPResvMsg(*this);}

    inline int getNHOP() {return getRsvp_hop().Next_Hop_Address;}
    inline int getLIH() {return getRsvp_hop().Logical_Interface_Handle;}
    inline FlowDescriptor_t* getFlowDescriptorList() {return flow_descriptor_list_var;} //FIXME
    inline void setFlowDescriptor(FlowDescriptor_t* f) {for(int i=0; i<InLIST_SIZE; i++) setFlow_descriptor_list(i,f[i]);} // FIXME
    void print();
};

inline void RSVPResvMsg::print()
{
    int sIP = 0;
    ev << "DestAddr = " << IPAddress(getDestAddress()) << "\n" <<
          "ProtId   = " << getProtId() << "\n" <<
          "DestPort = " << getDestPort() << "\n" <<
          "Next Hop = " << IPAddress(getNHOP()) << "\n" <<
          "LIH      = " << IPAddress(getLIH()) << "\n";

    for (int i = 0; i < InLIST_SIZE; i++)
            if ((sIP = getFlow_descriptor_list(i).Filter_Spec_Object.SrcAddress) != 0)
            {
                ev << "Receiver =" << IPAddress(sIP) <<
                    ",OutLabel=" << getFlow_descriptor_list(i).label <<
                    ", BW=" << getFlow_descriptor_list(i).Flowspec_Object.req_bandwidth <<
                    ", Delay=" << getFlow_descriptor_list(i).Flowspec_Object.link_delay << "\n";
                ev << "RRO={";
                for (int c = 0; c < MAX_ROUTE; c++)
                {
                    int rroEle = getFlow_descriptor_list(i).RRO[c];
                    if (rroEle != 0)
                        ev << IPAddress(rroEle) << ",";
                }
                ev << "}\n";
            }
}

//---

/**
 * RESV TEAR message
 */
class RSVPResvTear : public RSVPResvTear_Base
{
  public:
    RSVPResvTear(const char *name=NULL, int kind=RTEAR_MESSAGE) : RSVPResvTear_Base(name,kind) {}
    RSVPResvTear(const RSVPResvTear& other) : RSVPResvTear_Base(other.name()) {operator=(other);}
    RSVPResvTear& operator=(const RSVPResvTear& other) {RSVPResvTear_Base::operator=(other); return *this;}
    virtual cObject *dup() {return new RSVPResvTear(*this);}

    inline int getNHOP() {return getRsvp_hop().Next_Hop_Address;}
    inline int getLIH() {return getRsvp_hop().Logical_Interface_Handle;}

    inline FlowDescriptor_t* getFlowDescriptorList() {return flow_descriptor_list_var;} //FIXME
    inline void setFlowDescriptor(FlowDescriptor_t* f) {for(int i=0; i<InLIST_SIZE; i++) setFlow_descriptor_list(i,f[i]);}

    void print();
};

inline void RSVPResvTear::print()
{
    int sIP = 0;
    ev << "DestAddr = " << IPAddress(getDestAddress()) << "\n" <<
        "ProtId   = " << getProtId() << "\n" <<
        "DestPort = " << getDestPort() << "\n" <<
        "Next Hop = " << IPAddress(getNHOP()) << "\n" <<
        "LIH      = " << IPAddress(getLIH()) << "\n";

    for (int i = 0; i < InLIST_SIZE; i++)
        if ((sIP = getFlow_descriptor_list(i).Filter_Spec_Object.SrcAddress) != 0)
        {
            ev << "Receiver =" << IPAddress(sIP) <<
                ", BW=" << getFlow_descriptor_list(i).Flowspec_Object.req_bandwidth <<
                ", Delay=" << getFlow_descriptor_list(i).Flowspec_Object.link_delay << "\n";


        }
}

/**
 * RESV ERROR message
 */
class RSVPResvError : public RSVPResvError_Base
{
  public:
    RSVPResvError(const char *name=NULL, int kind=RERROR_MESSAGE) : RSVPResvError_Base(name,kind) {}
    RSVPResvError(const RSVPResvError& other) : RSVPResvError_Base(other.name()) {operator=(other);}
    RSVPResvError& operator=(const RSVPResvError& other) {RSVPResvError_Base::operator=(other); return *this;}
    virtual cObject *dup() {return new RSVPResvError(*this);}

    inline int getNHOP() {return getRsvp_hop().Next_Hop_Address;}
    inline int getLIH() {return getRsvp_hop().Logical_Interface_Handle;}
};


#endif




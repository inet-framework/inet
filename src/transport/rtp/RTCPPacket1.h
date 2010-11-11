//
// Generated file, do not edit! Created by opp_msgc 4.1 from transport/rtp/RTCPPacket1.msg.
//

#ifndef __INET_RTCPPACKET1_H_
#define __INET_RTCPPACKET1_H_

#include <omnetpp.h>

#include "RTCPPacket1_m.h"


class RTCPPacket : public RTCPPacket_Base
{
  public:
    RTCPPacket(const char *name=NULL, int kind=0);
    RTCPPacket(const RTCPPacket& other) : RTCPPacket_Base(other.getName()) {operator=(other);}
    RTCPPacket& operator=(const RTCPPacket& other) {RTCPPacket_Base::operator=(other); return *this;}
    virtual RTCPPacket *dup() const {return new RTCPPacket(*this);}

    // ADD CODE HERE to redefine and implement pure virtual functions from RTCPPacket_Base

    // rtcpLength is the header field length
    // of an rtcp packet
    // in 32 bit words minus one
    virtual int getRtcpLength() const { return (int)(getByteLength() / 4) - 1; }
    virtual void setRtcpLength(int rtcpLength_var) { throw cRuntimeError("Don't use setRtcpLength()!"); };
};

class RTCPCompoundPacket : public RTCPCompoundPacket_Base
{
  public:
    RTCPCompoundPacket(const char *name=NULL, int kind=0);
    RTCPCompoundPacket(const RTCPCompoundPacket& other) : RTCPCompoundPacket_Base(other.getName()) {operator=(other);}
    RTCPCompoundPacket& operator=(const RTCPCompoundPacket& other) {RTCPCompoundPacket_Base::operator=(other); return *this;}
    virtual RTCPCompoundPacket *dup() const {return new RTCPCompoundPacket(*this);}
    // ADD CODE HERE to redefine and implement pure virtual functions from RTCPCompoundPacket_Base
    void addRTCPPacket(RTCPPacket *rtcpPacket);
};

#endif // __INET_RTCPPACKET1_H_

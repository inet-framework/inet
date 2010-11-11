
#ifndef __INET_RTCPPACKET3_H_
#define __INET_RTCPPACKET3_H_

#include <omnetpp.h>

#include "RTCPPacket3_m.h"


class RTCPSenderReportPacket : public RTCPSenderReportPacket_Base
{
  public:
    RTCPSenderReportPacket(const char *name=NULL, int kind=0);
    RTCPSenderReportPacket(const RTCPSenderReportPacket& other) : RTCPSenderReportPacket_Base(other.getName()) {operator=(other);}
    RTCPSenderReportPacket& operator=(const RTCPSenderReportPacket& other) {RTCPSenderReportPacket_Base::operator=(other); return *this;}
    virtual RTCPSenderReportPacket *dup() const {return new RTCPSenderReportPacket(*this);}
    // ADD CODE HERE to redefine and implement pure virtual functions from RTCPSenderReportPacket_Base
};

#endif // _RTCPPACKET3_M_H_

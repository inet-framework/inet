//
// Generated file, do not edit! Created by opp_msgc 4.1 from transport/rtp/RTCPPacket2.msg.
//

#ifndef __INET_RTCPPACKET2_H_
#define __INET_RTCPPACKET2_H_

#include <omnetpp.h>

#include "RTCPPacket2_m.h"


class RTCPReceiverReportPacket : public RTCPReceiverReportPacket_Base
{
  public:
    RTCPReceiverReportPacket(const char *name=NULL, int kind=0);
    RTCPReceiverReportPacket(const RTCPReceiverReportPacket& other) : RTCPReceiverReportPacket_Base(other.getName()) {operator=(other);}
    RTCPReceiverReportPacket& operator=(const RTCPReceiverReportPacket& other) {RTCPReceiverReportPacket_Base::operator=(other); return *this;}
    virtual RTCPReceiverReportPacket *dup() const {return new RTCPReceiverReportPacket(*this);}
    // ADD CODE HERE to redefine and implement pure virtual functions from RTCPReceiverReportPacket_Base
    /**
     * Adds a receiver report to this receiver report packet.
     */
    virtual void addReceptionReport(ReceptionReport *report);
};

class RTCPSDESPacket : public RTCPSDESPacket_Base
{
  public:
    RTCPSDESPacket(const char *name=NULL, int kind=0);
    RTCPSDESPacket(const RTCPSDESPacket& other) : RTCPSDESPacket_Base(other.getName()) {operator=(other);}
    RTCPSDESPacket& operator=(const RTCPSDESPacket& other) {RTCPSDESPacket_Base::operator=(other); return *this;}
    virtual RTCPSDESPacket *dup() const {return new RTCPSDESPacket(*this);}
    // ADD CODE HERE to redefine and implement pure virtual functions from RTCPSDESPacket_Base
    void addSDESChunk(SDESChunk *sdesChunk);
};

 class RTCPByePacket : public RTCPByePacket_Base
{
  public:
    RTCPByePacket(const char *name=NULL, int kind=0);
    RTCPByePacket(const RTCPByePacket& other) : RTCPByePacket_Base(other.getName()) {operator=(other);}
    RTCPByePacket& operator=(const RTCPByePacket& other) {RTCPByePacket_Base::operator=(other); return *this;}
    virtual RTCPByePacket *dup() const {return new RTCPByePacket(*this);}
    // ADD CODE HERE to redefine and implement pure virtual functions from RTCPByePacket_Base
};

#endif // __INET_RTCPPACKET2_H_

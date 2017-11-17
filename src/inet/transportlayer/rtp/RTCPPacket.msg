//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.transportlayer.rtp.reports;

cplusplus{{
#include "inet/transportlayer/rtp/reports.h"
#include "inet/transportlayer/rtp/sdes.h"
}}

class cArray;

namespace inet;

namespace inet::rtp;

enum RTCPPacketType
{
    RTCP_PT_UNDEF =   0; // default value undefined
    RTCP_PT_SR    = 200; // sender report
    RTCP_PT_RR    = 201; // receiver report
    RTCP_PT_SDES  = 202; // source description
    RTCP_PT_BYE   = 203; // bye
};

class RTCPPacket extends FieldsChunk
{
    chunkLength = B(4);
    @customize(true);  // see the generated C++ header for more info
    int8 version = 2;
    bool padding = 0;
    short count = 0;
    short packetType @enum(RTCPPacketType) = RTCP_PT_UNDEF;
    abstract int rtcpLength;
}

cplusplus{{

class INET_API RTCPPacket : public RTCPPacket_Base
{
  public:
    RTCPPacket() : RTCPPacket_Base() {};
    RTCPPacket(const RTCPPacket& other) : RTCPPacket_Base(other) {}
    RTCPPacket& operator=(const RTCPPacket& other) { RTCPPacket_Base::operator=(other); return *this; }
    virtual RTCPPacket *dup() const override { return new RTCPPacket(*this); }

    // ADD CODE HERE to redefine and implement pure virtual functions from RTCPPacket_Base

    // rtcpLength is the header field length
    // of an rtcp packet
    // in 32 bit words minus one
    virtual int getRtcpLength() const override { return (int)(B(getChunkLength()).get() / 4) - 1; }
    virtual void setRtcpLength(int rtcpLength_var) override { throw cRuntimeError("Don't use setRtcpLength()!"); };
};

}}

class RTCPReceiverReportPacket extends RTCPPacket
{
    packetType = RTCP_PT_RR;
    @customize(true);  // see the generated C++ header for more info
    uint32 ssrc = 0;
    cArray receptionReports;
}

cplusplus {{

class INET_API RTCPReceiverReportPacket : public RTCPReceiverReportPacket_Base
{
  public:
    RTCPReceiverReportPacket();
    RTCPReceiverReportPacket(const RTCPReceiverReportPacket& other) : RTCPReceiverReportPacket_Base(other) {}
    RTCPReceiverReportPacket& operator=(const RTCPReceiverReportPacket& other) { RTCPReceiverReportPacket_Base::operator=(other); return *this; }
    virtual RTCPReceiverReportPacket *dup() const override { return new RTCPReceiverReportPacket(*this); }
    // ADD CODE HERE to redefine and implement pure virtual functions from RTCPReceiverReportPacket_Base
    /**
     * Adds a receiver report to this receiver report packet.
     */
    virtual void addReceptionReport(ReceptionReport *report);
};

}}

class RTCPSDESPacket extends RTCPPacket
{
    packetType = RTCP_PT_SDES;
    @customize(true);  // see the generated C++ header for more info
    cArray sdesChunks;
}

cplusplus {{

class INET_API RTCPSDESPacket : public RTCPSDESPacket_Base
{
  public:
    RTCPSDESPacket();
    RTCPSDESPacket(const RTCPSDESPacket& other) : RTCPSDESPacket_Base(other) {}
    RTCPSDESPacket& operator=(const RTCPSDESPacket& other) { RTCPSDESPacket_Base::operator=(other); return *this; }
    virtual RTCPSDESPacket *dup() const override { return new RTCPSDESPacket(*this); }
    // ADD CODE HERE to redefine and implement pure virtual functions from RTCPSDESPacket_Base
    void addSDESChunk(SDESChunk *sdesChunk);
};

}}

class RTCPByePacket extends RTCPPacket
{
    packetType = RTCP_PT_BYE;
    count = 1;
    @customize(true);  // see the generated C++ header for more info
    uint32 ssrc = 0;
}

cplusplus {{

class INET_API RTCPByePacket : public RTCPByePacket_Base
{
  public:
    RTCPByePacket();
    RTCPByePacket(const RTCPByePacket& other) : RTCPByePacket_Base(other) {}
    RTCPByePacket& operator=(const RTCPByePacket& other) { RTCPByePacket_Base::operator=(other); return *this; }
    virtual RTCPByePacket *dup() const override { return new RTCPByePacket(*this); }
    // ADD CODE HERE to redefine and implement pure virtual functions from RTCPByePacket_Base
};

}}

class RTCPSenderReportPacket extends RTCPReceiverReportPacket
{
    packetType = RTCP_PT_SR;
    @customize(true);  // see the generated C++ header for more info
    SenderReport senderReport;
}

cplusplus {{

class INET_API RTCPSenderReportPacket : public RTCPSenderReportPacket_Base
{
  public:
    RTCPSenderReportPacket();
    RTCPSenderReportPacket(const RTCPSenderReportPacket& other) : RTCPSenderReportPacket_Base(other) {}
    RTCPSenderReportPacket& operator=(const RTCPSenderReportPacket& other) { RTCPSenderReportPacket_Base::operator=(other); return *this; }
    virtual RTCPSenderReportPacket *dup() const override { return new RTCPSenderReportPacket(*this); }
    // ADD CODE HERE to redefine and implement pure virtual functions from RTCPSenderReportPacket_Base
};

}}


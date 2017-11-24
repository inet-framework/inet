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

namespace inet;

namespace inet::rtp;

enum RtcpPacketType
{
    RTCP_PT_UNDEF =   0; // default value undefined
    RTCP_PT_SR    = 200; // sender report
    RTCP_PT_RR    = 201; // receiver report
    RTCP_PT_SDES  = 202; // source description
    RTCP_PT_BYE   = 203; // bye
};

class RtcpPacket extends FieldsChunk
{
    chunkLength = B(4);
    @customize;  // see the generated C++ header for more info
    int8 version = 2;
    bool padding = 0;
    short count = 0;
    RtcpPacketType packetType = RTCP_PT_UNDEF;
    abstract int rtcpLength;
}

cplusplus{{

class INET_API RtcpPacket : public RtcpPacket_Base
{
  public:
    RtcpPacket() : RtcpPacket_Base() {};
    RtcpPacket(const RtcpPacket& other) : RtcpPacket_Base(other) {}
    RtcpPacket& operator=(const RtcpPacket& other) { RtcpPacket_Base::operator=(other); return *this; }
    virtual RtcpPacket *dup() const override { return new RtcpPacket(*this); }

    // ADD CODE HERE to redefine and implement pure virtual functions from RTCPPacket_Base

    // rtcpLength is the header field length
    // of an rtcp packet
    // in 32 bit words minus one
    virtual int getRtcpLength() const override { return (int)(B(getChunkLength()).get() / 4) - 1; }
    virtual void setRtcpLength(int rtcpLength_var) override { throw cRuntimeError("Don't use setRtcpLength()!"); };
};

}}

class RtcpReceiverReportPacket extends RtcpPacket
{
    packetType = RTCP_PT_RR;
    @customize;  // see the generated C++ header for more info
    uint32 ssrc = 0;
    cArray receptionReports;
}

cplusplus {{

class INET_API RtcpReceiverReportPacket : public RtcpReceiverReportPacket_Base
{
  public:
    RtcpReceiverReportPacket();
    RtcpReceiverReportPacket(const RtcpReceiverReportPacket& other) : RtcpReceiverReportPacket_Base(other) {}
    RtcpReceiverReportPacket& operator=(const RtcpReceiverReportPacket& other) { RtcpReceiverReportPacket_Base::operator=(other); return *this; }
    virtual RtcpReceiverReportPacket *dup() const override { return new RtcpReceiverReportPacket(*this); }
    // ADD CODE HERE to redefine and implement pure virtual functions from RTCPReceiverReportPacket_Base
    /**
     * Adds a receiver report to this receiver report packet.
     */
    virtual void addReceptionReport(ReceptionReport *report);
};

}}

class RtcpSdesPacket extends RtcpPacket
{
    packetType = RTCP_PT_SDES;
    @customize;  // see the generated C++ header for more info
    cArray sdesChunks;
}

cplusplus {{

class INET_API RtcpSdesPacket : public RtcpSdesPacket_Base
{
  public:
    RtcpSdesPacket();
    RtcpSdesPacket(const RtcpSdesPacket& other) : RtcpSdesPacket_Base(other) {}
    RtcpSdesPacket& operator=(const RtcpSdesPacket& other) { RtcpSdesPacket_Base::operator=(other); return *this; }
    virtual RtcpSdesPacket *dup() const override { return new RtcpSdesPacket(*this); }
    // ADD CODE HERE to redefine and implement pure virtual functions from RTCPSDESPacket_Base
    void addSDESChunk(SdesChunk *sdesChunk);
};

}}

class RtcpByePacket extends RtcpPacket
{
    packetType = RTCP_PT_BYE;
    count = 1;
    @customize;  // see the generated C++ header for more info
    uint32 ssrc = 0;
}

cplusplus {{

class INET_API RtcpByePacket : public RtcpByePacket_Base
{
  public:
    RtcpByePacket();
    RtcpByePacket(const RtcpByePacket& other) : RtcpByePacket_Base(other) {}
    RtcpByePacket& operator=(const RtcpByePacket& other) { RtcpByePacket_Base::operator=(other); return *this; }
    virtual RtcpByePacket *dup() const override { return new RtcpByePacket(*this); }
    // ADD CODE HERE to redefine and implement pure virtual functions from RTCPByePacket_Base
};

}}

class RtcpSenderReportPacket extends RtcpReceiverReportPacket
{
    packetType = RTCP_PT_SR;
    @customize;  // see the generated C++ header for more info
    SenderReport senderReport;
}

cplusplus {{

class INET_API RtcpSenderReportPacket : public RtcpSenderReportPacket_Base
{
  public:
    RtcpSenderReportPacket();
    RtcpSenderReportPacket(const RtcpSenderReportPacket& other) : RtcpSenderReportPacket_Base(other) {}
    RtcpSenderReportPacket& operator=(const RtcpSenderReportPacket& other) { RtcpSenderReportPacket_Base::operator=(other); return *this; }
    virtual RtcpSenderReportPacket *dup() const override { return new RtcpSenderReportPacket(*this); }
    // ADD CODE HERE to redefine and implement pure virtual functions from RTCPSenderReportPacket_Base
};

}}


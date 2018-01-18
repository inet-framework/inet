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
import inet.transportlayer.rtp.Reports;

cplusplus{{
#include "inet/transportlayer/rtp/Sdes.h"
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
    @customize;  // implements rtcpLength getter/setter
    int8 version = 2;
    bool padding = 0;
    short count = 0;
    RtcpPacketType packetType = RTCP_PT_UNDEF;
    abstract int rtcpLength;
}

cplusplus {{

class INET_API RtcpPacket : public RtcpPacket_Base
{
  public:
    RtcpPacket() : RtcpPacket_Base() {};
    RtcpPacket(const RtcpPacket& other) : RtcpPacket_Base(other) {}
    RtcpPacket& operator=(const RtcpPacket& other) { RtcpPacket_Base::operator=(other); return *this; }
    virtual RtcpPacket *dup() const override { return new RtcpPacket(*this); }

    // ADD CODE HERE to redefine and implement pure virtual functions from RtcpPacket_Base

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
    chunkLength = getChunkLength() + B(4);
    uint32 ssrc = 0;
    cArray receptionReports;
}

cplusplus(RtcpReceiverReportPacket) {{
  public:
    /** Adds a receiver report to this receiver report packet. */
    virtual void addReceptionReport(ReceptionReport *report);
}}

class RtcpSdesPacket extends RtcpPacket
{
    packetType = RTCP_PT_SDES;
    cArray sdesChunks;
}

cplusplus(RtcpSdesPacket) {{
  public:
    void addSDESChunk(SdesChunk *sdesChunk);
}}

class RtcpByePacket extends RtcpPacket
{
    packetType = RTCP_PT_BYE;
    chunkLength = getChunkLength() + B(4);
    count = 1;
    uint32 ssrc = 0;
}

class RtcpSenderReportPacket extends RtcpReceiverReportPacket
{
    packetType = RTCP_PT_SR;
    chunkLength = getChunkLength() + B(20);
    SenderReport senderReport;
}


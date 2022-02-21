//
// Copyright (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;

namespace inet::rtp;

//
// Represents an RTP sender report as contained
// in an ~RtcpSenderReportPacket.
//
class SenderReport extends cObject
{
    // The ntp time stamp.
    uint64_t NTPTimeStamp;

    // The rtp time stamp.
    uint32_t RTPTimeStamp;

    // The number of packets sent.
    uint32_t packetCount;

    // The number of (payload) bytes sent.
    uint32_t byteCount;
};

cplusplus(SenderReport) {{
  public:
    /**
     * Writes a short info about this SenderReport into the given string.
     */
    virtual std::string str() const override;

    /**
     * Writes a longer info about this SenderReport into the given stream.
     */
    virtual void dump(std::ostream& os) const;
}}

//
// Rrepresents an RTP receiver report stored
// in an RTPSenderReportPacket or RTPReceiverReport.
//
class ReceptionReport extends cObject
{
    // The ssrc identifier of the sender this ~ReceptionReport is for.
    uint32_t ssrc;

    // The fraction lost.
    uint8_t fractionLost;

    // The number of packets expected minus the number of packets received.
    int packetsLostCumulative;

    // The extended highest sequence number received.
    uint32_t sequenceNumber;

    // The interarrival jitter.
    int jitter;

    // The rtp time stamp of the last ~SenderReport received from this source.
    int lastSR;

    // The delay since the last ~SenderReport from this sender has been
    // received in units of 1/65536 seconds.
    int delaySinceLastSR;
};

cplusplus(ReceptionReport) {{
  public:
    /**
     * Writes a short info about this ReceptionReport into the given string.
     */
    virtual std::string str() const override;

    /**
     * Writes a longer info about this ReceptionReport into the given stream.
     */
    virtual void dump(std::ostream& os) const;
}}

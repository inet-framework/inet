//
// Copyright (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>
// Copyright (C) 2007 Ahmed Ayadi <ahmed.ayadi@sophia.inria.fr>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_RTPRECEIVERINFO_H
#define __INET_RTPRECEIVERINFO_H

#include <cassert>

#include "inet/transportlayer/rtp/RtpParticipantInfo.h"

namespace inet {

namespace rtp {

/**
 * This class, a subclass of RtpParticipantInfo, is used for storing information
 * about other RTP endsystems.
 * This class processes RTP packets, rtcp sender reports and rtcp sdes chunks
 * originating from this endsystem.
 */
class INET_API RtpReceiverInfo : public RtpParticipantInfo
{
  public:
    enum { MAX_INACTIVE_INTERVALS = 5 };
    /**
     * Default constructor.
     */
    RtpReceiverInfo(uint32_t ssrc = 0);

    /**
     * Copy constructor.
     */
    RtpReceiverInfo(const RtpReceiverInfo& receiverInfo);

    /**
     * Destructor.
     */
    virtual ~RtpReceiverInfo();

    /**
     * Assignment operator.
     */
    RtpReceiverInfo& operator=(const RtpReceiverInfo& receiverInfo);

    /**
     * Duplicates this RtpReceiverInfo by calling the copy constructor.
     */
    virtual RtpReceiverInfo *dup() const override;

    /**
     * Extracts information of the given RtpPacket.
     * Also sets _inactiveIntervals to 0.
     */
    virtual void processRTPPacket(Packet *packet, int id, simtime_t arrivalTime) override;

    /**
     * Extracts information of the given SenderReport.
     */
    virtual void processSenderReport(SenderReport *report, simtime_t arrivalTime);

    /**
     * Extracts information of the given SdesChunk.
     */
    virtual void processSDESChunk(const SdesChunk *sdesChunk, simtime_t arrivalTime) override;

    /**
     * Returns a ReceptionReport if this RTP end system is a sender,
     * nullptr otherwise.
     */
    virtual ReceptionReport *receptionReport(simtime_t now) override;

    /**
     * Informs this RtpReceiverInfo that one rtcp interval has past.
     */
    virtual void nextInterval(simtime_t now) override;

    /**
     * Returns true if this RTP end system is regarded active.
     */
    virtual bool isActive();

    /**
     * Returns true if this RTP end system is regarded valid.
     */
    virtual bool isValid();

    /**
     * Returns true if this RTP end system should be deleted from
     * the list of known RTP session participant.
     * This method should be called directly after nextInterval().
     */
    virtual bool toBeDeleted(simtime_t now) override;

  private:
    void copy(const RtpReceiverInfo& other);

  protected:
    /**
     * The sequence number of the first RtpPacket received.
     */
    uint16_t _sequenceNumberBase = 0;

    /**
     * The highest sequence number of an RtpPacket received.
     */
    uint16_t _highestSequenceNumber = 0;

    /**
     * The highest sequence number of an RtpPacket received
     * before the beginning of the current rtcp interval.
     */
    uint32_t _highestSequenceNumberPrior = 0;

    /**
     * The number of sequence number wrap arounds.
     */
    uint32_t _sequenceNumberCycles = 0;

    /**
     * How many RTP packets from this source have been received.
     */
    uint32_t _packetsReceived = 0;

    /**
     * How many RTP packets have been received from this source
     * before the current rtcp interval began.
     */
    uint32_t _packetsReceivedPrior = 0;

    /**
     * The interarrival jitter. See RTP rfc for details.
     */
    simtime_t _jitter;

    /**
     * The output vector for jitter value
     */
//    cOutVector _jitterOutVector;
    /**
     * The output vector for packet lost
     */
//    cOutVector _packetLostOutVector;
    /**
     * The clock rate (in ticks per second) the sender increases the
     * RTP timestamps. It is calculated when two sender reports have
     * been received.
     */
    int _clockRate = 0;

    /**
     * The RTP time stamp of the last SenderReport received from this sender.
     */
    uint32_t _lastSenderReportRTPTimeStamp = 0;

    /**
     * The ntp time stamp of the last SenderReport received from this sender.
     */
    uint64_t _lastSenderReportNTPTimeStamp = 0;

    /**
     * The Rtp time stamp of the last RtpPacket received from this sender.
     * Needed for calculating the jitter.
     */
    uint32_t _lastPacketRTPTimeStamp = 0;

    /**
     * The arrival time of the last RtpPacket received from this sender.
     * Needed for calculating the jitter.
     */
    simtime_t _lastPacketArrivalTime;

    /**
     * The arrival time of the last SenderReport received from this sender.
     */
    simtime_t _lastSenderReportArrivalTime;

    /**
     * The consecutive number of rtcp intervals this rtcp end system
     * hasn't sent anything.
     */
    int _inactiveIntervals = 0;

    /**
     * The time when this RTP end system has been inactive for five
     * consecutive rtcp intervals.
     */
    simtime_t _startOfInactivity = 0.0;

    /**
     * The number of RTP and rtcp packets received from this RTP end system.
     */
    int _itemsReceived = 0;
};

} // namespace rtp

} // namespace inet

#endif


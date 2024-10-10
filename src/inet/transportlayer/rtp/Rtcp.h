//
// Copyright (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>
// Copyright (C) 2007 Ahmed Ayadi <ahmed.ayadi@sophia.inria.fr>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_RTCP_H
#define __INET_RTCP_H

#include "inet/common/IModuleInterfaceLookup.h"
#include "inet/common/lifecycle/LifecycleUnsupported.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/queueing/contract/IPassivePacketSink.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {
namespace rtp {

using namespace inet::queueing;

// Forward declarations:
class RtcpByePacket;
class RtcpCompoundPacket;
class RtcpReceiverReportPacket;
class RtcpSdesPacket;
class RtcpSenderReportPacket;
class RtpInnerPacket;
class RtpPacket;
class RtpParticipantInfo;
class RtpSenderInfo;

/**
 * The class Rtcp is responsible for creating, receiving and
 * processing of rtcp packets. It also keeps track of this and other
 * Rtp end systems.
 */
class INET_API Rtcp : public cSimpleModule, public LifecycleUnsupported, public IPassivePacketSink, public IModuleInterfaceLookup
{
  public:
    Rtcp();
    virtual ~Rtcp();

    virtual bool canPushSomePacket(const cGate *gate) const override { return gate->isName("udpIn"); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return gate->isName("udpIn"); }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }

    virtual cGate *lookupModuleInterface(cGate *gate, const std::type_info& type, const cObject *arguments, int direction) override;

  protected:
    /**
     * Initializes variables.
     */
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /**
     * Message handling. Dispatches messages by arrival gate.
     */
    virtual void handleMessage(cMessage *msg) override;

    /**
     * Handles messages from the Rtp module.
     */
    virtual void handleMessageFromRTP(cMessage *msg);

    /**
     * Handles messages coming from the socket layer.
     */
    virtual void handleMessageFromUDP(cMessage *msg);

    /**
     * Handles self messages.
     */
    virtual void handleSelfMessage(cMessage *msg);

    /**
     * Initializes the rtcp module when the session is started.
     */
    virtual void handleInitializeRTCP(RtpInnerPacket *rinp);

    /**
     * Stores information about the new transmission.
     */
    virtual void handleSenderModuleInitialized(RtpInnerPacket *rinp);

    /**
     * Stores information about an outgoing Rtp data packet.
     */
    virtual void handleDataOut(RtpInnerPacket *packet);

    /**
     * Stores information about an outgoing Rtp data packet.
     */
    virtual void handleDataIn(RtpInnerPacket *rinp);

    /**
     * Makes the rtcp module send an RtcpByePacket in the next
     * RtcpCompoundPacket to tell other participants in the Rtp
     * session that this end system leaves.
     */
    virtual void handleLeaveSession(RtpInnerPacket *rinp);

    /**
     * Called when the socket layer has finished a connect.
     */
    virtual void connectRet();

    /**
     * Called when this rtcp module receives data from the
     * socket layer.
     */
    virtual void readRet(Packet *sifpIn);

    /**
     * Request a server socket from the socket layer.
     */
    virtual void createSocket();

    /**
     * Chooses the ssrc identifier for this end system.
     */
    virtual void chooseSSRC();

    /**
     * Calculates the length of the next rtcp interval an issues
     * a self message to remind itself.
     */
    virtual void scheduleInterval();

    /**
     * Creates and sends an RtcpCompoundPacket.
     */
    virtual void createPacket();

    /**
     * Extracts information of a sent RtpPacket.
     */
    virtual void processOutgoingRTPPacket(Packet *packet);

    /**
     * Extracts information of a received RtpPacket.
     */
    virtual void processIncomingRTPPacket(Packet *packet, Ipv4Address address, int port);

    /**
     * Extracts information of a received RtcpCompoundPacket.
     */
    virtual void processIncomingRTCPPacket(Packet *packet, Ipv4Address address, int port);
    void processIncomingRTCPSenderReportPacket(const Ptr<const RtcpSenderReportPacket>& rtcpSenderReportPacket, Ipv4Address address, int port);
    void processIncomingRTCPReceiverReportPacket(const Ptr<const RtcpReceiverReportPacket>& rtcpReceiverReportPacket, Ipv4Address address, int port);
    void processIncomingRTCPSDESPacket(const Ptr<const RtcpSdesPacket>& rtcpSDESPacket, Ipv4Address address, int port, simtime_t arrivalTime);
    void processIncomingRTCPByePacket(const Ptr<const RtcpByePacket>& rtcpByePacket, Ipv4Address address, int port);

    /**
     * Returns the RtpParticipantInfo object used for storing information
     * about the Rtp end system with this ssrc identifier.
     * Returns nullptr if this end system is unknown.
     */
    virtual RtpParticipantInfo *findParticipantInfo(uint32_t ssrc);

    /**
     * Recalculates the average size of an RtcpCompoundPacket when
     * one of this size has been sent or received.
     */
    virtual void calculateAveragePacketSize(int size);

  protected:
    /**
     * The maximum size an RTCPCompundPacket can have.
     */
    int _mtu = 0;

    /**
     * The bandwidth for this Rtp session.
     */
    int _bandwidth = 0;

    /**
     * The percentage of bandwidth for rtcp.
     */
    int _rtcpPercentage = 0;

    /**
     * The destination address.
     */
    Ipv4Address _destinationAddress;

    /**
     * The rtcp port.
     */
    int _port = -1;

    /**
     * True when this end system has chosen its ssrc identifier.
     */
    bool _ssrcChosen = false;

    /**
     * True when this end system is about to leave the session.
     */
    bool _leaveSession = false;

    /**
     * The RtpSenderInfo about this end system.
     */
    RtpSenderInfo *_senderInfo = nullptr;

    /**
     * Information about all known Rtp end system participating in
     * this Rtp session.
     */
    cArray _participantInfos;

    /**
     * The UDP socket for sending/receiving rtcp packets.
     */
    UdpSocket _udpSocket;

    /**
     * The number of packets this rtcp module has
     * calculated.
     */
    int _packetsCalculated = 0;

    /**
     * The average size of an RtcpCompoundPacket.
     */
    double _averagePacketSize = 0;

    /**
     * The output vector for statistical data about the
     * behaviour of rtcp. Every participant's rtcp module
     * writes its calculated rtcp interval (without variation
     */
//    cOutVector *_rtcpIntervalOutVector;
};

} // namespace rtp
} // namespace inet

#endif


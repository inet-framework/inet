/***************************************************************************
                       RTCP.h  -  description
                             -------------------
    (C) 2007 Ahmed Ayadi  <ahmed.ayadi@sophia.inria.fr>
    (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef __INET_RTCPENDSYSTEMMODULE_H
#define __INET_RTCPENDSYSTEMMODULE_H


#include "INETDefs.h"
#include "IPv4Address.h"
#include "UDPSocket.h"


//Forward declarations:
class RTCPByePacket;
class RTCPCompoundPacket;
class RTCPReceiverReportPacket;
class RTCPSDESPacket;
class RTCPSenderReportPacket;
class RTPInnerPacket;
class RTPPacket;
class RTPParticipantInfo;
class RTPSenderInfo;


/**
 * The class RTCP is responsible for creating, receiving and
 * processing of rtcp packets. It also keeps track of this and other
 * RTP end systems.
 */
class INET_API RTCP : public cSimpleModule
{
  public:
    RTCP();
    virtual ~RTCP();

  protected:
    /**
     * Initializes variables.
     */
    virtual void initialize();

    /**
     * Message handling. Dispatches messages by arrival gate.
     */
    virtual void handleMessage(cMessage *msg);

    /**
     * Handles messages from the RTP module.
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
    virtual void handleInitializeRTCP(RTPInnerPacket *rinp);

    /**
     * Stores information about the new transmission.
     */
    virtual void handleSenderModuleInitialized(RTPInnerPacket *rinp);

    /**
     * Stores information about an outgoing RTP data packet.
     */
    virtual void handleDataOut(RTPInnerPacket *packet);

    /**
     * Stores information about an outgoing RTP data packet.
     */
    virtual void handleDataIn(RTPInnerPacket *rinp);

    /**
     * Makes the rtcp module send an RTCPByePacket in the next
     * RTCPCompoundPacket to tell other participants in the RTP
     * session that this end system leaves.
    */
    virtual void handleLeaveSession(RTPInnerPacket *rinp);

    /**
     * Called when the socket layer has finished a connect.
     */
    virtual void connectRet();

    /**
     * Called when this rtcp module receives data from the
     * socket layer.
     */
    virtual void readRet(cPacket *sifpIn);

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
     * Creates and sends an RTCPCompoundPacket.
     */
    virtual void createPacket();

    /**
     * Extracts information of a sent RTPPacket.
     */
    virtual void processOutgoingRTPPacket(RTPPacket *packet);

    /**
     * Extracts information of a received RTPPacket.
     */
    virtual void processIncomingRTPPacket(RTPPacket *packet, IPv4Address address, int port);

    /**
     * Extracts information of a received RTCPCompoundPacket.
     */
    virtual void processIncomingRTCPPacket(RTCPCompoundPacket *packet, IPv4Address address, int port);
    void processIncomingRTCPSenderReportPacket(
            RTCPSenderReportPacket *rtcpSenderReportPacket, IPv4Address address, int port);
    void processIncomingRTCPReceiverReportPacket(
            RTCPReceiverReportPacket *rtcpReceiverReportPacket, IPv4Address address, int port);
    void processIncomingRTCPSDESPacket(
            RTCPSDESPacket *rtcpSDESPacket, IPv4Address address, int port, simtime_t arrivalTime);
    void processIncomingRTCPByePacket(RTCPByePacket *rtcpByePacket, IPv4Address address, int port);

    /**
     * Returns the RTPParticipantInfo object used for storing information
     * about the RTP end system with this ssrc identifier.
     * Returns NULL if this end system is unknown.
     */
    virtual RTPParticipantInfo* findParticipantInfo(uint32 ssrc);

    /**
     * Recalculates the average size of an RTCPCompoundPacket when
     * one of this size has been sent or received.
     */
    virtual void calculateAveragePacketSize(int size);

  protected:
    /**
     * The maximum size an RTCPCompundPacket can have.
     */
    int _mtu;

    /**
     * The bandwidth for this RTP session.
     */
    int _bandwidth;

    /**
     * The percentage of bandwidth for rtcp.
     */
    int _rtcpPercentage;

    /**
     * The destination address.
     */
    IPv4Address _destinationAddress;

    /**
     * The rtcp port.
     */
    int _port;

    /**
     * True when this end system has chosen its ssrc identifier.
     */
    bool _ssrcChosen;

    /**
     * True when this end system is about to leave the session.
     */
    bool _leaveSession;

    /**
     * The RTPSenderInfo about this end system.
     */
    RTPSenderInfo *_senderInfo;

    /**
     * Information about all known RTP end system participating in
     * this RTP session.
     */
    cArray _participantInfos;

    /**
     * The UDP socket for sending/receiving rtcp packets.
     */
    UDPSocket _udpSocket;

    /**
     * The number of packets this rtcp module has
     * calculated.
     */
    int _packetsCalculated;

    /**
     * The average size of an RTCPCompoundPacket.
     */
    double _averagePacketSize;

    /**
     * The output vector for statistical data about the
     * behaviour of rtcp. Every participant's rtcp module
     * writes its calculated rtcp interval (without variation
     */
//  cOutVector *_rtcpIntervalOutVector;

    //statistics
    static simsignal_t rcvdPkSignal;
};

#endif

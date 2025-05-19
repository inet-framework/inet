//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_APPLICATIONS_QUIC_PACKETBUILDER_H_
#define INET_APPLICATIONS_QUIC_PACKETBUILDER_H_

#include "inet/common/packet/Packet.h"
#include "inet/common/packet/ChunkQueue.h"
#include "scheduler/IScheduler.h"
#include "ReceivedPacketsAccountant.h"
#include "packet/QuicPacket.h"
#include "packet/DplpmtudProbePacket.h"
#include "dplpmtud/Dplpmtud.h"
#include "packet/ConnectionId.h"
#include "TransportParameters.h"

namespace inet {
namespace quic {

class ReceivedPacketsAccountant;
class IScheduler;

class PacketBuilder {
public:
    PacketBuilder(std::vector<QuicFrame*> *controlQueue, IScheduler *scheduler, ReceivedPacketsAccountant *receivedPacketsAccountant[]);
    virtual ~PacketBuilder();

    void readParameters(cModule *module);

    /**
     * Builds a regular 1-RTT packet, that may contain
     * (1) an ack frame,
     * (2) control frames, and/or
     * (3) stream frames.
     *
     * @param maxPacketSize Upper limit for the size of an ack-eliciting packet.
     * @param safePacketSize Upper limit for the size of a non-ack-eliciting packet.
     * @return Pointer to the created QuicHeader object.
     */
    QuicPacket *buildPacket(int maxPacketSize, int safePacketSize);

    QuicPacket *buildAckOnlyPacket(int maxPacketSize, PacketNumberSpace pnSpace);

    /**
     * Builds a packet that is ack eliciting. That means, for now, a packet that include control and/or stream frames.
     *
     * @param maxPacketSize Specifies an upper limit for the packet created by this method.
     * @return A pointer to an ack eliciting packet with new stream data, or nullptr, if no new stream data were available.
     */
    QuicPacket *buildAckElicitingPacket(int maxPacketSize);

    /**
     * Builds a packet that is ack eliciting by resending data from STREAM frames of outstanding packets.
     *
     * @param sentPackets Outstanding packets that should contain a stream frame for the new packet.
     * @param maxPacketSize Upper limit for the size of the new packet.
     * @param skipPacketNumber If true, uses a packet number two larger than the number of the last packet
     * This triggers the receiver to send an ack immediately.
     * @return A pointer to an ack elicting packet with stream data from the given outstanding packets,
     * or nullptr, if there is no retransmitable stream frame in the outstanding packet.
     */
    QuicPacket *buildAckElicitingPacket(std::vector<QuicPacket*> *sentPackets, int maxPacketSize, bool skipPacketNumber=false);

    QuicPacket *buildZeroRttPacket(int maxPacketSize);
    QuicPacket *buildPingPacket();
    QuicPacket *buildDplpmtudProbePacket(int packetSize, Dplpmtud *dplpmtud);
    QuicPacket *buildClientInitialPacket(int maxPacketSize, TransportParameters *tp, uint32_t token);
    QuicPacket *buildServerInitialPacket(int maxPacketSize);
    QuicPacket *buildHandshakePacket(int maxPacketSize, TransportParameters *tp);
    QuicPacket *buildConnectionClosePacket(int maxPacketSize, bool sendAck, bool appInitiated, int errorCode);
    void addHandshakeDone();
    void addNewTokenFrame(uint32_t token);
    void setSrcConnectionId(ConnectionId *connectionId) {
        this->srcConnectionId = connectionId;
    }
    void setDstConnectionId(ConnectionId *connectionId) {
        this->dstConnectionId = connectionId;
    }

private:
    std::vector<QuicFrame*> *controlQueue;
    IScheduler *scheduler;
    ReceivedPacketsAccountant **receivedPacketsAccountant;

    uint64_t packetNumber[3];
    ConnectionId *srcConnectionId = nullptr;
    ConnectionId *dstConnectionId = nullptr;
    bool bundleAckForNonAckElicitingPackets;
    bool skipPacketNumberForDplpmtudProbePackets;

    QuicPacket *createPacket(PacketNumberSpace pnSpace, bool skipPacketNumber, bool zeroRtt = false);
    Ptr<InitialPacketHeader> createInitialHeader();
    Ptr<HandshakePacketHeader> createHandshakeHeader();
    QuicPacket *createOneRttPacket(bool skipPacketNumber=false);
    QuicPacket *createZeroRttPacket();
    Ptr<OneRttPacketHeader> createOneRttHeader();
    Ptr<ZeroRttPacketHeader> createZeroRttHeader();
    QuicPacket *addFramesFromControlQueue(QuicPacket *packet, int maxPacketSize);
    QuicPacket *addFrameToPacket(QuicPacket *packet, QuicFrame *frame, bool skipPacketNumber=false);
    size_t getPacketSize(QuicPacket *packet);
    QuicFrame *createPingFrame();
    QuicFrame *createPaddingFrame(int length = 1);
    QuicFrame *createCryptoFrame(TransportParameters *tp = nullptr);
    void fillLongHeader(Ptr<LongPacketHeader> packetHeader);
    QuicFrame *createHandshakeDoneFrame();
    QuicFrame *createNewTokenFrame(uint32_t token);
    QuicFrame *createConnectionCloseFrame(bool appInitiated, int errorCode);
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_PACKETBUILDER_H_ */

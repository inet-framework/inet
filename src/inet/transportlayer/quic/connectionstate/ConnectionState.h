//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_APPLICATIONS_QUIC_CONNECTIONSTATE_CONNECTIONSTATE_H_
#define INET_APPLICATIONS_QUIC_CONNECTIONSTATE_CONNECTIONSTATE_H_

#include "../Connection.h"
#include "../packet/PacketHeader_m.h"
#include "../packet/FrameHeader_m.h"
#include "../packet/QuicPacket.h"
#include "inet/transportlayer/contract/quic/QuicCommand_m.h"

namespace inet {
namespace quic {

class ConnectionState {
public:
    ConnectionState(Connection *context);
    virtual ~ConnectionState();

    virtual void start();
    virtual ConnectionState *processAppCommand(cMessage *msg);
    virtual ConnectionState *processConnectAppCommand(cMessage *msg);
    virtual ConnectionState *processSendAppCommand(cMessage *msg);
    virtual ConnectionState *processCloseAppCommand(cMessage *msg);
    virtual ConnectionState *processRecvAppCommand(cMessage *msg);
    virtual ConnectionState *processConnectAndSendAppCommand(cMessage *msg);

    virtual ConnectionState *processPacket(Packet *msg);
    virtual ConnectionState *processInitialPacket(const Ptr<const InitialPacketHeader>& packetHeader, Packet *pkt);
    virtual ConnectionState *processHandshakePacket(const Ptr<const HandshakePacketHeader>& packetHeader, Packet *pkt);
    virtual ConnectionState *processZeroRttPacket(const Ptr<const ZeroRttPacketHeader>& packetHeader, Packet *pkt);
    virtual ConnectionState *processOneRttPacket(const Ptr<const OneRttPacketHeader>& packetHeader, Packet *pkt);

    virtual void processFrames(Packet *pkt, PacketNumberSpace pnSpace);
    virtual bool containsFrame(Packet *pkt, enum FrameHeaderType frameType);
    virtual void discardFrames(Packet *pkt);
    virtual void processStreamFrame(const Ptr<const StreamFrameHeader>& frameHeader, Packet *pkt);
    virtual void processAckFrame(const Ptr<const AckFrameHeader>& frameHeader, PacketNumberSpace pnSpace);
    virtual void processFrame(Packet *pkt, PacketNumberSpace pnSpace);
    virtual void processMaxDataFrame(const Ptr<const MaxDataFrameHeader>& frameHeader);
    virtual void processMaxStreamDataFrame(const Ptr<const MaxStreamDataFrameHeader>& frameHeader);
    virtual void processStreamDataBlockedFrame(const Ptr<const StreamDataBlockedFrameHeader>& frameHeader);
    virtual void processDataBlockedFrame(const Ptr<const DataBlockedFrameHeader>& frameHeader);
    virtual void processCryptoFrame(const Ptr<const CryptoFrameHeader>& frameHeader, Packet *pkt);
    virtual void processHandshakeDoneFrame();
    virtual void processNewTokenFrame(const Ptr<const NewTokenFrameHeader>& frameHeader);
    virtual void processConnectionCloseFrame();

    virtual ConnectionState *processIcmpPtb(Packet *droppedPacket, int ptbMtu);
    virtual ConnectionState *processIcmpPtb(uint64_t droppedPacketNumber, int ptbMtu);

    virtual ConnectionState *processTimeout(cMessage *msg);
    virtual ConnectionState *processLossDetectionTimeout(cMessage *msg);
    virtual ConnectionState *processAckDelayTimeout(cMessage *msg);
    virtual ConnectionState *processDplpmtudRaiseTimeout(cMessage *msg);
    virtual ConnectionState *processConnectionCloseTimeout(cMessage *msg);

    virtual EncryptionLevel getEncryptionLevel() = 0;

protected:
    Connection *context;
    std::string name;
    bool ackElicitingPacket;

};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_CONNECTIONSTATE_CONNECTIONSTATE_H_ */

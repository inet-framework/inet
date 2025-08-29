//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_TRANSPORTLAYER_QUIC_CONNECTIONSTATE_ESTABLISHEDCONNECTIONSTATE_H_
#define INET_TRANSPORTLAYER_QUIC_CONNECTIONSTATE_ESTABLISHEDCONNECTIONSTATE_H_

#include "ConnectionState.h"

namespace inet {
namespace quic {

class EstablishedConnectionState: public ConnectionState {
public:
    EstablishedConnectionState(Connection *context) : ConnectionState(context) {
        name = "Established";
    }

    virtual void start() override;
    virtual ConnectionState *processSendAppCommand(cMessage *msg) override;
    virtual ConnectionState *processCloseAppCommand(cMessage *msg) override;
    virtual ConnectionState *processZeroRttPacket(const Ptr<const ZeroRttPacketHeader>& packetHeader, Packet *pkt) override;
    virtual ConnectionState *processOneRttPacket(const Ptr<const OneRttPacketHeader>& packetHeader, Packet *pkt) override;
    virtual void processStreamFrame(const Ptr<const StreamFrameHeader>& frameHeader, Packet *pkt) override;
    virtual void processAckFrame(const Ptr<const AckFrameHeader>& frameHeader, PacketNumberSpace pnSpace) override;
    virtual void processMaxDataFrame(const Ptr<const MaxDataFrameHeader>& frameHeader) override;
    virtual void processMaxStreamDataFrame(const Ptr<const MaxStreamDataFrameHeader>& frameHeader) override;
    virtual void processStreamDataBlockedFrame(const Ptr<const StreamDataBlockedFrameHeader>& frameHeader) override;
    virtual void processDataBlockedFrame(const Ptr<const DataBlockedFrameHeader>& frameHeader) override;
    virtual void processCryptoFrame(const Ptr<const CryptoFrameHeader>& frameHeader, Packet *pkt) override;
    virtual void processHandshakeDoneFrame() override;
    virtual void processNewTokenFrame(const Ptr<const NewTokenFrameHeader>& frameHeader) override;
    virtual void processConnectionCloseFrame() override;
    virtual ConnectionState *processLossDetectionTimeout(cMessage *msg) override;
    virtual ConnectionState *processAckDelayTimeout(cMessage *msg) override;
    virtual ConnectionState *processRecvAppCommand(cMessage *msg) override;
    virtual ConnectionState *processIcmpPtb(uint64_t droppedPacketNumber, int ptbMtu) override;
    virtual ConnectionState *processDplpmtudRaiseTimeout(cMessage *msg) override;
    virtual ConnectionState *processInitialPacket(const Ptr<const InitialPacketHeader>& packetHeader, Packet *pkt) override;
    virtual ConnectionState *processHandshakePacket(const Ptr<const HandshakePacketHeader>& packetHeader, Packet *pkt) override;

    EncryptionLevel getEncryptionLevel() override { return EncryptionLevel::OneRtt; }

private:
    bool gotCryptoFin = false;
    bool gotConnectionClose = false;
    bool processingZeroRttPacket = false;
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_CONNECTIONSTATE_ESTABLISHEDCONNECTIONSTATE_H_ */

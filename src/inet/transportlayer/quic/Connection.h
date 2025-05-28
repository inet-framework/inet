//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_APPLICATIONS_QUIC_CONNECTION_H_
#define INET_APPLICATIONS_QUIC_CONNECTION_H_

#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "Quic.h"
#include "UdpSocket.h"
#include "AppSocket.h"
#include "packet/QuicPacket.h"
#include "connectionstate/ConnectionState.h"
#include "inet/common/packet/ChunkQueue.h"
#include "PacketBuilder.h"
#include "ReceivedPacketsAccountant.h"
#include "ReliabilityManager.h"
#include "Timer.h"
#include "congestioncontrol/ICongestionController.h"
#include "flowcontroller/FlowController.h"
#include "flowcontroller/FlowControlResponder.h"
#include "Statistics.h"
#include "dplpmtud/Dplpmtud.h"
#include "Path.h"
#include "stream/Stream.h"
#include "packet/ConnectionId.h"
#include "TransportParameters.h"

extern "C" {
#include "picotls.h"
#include "picotls/openssl_opp.h"
}

namespace inet {
namespace quic {

// Forward declarations:
class Quic;
class UdpSocket;
class ConnectionState;
class Timer;
class ReliabilityManager;
class ReceivedPacketsAccountant;
class PacketBuilder;
class IScheduler;
class Stream;
class ConnectionFlowControlResponder;
class ICongestionController;
class Path;

enum TimerType
{
    LOSS_DETECTION_TIMER,
    ACK_DELAY_TIMER,
    DPLPMTUD_RAISE_TIMER,
    CONNECTION_CLOSE_TIMER
    //QUIC_T_TIMEOUT_IDLE,
    //QUIC_T_SEND
};


enum class EncryptionLevel {
    Initial = 0,
    ZeroRtt = 1,
    Handshake = 2,
    OneRtt = 3
};

class Connection
{
  public:
    Connection(Quic *quicSimpleMod, bool is_server, UdpSocket *udpSocket, AppSocket *appSocket, L3Address remoteAddr, uint16_t remotePort, uint64_t srcConnectionId);
    virtual ~Connection();
    virtual void processAppCommand(cMessage *msg);
    virtual void processPackets(Packet *pkt);
    virtual void processTimeout(cMessage *msg);
    void sendPackets();

    bool is_server;
    ptls_t *tls;

    // TODO: maybe better in ConnectionState?
    EncryptionKey egressKey;
    EncryptionKey ingressKey;

    ChunkQueue cryptoQueues[4];

    /**
     * Enqueues data in the corresponding stream queue and triggers packet sending.
     * Called upon a send command from app.
     *
     * @param streamId Id of the stream.
     * @param data Data to enqueue.
     */
    void newStreamData(uint64_t streamId, Ptr<const Chunk> data);

    void newCryptoData(EncryptionLevel epoch, Ptr<const Chunk> data);

    void processReceivedData(uint64_t streamId, uint64_t offset, Ptr<const Chunk> data);
    void accountReceivedPacket(uint64_t packetNumber, bool ackEliciting, PacketNumberSpace space, bool isIBitSet);
    void onMaxDataFrameReceived(uint64_t maxData);
    void onMaxStreamDataFrameReceived(uint64_t streamId, uint64_t maxStreamData);
    void onStreamDataBlockedFrameReceived(uint64_t streamId, uint64_t streamDataLimit);
    void onDataBlockedFrameReceived(uint64_t dataLimit);
    void onMaxDataFrameLost();

    /**
     * Creates a timer with the given type and name.
     *
     * @param kind Kind of the timer message.
     * @param name Name of the timer message.
     * @return Pointer to the created Timer object.
     */
    Timer *createTimer(TimerType kind, std::string name);

    Timer *createTimer(cMessage *msg);

    /**
     * Creates a probe packet (aka tail loss probe) by using
     * (1) new data,
     * (2) retransmit sent but outstanding data, or
     * (3) a ping frame.
     * After that, it sends the probe packet.
     * This method is used by ReliabilityManager when its lossDetectionTimer fires.
     *
     * @param ptoCount The number of subsequent probe timeouts.
     */
    void sendProbePacket(uint ptoCount);

    void sendClientInitialPacket(uint32_t token = 0);
    void sendServerInitialPacket();
    void sendHandshakePacket(bool includeTransportParamters);
    void sendDataToApp(uint64_t streamId, B expectedDataSize);
    void handleAckFrame(const Ptr<const AckFrameHeader>& frameHeader, PacketNumberSpace pnSpace);
    void processIcmpPtb(Packet *droppedPkt, int ptbMtu);
    void reportPtb(uint64_t droppedPacketNumber, int ptbMtu);
    void setHandshakeConfirmed(bool value);
    bool isHandshakeConfirmed();
    void addDstConnectionId(uint64_t id, uint8_t length);
    void clearDstConnectionIds();
    void sendAck(PacketNumberSpace pnSpace);
    void established();
    void sendHandshakeDone();
    void enqueueZeroRttTokenFrame();
    void buildClientTokenAndSendToApp(uint32_t token);
    void close(bool sendAck, bool appInitiated);
    void sendConnectionClose(bool sendAck, bool appInitiated, int errorCode);
    bool belongsPacketTo(Packet *pkt, uint64_t dstConnectionId);
    uint32_t processClientTokenExtractToken(const char *clientToken);
    void addConnectionForInitialConnectionId(uint64_t initialConnectionId);
    void removeConnectionForInitialConnectionId();
    void initializeRemoteTransportParameters(uint64_t maxData, uint64_t maxStreamData);
    void initializeRemoteTransportParameters(Ptr<const TransportParametersExtension> transportParametersExt);


    ReliabilityManager *getReliabilityManager() {
        return this->reliabilityManager;
    }
    ReceivedPacketsAccountant *getReceivedPacketsAccountant(PacketNumberSpace space) {
        return receivedPacketsAccountants[space];
    }

    TransportParameters *getLocalTransportParameters() {
        return localTransportParameters;
    }

    TransportParameters *getRemoteTransportParameters() {
        return remoteTransportParameters;
    }

    ConnectionFlowController *getConnectionFlowController() {
        return connectionFlowController;
    }

    ConnectionFlowControlResponder *getConnectionFlowControlResponder() {
        return connectionFlowControlResponder;
    }

    std::vector<QuicFrame *> *getControlQueue() {
        return &controlQueue;
    }

    Quic *getModule() {
        return quicSimpleMod;
    }

    uint64_t getMaxStreamDataFrameThreshold(){
        return maxStreamDataFrameThreshold;
    }

    bool getRoundConsumedDataValue(){
        return roundConsumedDataValue;
    }

    Path *getPath() {
        return path;
    }

    void onDplpmtudLeftBase() {
        dplpmutdInIntialBase = false;
    }

    std::vector<ConnectionId*> getSrcConnectionIds(){
        return srcConnectionIds;
    }

    std::vector<ConnectionId*> getDstConnectionIds() {
        return dstConnectionIds;
    }

    AppSocket *getAppSocket() {
        return appSocket;
    }

    void setAppSocket(AppSocket *appSocket) {
        this->appSocket = appSocket;
    }

    UdpSocket *getUdpSocket() {
        return udpSocket;
    }


  private:
    Quic *quicSimpleMod;
    AppSocket *appSocket;
    UdpSocket *udpSocket;

    std::vector<ConnectionId*> srcConnectionIds;
    std::vector<ConnectionId*> dstConnectionIds;
    ConnectionState *connectionState;

    std::vector<QuicFrame *> controlQueue;
    PacketBuilder *packetBuilder;
    IScheduler *scheduler;
    std::map<uint64_t, Stream *> streamMap;
    TransportParameters *localTransportParameters = nullptr;
    TransportParameters *remoteTransportParameters = nullptr;
    ConnectionFlowController *connectionFlowController = nullptr;
    ConnectionFlowControlResponder *connectionFlowControlResponder = nullptr;

    ReceivedPacketsAccountant *receivedPacketsAccountants[3];
    ReliabilityManager *reliabilityManager;

    ICongestionController *congestionController;

    bool closed = false;

    bool acceptDataFromApp;
    int sendQueueLimit;
    double sendQueueLowWaterRatio;
    double maxDataFrameThreshold;
    double maxStreamDataFrameThreshold;
    bool roundConsumedDataValue;
    bool sendMaxDataFramesImmediately;
    int useCwndForParallelProbes;
    bool sendDataDuringInitalDplpmtudBase;
    //bool reduceTlpSizeOnlyIfPmtuInvalidPossible;

    bool dplpmutdInIntialBase;

    bool handshakeConfirmed = false;

    Statistics *stats;

    Path *path;


    int lastMaxQuicPacketSize;
    simsignal_t usedMaxQuicPacketSizeStat;
    simsignal_t packetNumberReceivedStat;
    simsignal_t packetNumberSentStat;
    Timer *closeTimer = nullptr;

    bool initialConnectionIdSet = false;
    uint64_t initialConnectionId = 0;

    bool remoteTransportParametersInitialized = false;

    void sendPacket(QuicPacket *packet, PacketNumberSpace pnSpace, bool track = true);
    Stream *findOrCreateStream(uint64_t streamid);
    uint64_t getStreamsSendQueueLength();
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_CONNECTION_H_ */

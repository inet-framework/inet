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

#ifndef INET_APPLICATIONS_QUIC_CONNECTION_H_
#define INET_APPLICATIONS_QUIC_CONNECTION_H_

#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "Quic.h"
#include "UdpSocket.h"
#include "AppSocket.h"
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
    DPLPMTUD_RAISE_TIMER
    //QUIC_T_TIMEOUT_IDLE,
    //QUIC_T_TIMEOUT_CLOSING,
    //QUIC_T_SEND
};

class Connection
{
  public:
    Connection(Quic *quicSimpleMod, UdpSocket *udpSocket, AppSocket *appSocket, L3Address remoteAddr, int remotePort);
    virtual ~Connection();
    virtual void processAppCommand(cMessage *msg);
    virtual void processPackets(Packet *pkt);
    virtual void processTimeout(cMessage *msg);
    void sendPackets();
    void newStreamData(uint64_t streamId, Ptr<const Chunk> data);
    void processReceivedData(uint64_t streamId, uint64_t offset, Ptr<const Chunk> data);
    void accountReceivedPacket(uint64_t packetNumber, bool ackEliciting, PacketNumberSpace space, bool isIBitSet);
    void onMaxDataFrameReceived(uint64_t maxData);
    void onMaxStreamDataFrameReceived(uint64_t streamId, uint64_t maxStreamData);
    void onStreamDataBlockedFrameReceived(uint64_t streamId, uint64_t streamDataLimit);
    void onDataBlockedFrameReceived(uint64_t dataLimit);
    void onMaxDataFrameLost();
    Timer *createTimer(TimerType kind, std::string name);
    Timer *createTimer(cMessage *msg);
    void sendProbePacket(uint ptoCount);
    void sendClientInitialPacket();
    void sendServerInitialPacket();
    void sendHandshakePacket(bool includeTransportParamters);
    void sendDataToApp(uint64_t streamId, B expectedDataSize);
    void handleAckFrame(const Ptr<const AckFrameHeader>& frameHeader, PacketNumberSpace pnSpace);
    void processIcmpPtb(Packet *droppedPkt, int ptbMtu);
    void reportPtb(int droppedPacketNumber, int ptbMtu);
    void setHandshakeConfirmed(bool value);
    bool isHandshakeConfirmed();
    void addDstConnectionId(uint64_t id, uint8_t length);
    void sendAck(PacketNumberSpace pnSpace);
    void established();
    void sendHandshakeDone();
    void close(bool sendAck, bool appInitiated);
    void sendConnectionClose(bool sendAck, bool appInitiated, int errorCode);

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

    bool handshakeConfirmed;

    Statistics *stats;

    Path *path;

    int lastMaxQuicPacketSize;
    simsignal_t usedMaxQuicPacketSizeStat;
    simsignal_t packetNumberReceivedStat;
    simsignal_t packetNumberSentStat;


    void sendPacket(QuicPacket *packet, PacketNumberSpace pnSpace, bool track = true);
    Stream *findOrCreateStream(uint64_t streamid);
    uint64_t getStreamsSendQueueLength();
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_CONNECTION_H_ */

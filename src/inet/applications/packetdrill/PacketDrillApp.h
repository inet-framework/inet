//
// Copyright (C) 2014 Irene Ruengeler
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_PACKETDRILLAPP_H_
#define __INET_PACKETDRILLAPP_H_

#include "inet/common/INETDefs.h"

#include "inet/applications/base/ApplicationBase.h"
#include "inet/applications/packetdrill/PacketDrill.h"
#include "inet/applications/packetdrill/PacketDrillUtils.h"
#include "inet/common/socket/SocketMap.h"
#include "inet/linklayer/tun/TunSocket.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/transportlayer/contract/sctp/SctpSocket.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/transportlayer/tcp/TcpConnection.h"
#include "inet/transportlayer/tcp_common/TcpHeader_m.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"

namespace inet {

class PacketDrill;
class PacketDrillScript;

/**
 * Implements the packetdrill application simple module. See the NED file for more info.
 */
class INET_API PacketDrillApp : public ApplicationBase,
        public UdpSocket::ICallback, public TcpSocket::ICallback,
        public SctpSocket::ICallback, TunSocket::ICallback
{
  public:
    PacketDrillApp();
    virtual ~PacketDrillApp();

    int getLocalPort() const { return localPort; }
    int getRemotePort() const { return remotePort; }
    uint32 getIdInbound() const { return idInbound; }
    uint32 getIdOutbound() const { return idOutbound; }
    uint32 getPeerTS() { return peerTS; }
    void increaseIdInbound() { idInbound++; }
    void increaseIdOutbound() { idOutbound++; }
    const L3Address getLocalAddress() { return localAddress; }
    const L3Address getRemoteAddress() { return remoteAddress; }
    uint32 getPeerVTag() { return peerVTag; }
    uint32 getLocalVTag() { return localVTag; }
    uint32 getPeerCumTsn() { return peerCumTsn; }
    uint32 getInitPeerTsn() { return initPeerTsn; }
    simtime_t getPeerHeartbeatTime() { return peerHeartbeatTime; }
    void setSeqNumMap(uint32 ownNum, uint32 liveNum) { seqNumMap[ownNum] = liveNum; }
    uint32 getSeqNumMap(uint32 ownNum) { return seqNumMap[ownNum]; }
    bool findSeqNumMap(uint32 num);
    CrcMode getCrcMode() { return crcMode; }

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    void initialize(int stage) override;
    void handleMessageWhenUp(cMessage *msg) override;
    void finish() override;

    void handleTimer(cMessage *msg);

  private:
    PacketDrillScript *script = nullptr;
    PacketDrillConfig *config = nullptr;
    L3Address localAddress;
    L3Address remoteAddress;
    int localPort = 0;
    int remotePort = 0;
    int protocol = 0;
    int tcpConnId = -1;
    int sctpAssocId = -1;
    int tunSocketId = -1;
    int tunInterfaceId = -1;
    UdpSocket udpSocket;
    TcpSocket tcpSocket;
    SctpSocket sctpSocket;
    TunSocket tunSocket;
    SocketMap socketMap;    // holds TCP connections with peers
    PacketDrill *pd = nullptr;
    bool msgArrived = false;
    bool recvFromSet = false;
    bool listenSet = false;
    bool acceptSet = false;
    bool establishedPending = false;
    bool abortSent = false;
    bool socketOptionsArrived_ = false;
    cPacketQueue *receivedPackets = nullptr;
    cPacketQueue *outboundPackets = nullptr;
    simtime_t simStartTime;
    simtime_t simRelTime;
    uint32 expectedMessageSize = 0;
    uint32 relSequenceOut = 0;
    uint32 peerTS = 0;
    uint16 peerWindow = 0;
    uint16 peerInStreams = 0;
    uint16 peerOutStreams = 0;
    sctp::SctpCookie *peerCookie = nullptr;
    uint16 peerCookieLength = 0;
    uint32 initPeerTsn = 0;
    uint32 initLocalTsn = 0;
    uint32 localDiffTsn = 0;
    uint32 peerCumTsn = 0;
    uint32 localCumTsn = 0;
    uint32 eventCounter = 0;
    uint32 numEvents = 0;
    uint32 idInbound = 0;
    uint32 idOutbound = 0;
    uint32 localVTag = 0;
    uint32 peerVTag = 0;
    std::map<uint32, uint32> seqNumMap;
    simtime_t peerHeartbeatTime;
    cMessage *eventTimer = nullptr;
    CrcMode crcMode = CRC_MODE_UNDEFINED;
    int bytesSent = 0;

  private:
    void scheduleEvent();

    void runEvent(PacketDrillEvent *event);
    void runSystemCallEvent(PacketDrillEvent *event, struct syscall_spec *syscall);
    void closeAllSockets();

    int syscallSocket(struct syscall_spec *syscall, cQueue *args, char **error);

    int syscallBind(struct syscall_spec *syscall, cQueue *args, char **error);

    int syscallListen(struct syscall_spec *syscall, cQueue *args, char **error);

    int syscallConnect(struct syscall_spec *syscall, cQueue *args, char **error);

    int syscallWrite(struct syscall_spec *syscall, cQueue *args, char **error);

    int syscallAccept(struct syscall_spec *syscall, cQueue *args, char **error);

    int syscallSetsockopt(struct syscall_spec *syscall, cQueue *args, char **error);

    int syscallGetsockopt(struct syscall_spec *syscall, cQueue *args, char **error);

    int syscallSendTo(struct syscall_spec *syscall, cQueue *args, char **error);

    int syscallRead(PacketDrillEvent *event, struct syscall_spec *syscall, cQueue *args, char **error);

    int syscallRecvFrom(PacketDrillEvent *event, struct syscall_spec *syscall, cQueue *args, char **error);

    int syscallClose(struct syscall_spec *syscall, cQueue *args, char **error);

    int syscallShutdown(struct syscall_spec *syscall, cQueue *args, char **error);

    int syscallSctpSendmsg(struct syscall_spec *syscall, cQueue *args, char **error);

    int syscallSctpSend(struct syscall_spec *syscall, cQueue *args, char **error);

    bool compareDatagram(Packet *storedDatagram, Packet *liveDatagram);

    bool compareUdpHeader(const Ptr<const UdpHeader>& storedUdp, const Ptr<const UdpHeader>& liveUdp);

    bool compareTcpHeader(const Ptr<const tcp::TcpHeader>& storedTcp, const Ptr<const tcp::TcpHeader>& liveTcp);

    bool compareSctpPacket(const Ptr<const sctp::SctpHeader>& storedSctp, const Ptr<const sctp::SctpHeader>& liveSctp);

    bool compareInitPacket(const sctp::SctpInitChunk* storedInitChunk, const sctp::SctpInitChunk* liveInitChunk);

    bool compareDataPacket(const sctp::SctpDataChunk* storedDataChunk, const sctp::SctpDataChunk* liveDataChunk);

    bool compareSackPacket(const sctp::SctpSackChunk* storedSackChunk, const sctp::SctpSackChunk* liveSackChunk);

    bool compareInitAckPacket(const sctp::SctpInitAckChunk* storedInitAckChunk, const sctp::SctpInitAckChunk* liveInitAckChunk);

    bool compareReconfigPacket(const sctp::SctpStreamResetChunk* storedReconfigChunk, const sctp::SctpStreamResetChunk* liveReconfigChunk);

    int verifyTime(enum eventTime_t timeType,
            simtime_t script_usecs, simtime_t script_usecs_end,
            simtime_t offset, simtime_t liveTime, const char *description);

    void adjustTimes(PacketDrillEvent *event);

    /** @name OperationalBase lifecycle methods */
    //@{
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;
    //@}

    /** @name TcpSocket::ICallback methods */
    //@{
    virtual void socketDataArrived(TcpSocket *socket, Packet *msg, bool urgent) override;
    virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) override;
    virtual void socketEstablished(TcpSocket *socket) override;
    virtual void socketPeerClosed(TcpSocket *socket) override;
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override;
    virtual void socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) override { }
    virtual void socketDeleted(TcpSocket *socket) override {}   //TODO
    //@}

    /** @name UdpSocket::ICallback methods */
    //@{
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;
    //@}

    /** @name SctpSocket::ICallback methods */
    //@{
    virtual void socketDataArrived(SctpSocket *socket, Packet *packet, bool urgent) override;
    virtual void socketDataNotificationArrived(SctpSocket *socket, Message *msg) override;
    virtual void socketAvailable(SctpSocket *socket, Indication *indication) override;
    virtual void socketEstablished(SctpSocket *socket, unsigned long int buffer) override;
    virtual void socketOptionsArrived(SctpSocket *socket, Indication *indication) override;
    virtual void socketPeerClosed(SctpSocket *socket) override;
    virtual void socketClosed(SctpSocket *socket) override;
    virtual void socketFailure(SctpSocket *socket, int code) override;
    virtual void socketStatusArrived(SctpSocket *socket, SctpStatusReq *status) override;
    virtual void socketDeleted(SctpSocket *socket) override;
    virtual void sendRequestArrived(SctpSocket *socket) override;
    virtual void msgAbandonedArrived(SctpSocket *socket) override;
    virtual void shutdownReceivedArrived(SctpSocket *socket) override;
    virtual void sendqueueFullArrived(SctpSocket *socket) override;
    virtual void sendqueueAbatedArrived(SctpSocket *socket, unsigned long int buffer) override;
    virtual void addressAddedArrived(SctpSocket *socket, L3Address localAddr, L3Address remoteAddr) override;
    //@}

    /** @name TunSocket::ICallback methods */
    //@{
    virtual void socketDataArrived(TunSocket *socket, Packet *packet) override;
    virtual void socketClosed(TunSocket *socket) override;
    //@}
};

} // namespace inet

#endif


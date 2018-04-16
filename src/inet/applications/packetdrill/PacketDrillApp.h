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
#include "inet/applications/tcpapp/TcpSessionApp.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/linklayer/tun/TunSocket.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"
#include "inet/transportlayer/contract/sctp/SctpSocket.h"
#include "inet/transportlayer/tcp/TcpConnection.h"
#include "inet/transportlayer/tcp_common/TcpHeader_m.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"
#include "inet/applications/packetdrill/PacketDrill.h"
#include "inet/applications/packetdrill/PacketDrillUtils.h"

namespace inet {

class PacketDrill;
class PacketDrillScript;

/**
 * Implements the packetdrill application simple module. See the NED file for more info.
 */
class INET_API PacketDrillApp : public TcpSessionApp, public ILifecycle
{
    public:
        PacketDrillApp();
        virtual ~PacketDrillApp();

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override
    { Enter_Method_Silent(); throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName()); return true; }

    const int getLocalPort() { return localPort;};
    const int getRemotePort() { return remotePort;};
    const uint32 getIdInbound() { return idInbound;};
    const uint32 getIdOutbound() { return idOutbound;};
    uint32 getPeerTS() { return peerTS; };
    void increaseIdInbound() { idInbound++;};
    void increaseIdOutbound() { idOutbound++;};
    const L3Address getLocalAddress() { return localAddress; };
    const L3Address getRemoteAddress() { return remoteAddress; };
    uint32 getPeerVTag() { return peerVTag; };
    uint32 getLocalVTag() { return localVTag; };
    uint32 getPeerCumTsn() { return peerCumTsn; };
    uint32 getInitPeerTsn() { return initPeerTsn; };
    simtime_t getPeerHeartbeatTime() { return peerHeartbeatTime; };
    void setSeqNumMap(uint32 ownNum, uint32 liveNum) { seqNumMap[ownNum] = liveNum; };
    uint32 getSeqNumMap(uint32 ownNum) { return seqNumMap[ownNum]; };
    bool findSeqNumMap(uint32 num);
    CrcMode getCrcMode() { return crcMode; };

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        void initialize(int stage) override;
        void handleMessage(cMessage *msg) override;
        void finish() override;

        void handleTimer(cMessage *msg) override;

    private:
        PacketDrillScript *script;
        PacketDrillConfig *config;
        L3Address localAddress;
        L3Address remoteAddress;
        int localPort;
        int remotePort;
        int protocol;
        int tcpConnId;
        int sctpAssocId;
        int tunSocketId;
        int udpSocketId;
        int tunInterfaceId;
        UdpSocket udpSocket;
        TcpSocket tcpSocket;
        SctpSocket sctpSocket;
        TunSocket tunSocket;
        PacketDrill *pd;
        bool msgArrived;
        bool recvFromSet;
        bool listenSet;
        bool acceptSet;
        bool establishedPending;
        bool abortSent;
        bool socketOptionsArrived;
        cPacketQueue *receivedPackets;
        cPacketQueue *outboundPackets;
        simtime_t simStartTime;
        simtime_t simRelTime;
        uint32 expectedMessageSize;
        uint32 relSequenceIn;
        uint32 relSequenceOut;
        uint32 peerTS;
        uint16 peerWindow;
        uint16 peerInStreams;
        uint16 peerOutStreams;
        sctp::SctpCookie *peerCookie;
        uint16 peerCookieLength;
        uint32 initPeerTsn;
        uint32 initLocalTsn;
        uint32 localDiffTsn;
        uint32 peerCumTsn;
        uint32 localCumTsn;
        uint32 eventCounter;
        uint32 numEvents;
        uint32 idInbound;
        uint32 idOutbound;
        uint32 localVTag;
        uint32 peerVTag;
        std::map<uint32, uint32> seqNumMap;
        simtime_t peerHeartbeatTime;
        cMessage *eventTimer;
        CrcMode crcMode = static_cast<CrcMode>(-1);


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
};

} // namespace inet

#endif



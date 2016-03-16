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
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"
#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "inet/transportlayer/tcp_common/TCPSegment_m.h"
#include "inet/transportlayer/udp/UDPPacket_m.h"
#include "inet/transportlayer/tcp/TCPConnection.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#include "inet/applications/tcpapp/TCPSessionApp.h"
#include "PacketDrill.h"
#include "PacketDrillUtils.h"

class PacketDrill;
class PacketDrillScript;

namespace inet {

using namespace tcp;
/**
 * Implements the packetdrill application simple module. See the NED file for more info.
 */
class INET_API PacketDrillApp : public TCPSessionApp, public ILifecycle
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

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        void initialize(int stage) override;
        void handleMessage(cMessage *msg) override;
        void finish() override;

        void handleTimer(cMessage *msg) override;

    private:
        const char *scriptFile;
        PacketDrillScript *script;
        PacketDrillConfig *config;
        L3Address localAddress;
        L3Address remoteAddress;
        int localPort;
        int remotePort;
        int protocol;
        int tcpConnId;
        UDPSocket udpSocket;
        TCPSocket tcpSocket;
        PacketDrill *pd;
        bool msgArrived;
        bool recvFromSet;
        bool listenSet;
        bool acceptSet;
        bool establishedPending;
        cPacketQueue *receivedPackets;
        cPacketQueue *outboundPackets;
        simtime_t simStartTime;
        simtime_t simRelTime;
        uint32 expectedMessageSize;
        uint32 relSequenceIn;
        uint32 relSequenceOut;
        uint32 peerTS;
        uint16 peerWindow;
        uint32 eventCounter;
        uint32 numEvents;
        uint32 idInbound;
        uint32 idOutbound;
        cMessage *eventTimer;


        void scheduleEvent();

        void runEvent(PacketDrillEvent *event);
        void runSystemCallEvent(PacketDrillEvent *event, struct syscall_spec *syscall);

        int syscallSocket(struct syscall_spec *syscall, cQueue *args, char **error);

        int syscallBind(struct syscall_spec *syscall, cQueue *args, char **error);

        int syscallListen(struct syscall_spec *syscall, cQueue *args, char **error);

        int syscallConnect(struct syscall_spec *syscall, cQueue *args, char **error);

        int syscallWrite(struct syscall_spec *syscall, cQueue *args, char **error);

        int syscallAccept(struct syscall_spec *syscall, cQueue *args, char **error);

        int syscallSendTo(struct syscall_spec *syscall, cQueue *args, char **error);

        int syscallRead(PacketDrillEvent *event, struct syscall_spec *syscall, cQueue *args, char **error);

        int syscallRecvFrom(PacketDrillEvent *event, struct syscall_spec *syscall, cQueue *args, char **error);

        int syscallClose(struct syscall_spec *syscall, cQueue *args, char **error);

        bool compareDatagram(IPv4Datagram *storedDatagram, IPv4Datagram *liveDatagram);

        bool compareUdpPacket(UDPPacket *storedUdp, UDPPacket *liveUdp);

        bool compareTcpPacket(TCPSegment *storedTcp, TCPSegment *liveTcp);

        int verifyTime(enum eventTime_t timeType,
            simtime_t script_usecs, simtime_t script_usecs_end,
            simtime_t offset, simtime_t liveTime, const char *description);

        void adjustTimes(PacketDrillEvent *event);
};

} // namespace inet

#endif



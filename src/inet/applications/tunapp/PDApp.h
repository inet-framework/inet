//
// Copyright (C) 2014 Irene Ruengeler
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __PDAPP_H_
#define __PDAPP_H_

//#include <platdep/timeutil.h>
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
#include "inet/applications/tunapp/PacketDrill.h"
#include "inet/applications/tunapp/PDUtils.h"

class PacketDrill;
class PDScript;

namespace inet {

/**
 * Implements the packetdrill application simple module. See the NED file for more info.
 */
class INET_API PDApp : public TCPSessionApp, public ILifecycle
{
    public:
        virtual ~PDApp();

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override
    { Enter_Method_Silent(); throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName()); return true; }

    int getLocalPort() { return localPort;};
    int getRemotePort() { return remotePort;};
    uint32 getIdInbound() { return idInbound;};
    uint32 getIdOutbound() { return idOutbound;};
    void increaseIdInbound() { idInbound++;};
    void increaseIdOutbound() { idOutbound++;};
    L3Address getLocalAddress() { return localAddress; };
    L3Address getRemoteAddress() { return remoteAddress; };

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        void initialize(int stage) override;
        void handleMessage(cMessage *msg) override;
        void finish() override;

        void handleTimer(cMessage *msg);

    private:
        const char *scriptFile;
        PDScript *script;
        PDConfig *config;
        L3Address localAddress;
        L3Address remoteAddress;
        int localPort;
        int remotePort;
        int protocol;
        UDPSocket udpSocket;
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
        uint32 eventCounter;
        uint32 numEvents;
        uint32 idInbound;
        uint32 idOutbound;
        cMessage *eventTimer;


        void scheduleEvent();

        void runEvent(PDEvent *event);
        void runSystemCallEvent(PDEvent *event, struct syscall_spec *syscall);

        int syscallSocket(struct syscall_spec *syscall, cQueue *args, char **error);

        int syscallBind(struct syscall_spec *syscall, cQueue *args, char **error);

        int syscallListen(struct syscall_spec *syscall, cQueue *args, char **error);

        int syscallConnect(struct syscall_spec *syscall, cQueue *args, char **error);

        int syscallAccept(struct syscall_spec *syscall, cQueue *args, char **error);

        int syscallSendTo(struct syscall_spec *syscall, cQueue *args, char **error);

        int syscallRecvFrom(PDEvent *event, struct syscall_spec *syscall, cQueue *args, char **error);

        int syscallClose(struct syscall_spec *syscall, cQueue *args, char **error);

        bool compareDatagram(IPv4Datagram *storedDatagram, IPv4Datagram *liveDatagram);

        bool compareUdpPacket(UDPPacket *storedUdp, UDPPacket *liveUdp);

        int verifyTime(enum eventTime_t timeType,
            simtime_t script_usecs, simtime_t script_usecs_end,
            simtime_t offset, simtime_t liveTime, const char *description);

        void adjustTimes(PDEvent *event);
};

} // namespace inet

#endif



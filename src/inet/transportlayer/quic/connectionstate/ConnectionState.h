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

    virtual ConnectionState *processPacket(Packet *msg);
    virtual ConnectionState *processInitialPacket(const Ptr<const InitialPacketHeader>& packetHeader, Packet *pkt);
    virtual ConnectionState *processHandshakePacket(const Ptr<const HandshakePacketHeader>& packetHeader, Packet *pkt);
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
    virtual void processConnectionCloseFrame();

    virtual ConnectionState *processIcmpPtb(Packet *droppedPacket, int ptbMtu);
    virtual ConnectionState *processIcmpPtb(uint32_t droppedPacketNumber, int ptbMtu);

    virtual ConnectionState *processTimeout(cMessage *msg);
    virtual ConnectionState *processLossDetectionTimeout(cMessage *msg);
    virtual ConnectionState *processAckDelayTimeout(cMessage *msg);
    virtual ConnectionState *processDplpmtudRaiseTimeout(cMessage *msg);

protected:
    Connection *context;
    std::string name;
    bool ackElicitingPacket;
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_CONNECTIONSTATE_CONNECTIONSTATE_H_ */

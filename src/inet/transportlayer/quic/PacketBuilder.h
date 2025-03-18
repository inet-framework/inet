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

namespace inet {
namespace quic {

class ReceivedPacketsAccountant;
class IScheduler;

class PacketBuilder {
public:
    PacketBuilder(std::vector<QuicFrame*> *controlQueue, IScheduler *scheduler, ReceivedPacketsAccountant *receivedPacketsAccountant[]);
    virtual ~PacketBuilder();

    void readParameters(cModule *module);
    QuicPacket *buildPacket(int maxPacketSize, int safePacketSize);
    QuicPacket *buildAckOnlyPacket(int maxPacketSize, PacketNumberSpace pnSpace);
    QuicPacket *buildAckElicitingPacket(int maxPacketSize);
    QuicPacket *buildAckElicitingPacket(std::vector<QuicPacket*> *sentPackets, int maxPacketSize, bool skipPacketNumber=false);
    QuicPacket *buildPingPacket();
    QuicPacket *buildDplpmtudProbePacket(int packetSize, Dplpmtud *dplpmtud);
    QuicPacket *buildClientInitialPacket(int maxPacketSize);
    QuicPacket *buildServerInitialPacket(int maxPacketSize);
    QuicPacket *buildHandshakePacket(int maxPacketSize);
    void addHandshakeDone();
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

    QuicPacket *createPacket(PacketNumberSpace pnSpace, bool skipPacketNumber);
    Ptr<InitialPacketHeader> createInitialHeader();
    Ptr<HandshakePacketHeader> createHandshakeHeader();
    QuicPacket *createOneRttPacket(bool skipPacketNumber=false);
    Ptr<ShortPacketHeader> createOneRttHeader();
    QuicPacket *addFramesFromControlQueue(QuicPacket *packet, int maxPacketSize);
    QuicPacket *addFrameToPacket(QuicPacket *packet, QuicFrame *frame, bool skipPacketNumber=false);
    size_t getPacketSize(QuicPacket *packet);
    QuicFrame *createPingFrame();
    QuicFrame *createPaddingFrame(int length = 1);
    QuicFrame *createCryptoFrame();
    void fillLongHeader(Ptr<LongPacketHeader> packetHeader);
    QuicFrame *createHandshakeDoneFrame();
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_PACKETBUILDER_H_ */

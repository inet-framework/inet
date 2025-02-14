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

namespace inet {
namespace quic {

class ReceivedPacketsAccountant;
class IScheduler;

class PacketBuilder {
public:
    PacketBuilder(std::vector<QuicFrame*> *controlQueue, IScheduler *scheduler, ReceivedPacketsAccountant *receivedPacketsAccountant);
    virtual ~PacketBuilder();

    void readParameters(cModule *module);
    QuicPacket *buildPacket(int maxPacketSize, int safePacketSize);
    QuicPacket *buildAckOnlyPacket(int maxPacketSize);
    QuicPacket *buildAckElicitingPacket(int maxPacketSize);
    QuicPacket *buildAckElicitingPacket(std::vector<QuicPacket*> *sentPackets, int maxPacketSize, bool skipPacketNumber=false);
    QuicPacket *buildPingPacket();
    QuicPacket *buildDplpmtudProbePacket(int packetSize, Dplpmtud *dplpmtud);
    void setConnectionId(uint64_t connectionId) {
        this->connectionId = connectionId;
    }

private:
    std::vector<QuicFrame*> *controlQueue;
    IScheduler *scheduler;
    ReceivedPacketsAccountant *receivedPacketsAccountant;

    uint64_t shortPacketNumber;
    uint64_t connectionId;
    bool bundleAckForNonAckElicitingPackets;
    bool skipPacketNumberForDplpmtudProbePackets;

    QuicPacket *createPacket(bool skipPacketNumber=false);
    Ptr<ShortPacketHeader> createHeader();
    QuicPacket *addFramesFromControlQueue(QuicPacket *packet, int maxPacketSize);
    QuicPacket *addFrameToPacket(QuicPacket *packet, QuicFrame *frame, bool skipPacketNumber=false);
    size_t getPacketSize(QuicPacket *packet);
    QuicFrame *createPingFrame();
    QuicFrame *createPaddingFrame(int length = 1);
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_PACKETBUILDER_H_ */

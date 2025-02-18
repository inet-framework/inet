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

#ifndef INET_APPLICATIONS_QUIC_QUICPACKET_H_
#define INET_APPLICATIONS_QUIC_QUICPACKET_H_

#include "PacketHeader_m.h"
#include "QuicFrame.h"
#include "inet/common/packet/Packet.h"

namespace inet {
namespace quic {

enum PacketNumberSpace {
    Initial,
    Handshake,
    ApplicationData
};

class QuicPacket {
public:
    QuicPacket(std::string name);
    virtual ~QuicPacket();

    uint64_t getPacketNumber();
    bool isCryptoPacket();

    void setHeader(Ptr<PacketHeader> header);
    void addFrame(QuicFrame *frame);
    Packet *createOmnetPacket();
    virtual void onPacketLost();
    virtual void onPacketAcked();

    virtual void setIBit(bool iBit);
    virtual bool isDplpmtudProbePacket();

    virtual bool containsFrame(QuicFrame *otherFrame);
    virtual int getMemorySize();

    bool countsAsInFlight() {
        return countsInFlight;
    }
    omnetpp::simtime_t getTimeSent() {
        return timeSent;
    }
    void setTimeSent(omnetpp::simtime_t time) {
        timeSent = time;
    }
    bool isAckEliciting() {
        return ackEliciting;
    }
    size_t getSize() {
        return size;
    }
    size_t getDataSize() {
        return dataSize;
    }
    std::vector<QuicFrame*> *getFrames() {
        return &frames;
    }
    std::string getName() {
        return name;
    }
    Ptr<PacketHeader> getHeader() {
        return header;
    }

private:
    bool ackEliciting;
    bool countsInFlight;
    omnetpp::simtime_t timeSent;
    Ptr<PacketHeader> header;
    std::vector<QuicFrame*> frames;
    size_t size;
    size_t dataSize;
    std::string name;

};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_QUICPACKET_H_ */

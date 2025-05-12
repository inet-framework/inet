//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
    Packet *createOmnetPacket(const char *secret);
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
    bool ackEliciting = false;
    bool countsInFlight = false;
    omnetpp::simtime_t timeSent;
    Ptr<PacketHeader> header;
    std::vector<QuicFrame*> frames;
    size_t size = 0;
    size_t dataSize = 0;
    std::string name;

};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_QUICPACKET_H_ */

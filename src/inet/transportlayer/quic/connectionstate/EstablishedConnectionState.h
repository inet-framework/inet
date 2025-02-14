/*
 * EstablishedConnectionState.h
 *
 *  Created on: 7 Feb 2025
 *      Author: msvoelker
 */

#ifndef INET_TRANSPORTLAYER_QUIC_CONNECTIONSTATE_ESTABLISHEDCONNECTIONSTATE_H_
#define INET_TRANSPORTLAYER_QUIC_CONNECTIONSTATE_ESTABLISHEDCONNECTIONSTATE_H_

#include "ConnectionState.h"

namespace inet {
namespace quic {

class EstablishedConnectionState: public ConnectionState {
public:
    EstablishedConnectionState(Connection *context) : ConnectionState(context) {
        name = "Established";
    }

    virtual ConnectionState *processSendAppCommand(cMessage *msg);
    virtual ConnectionState *processOneRttPacket(const Ptr<const OneRttPacketHeader>& packetHeader, Packet *pkt);
    virtual void processStreamFrame(const Ptr<const StreamFrameHeader>& frameHeader, Packet *pkt);
    virtual void processAckFrame(const Ptr<const AckFrameHeader>& frameHeader);
    virtual void processMaxDataFrame(const Ptr<const MaxDataFrameHeader>& frameHeader);
    virtual void processMaxStreamDataFrame(const Ptr<const MaxStreamDataFrameHeader>& frameHeader);
    virtual void processStreamDataBlockedFrame(const Ptr<const StreamDataBlockedFrameHeader>& frameHeader);
    virtual void processDataBlockedFrame(const Ptr<const DataBlockedFrameHeader>& frameHeader);
    virtual ConnectionState *processLossDetectionTimeout(cMessage *msg);
    virtual ConnectionState *processAckDelayTimeout(cMessage *msg);
    virtual ConnectionState *processRecvAppCommand(cMessage *msg);
    virtual ConnectionState *processIcmpPtb(uint32_t droppedPacketNumber, int ptbMtu);
    virtual ConnectionState *processDplpmtudRaiseTimeout(cMessage *msg);
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_CONNECTIONSTATE_ESTABLISHEDCONNECTIONSTATE_H_ */

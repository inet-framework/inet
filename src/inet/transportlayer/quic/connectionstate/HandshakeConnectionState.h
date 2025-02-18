/*
 * HandshakeConnectionState.h
 *
 *  Created on: 18 Feb 2025
 *      Author: msvoelker
 */

#ifndef INET_TRANSPORTLAYER_QUIC_CONNECTIONSTATE_HANDSHAKECONNECTIONSTATE_H_
#define INET_TRANSPORTLAYER_QUIC_CONNECTIONSTATE_HANDSHAKECONNECTIONSTATE_H_

#include "ConnectionState.h"

namespace inet {
namespace quic {

class HandshakeConnectionState: public ConnectionState {
public:
    HandshakeConnectionState(Connection *context) : ConnectionState(context) {
        name = "Handshake";
    }

    virtual ConnectionState *processHandshakePacket(const Ptr<const HandshakePacketHeader>& packetHeader, Packet *pkt) override;
    //virtual void processAckFrame(const Ptr<const AckFrameHeader>& frameHeader);
    //virtual ConnectionState *processLossDetectionTimeout(cMessage *msg);
    //virtual ConnectionState *processAckDelayTimeout(cMessage *msg);
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_CONNECTIONSTATE_HANDSHAKECONNECTIONSTATE_H_ */

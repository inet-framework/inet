/*
 * InitialSentConnectionState.h
 *
 *  Created on: 18 Feb 2025
 *      Author: msvoelker
 */

#ifndef INET_TRANSPORTLAYER_QUIC_CONNECTIONSTATE_INITIALSENTCONNECTIONSTATE_H_
#define INET_TRANSPORTLAYER_QUIC_CONNECTIONSTATE_INITIALSENTCONNECTIONSTATE_H_

#include "ConnectionState.h"

namespace inet {
namespace quic {

class InitialSentConnectionState: public ConnectionState {
public:
    InitialSentConnectionState(Connection *context) : ConnectionState(context) {
        name = "InitialSent";
    }

    virtual ConnectionState *processInitialPacket(const Ptr<const InitialPacketHeader>& packetHeader, Packet *pkt);
    //virtual void processAckFrame(const Ptr<const AckFrameHeader>& frameHeader);
    //virtual ConnectionState *processLossDetectionTimeout(cMessage *msg);
    //virtual ConnectionState *processAckDelayTimeout(cMessage *msg);
    void processAckFrame(const Ptr<const AckFrameHeader>& frameHeader, PacketNumberSpace pnSpace);
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_CONNECTIONSTATE_INITIALSENTCONNECTIONSTATE_H_ */

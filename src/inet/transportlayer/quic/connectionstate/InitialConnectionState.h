/*
 * InitialConnectionState.h
 *
 *  Created on: 7 Feb 2025
 *      Author: msvoelker
 */

#ifndef INET_TRANSPORTLAYER_QUIC_CONNECTIONSTATE_INITIALCONNECTIONSTATE_H_
#define INET_TRANSPORTLAYER_QUIC_CONNECTIONSTATE_INITIALCONNECTIONSTATE_H_

#include "ConnectionState.h"

namespace inet {
namespace quic {

class InitialConnectionState: public ConnectionState {
public:
    InitialConnectionState(Connection *context) : ConnectionState(context) {
        name = "Initial";
    }

    virtual ConnectionState *processConnectAppCommand(cMessage *msg) override;
    virtual ConnectionState *processInitialPacket(const Ptr<const InitialPacketHeader>& packetHeader, Packet *pkt) override;
    virtual void processCryptoFrame(const Ptr<const CryptoFrameHeader>& frameHeader, Packet *pkt) override;
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_CONNECTIONSTATE_INITIALCONNECTIONSTATE_H_ */

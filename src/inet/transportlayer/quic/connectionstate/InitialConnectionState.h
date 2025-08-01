//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

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
    virtual ConnectionState *processConnectAndSendAppCommand(cMessage *msg) override;
    virtual ConnectionState *processInitialPacket(const Ptr<const InitialPacketHeader>& packetHeader, Packet *pkt) override;
    virtual void processCryptoFrame(const Ptr<const CryptoFrameHeader>& frameHeader, Packet *pkt) override;

private:
    void generateAndSetTempDstConnectionId();
    void setConnectionKeysFromInitialRandom(uint64_t dstConnId);
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_CONNECTIONSTATE_INITIALCONNECTIONSTATE_H_ */

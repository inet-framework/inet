//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_TRANSPORTLAYER_QUIC_QUEUEAPPSOCKET_H_
#define INET_TRANSPORTLAYER_QUIC_QUEUEAPPSOCKET_H_

#include "AppSocket.h"

namespace inet {
namespace quic {

class QueueAppSocket: public AppSocket {
public:
    QueueAppSocket(Quic *quicSimpleMod);
    virtual ~QueueAppSocket();

    virtual void sendIndication(Indication *indication) override;
    virtual void processAppCommand(cMessage *msg) override;
    virtual void sendPacket(Packet *pkt) override;

    std::list<Indication *> getIndications();

private:
    std::list<Indication *> indications;
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_QUEUEAPPSOCKET_H_ */

//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "QueueAppSocket.h"

namespace inet {
namespace quic {

QueueAppSocket::QueueAppSocket(Quic *quicSimpleMod) : AppSocket(quicSimpleMod, -1) { }

QueueAppSocket::~QueueAppSocket() { }

void QueueAppSocket::sendIndication(Indication *indication)
{
    indications.push_back(indication);
}

void QueueAppSocket::sendPacket(Packet *pkt)
{
    throw cRuntimeError("QueueAppSocket::sendPacket: cannot send packets with a QueueAppSocket.");
}

void QueueAppSocket::processAppCommand(cMessage *msg)
{
    throw cRuntimeError("QueueAppSocket::processAppCommand: cannot process app commands in QueueAppSocket.");
}

std::list<Indication *> QueueAppSocket::getIndications()
{
    return indications;
}

} /* namespace quic */
} /* namespace inet */

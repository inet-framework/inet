//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/tcp/flavours/TcpNewReno2.h"

#include "inet/transportlayer/tcp/flavours/Rfc6675.h"

namespace inet {
namespace tcp {

Register_Class(TcpNewReno2);

// 1) Initialization of TCP protocol control block:
//    When the TCP protocol control block is initialized, recover is
//    set to the initial send sequence number.

TcpNewReno2::TcpNewReno2() : TcpTahoeRenoFamily(),
    state((TcpNewReno2StateVariables *&)TcpAlgorithm::state)
{
}

void TcpNewReno2::processRexmitTimer(TcpEventCode& event)
{
    TcpTahoeRenoFamily::processRexmitTimer(event);

    if (event == TCP_E_ABORT)
        return;
}

void TcpNewReno2::receivedDataAck(uint32_t firstSeqAcked)
{
    recovery->receivedDataAck(firstSeqAcked);
    congestionControl->receivedDataAck(firstSeqAcked);
    sendData(false);
}

void TcpNewReno2::receivedDuplicateAck()
{
    recovery->receivedDuplicateAck();
    congestionControl->receivedDuplicateAck();
}

} // namespace tcp
} // namespace inet


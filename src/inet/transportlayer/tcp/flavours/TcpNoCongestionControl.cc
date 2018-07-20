//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/transportlayer/tcp/flavours/TcpNoCongestionControl.h"
#include "inet/transportlayer/tcp/Tcp.h"

namespace inet {

namespace tcp {

Register_Class(TcpNoCongestionControl);

TcpNoCongestionControl::TcpNoCongestionControl() : TcpBaseAlg(),
    state((TcpNoCongestionControlStateVariables *&)TcpAlgorithm::state)
{
}

void TcpNoCongestionControl::initialize()
{
    TcpBaseAlg::initialize();

    // set congestion window to a practically infinite value
    state->snd_cwnd = 0x7fffffff;
}

void TcpNoCongestionControl::processRexmitTimer(TcpEventCode& event)
{
    TcpBaseAlg::processRexmitTimer(event);

    if (event == TCP_E_ABORT)
        return;

    // Tahoe-style retransmission: only one segment
    conn->retransmitOneSegment(true);
}

void TcpNoCongestionControl::receivedDataAck(uint32 firstSeqAcked)
{
    TcpBaseAlg::receivedDataAck(firstSeqAcked);

    // ack may have freed up some room in the window, try sending
    sendData(false);
}

} // namespace tcp

} // namespace inet


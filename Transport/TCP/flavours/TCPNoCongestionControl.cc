//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "TCPNoCongestionControl.h"
#include "TCP.h"

Register_Class(TCPNoCongestionControl);

TCPNoCongestionControl::TCPNoCongestionControl() : TCPBaseAlg(),
  state((TCPNoCongestionControlStateVariables *&)TCPAlgorithm::state)
{
}

void TCPNoCongestionControl::initialize()
{
    TCPBaseAlg::initialize();

    // set congestion window to a practically infinite value
    state->snd_cwnd = 0x7fffffff;
}

void TCPNoCongestionControl::processRexmitTimer(TCPEventCode& event)
{
    TCPBaseAlg::processRexmitTimer(event);
    if (event==TCP_E_ABORT)
        return;

    // Tahoe-style retransmission: only one segment
    conn->retransmitOneSegment();
}

void TCPNoCongestionControl::receivedDataAck(uint32 firstSeqAcked)
{
    TCPBaseAlg::receivedDataAck(firstSeqAcked);

    // ack may have freed up some room in the window, try sending
    sendData();
}


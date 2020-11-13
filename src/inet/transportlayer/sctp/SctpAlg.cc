//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2010-2012 Thomas Dreibholz
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "inet/transportlayer/sctp/SctpAlg.h"

#include "inet/transportlayer/sctp/Sctp.h"

namespace inet {
namespace sctp {

Register_Class(SctpAlg);

SctpAlg::SctpAlg() : SctpAlgorithm()
{
    state = nullptr;
}

SctpAlg::~SctpAlg()
{
    EV_DEBUG << "Destructor SctpAlg" << endl;
    // Note: don't delete "state" here, it'll be deleted from SctpAssociation
}

SctpStateVariables *SctpAlg::createStateVariables()
{
    ASSERT(state == nullptr);
    state = new SctpAlgStateVariables();
    return state;
}

void SctpAlg::established(bool active)
{
    if (active) {
        EV_INFO << "Completing connection: sending DATA" << endl;
    }
}

void SctpAlg::connectionClosed()
{
}

void SctpAlg::processTimer(cMessage *timer, SctpEventCode& event)
{
    EV_INFO << "no extra timers in this SCTP variant" << endl;
}

void SctpAlg::sendCommandInvoked(SctpPathVariables *path)
{
    if (state->allowCMT) {
        assoc->sendOnAllPaths(path);
    }
    else {
        assoc->sendOnPath(path);
    }
}

void SctpAlg::receivedDataAck(uint32_t)
{
}

void SctpAlg::receivedDuplicateAck()
{
    EV_INFO << "Duplicate ACK #" << endl;
}

void SctpAlg::receivedAckForDataNotYetSent(uint32_t seq)
{
    EV_INFO << "ACK acks something not yet sent, sending immediate ACK" << endl;
}

void SctpAlg::sackSent()
{
}

void SctpAlg::dataSent(uint32_t)
{
}

} // namespace sctp
} // namespace inet


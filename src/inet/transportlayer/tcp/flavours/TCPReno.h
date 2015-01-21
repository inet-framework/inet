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

#ifndef __INET_TCPRENO_H
#define __INET_TCPRENO_H

#include "inet/common/INETDefs.h"

#include "inet/transportlayer/tcp/flavours/TCPTahoeRenoFamily.h"

namespace inet {

namespace tcp {

/**
 * State variables for TCPReno.
 */
typedef TCPTahoeRenoFamilyStateVariables TCPRenoStateVariables;

/**
 * Implements TCP Reno.
 */
class INET_API TCPReno : public TCPTahoeRenoFamily
{
  protected:
    TCPRenoStateVariables *& state;    // alias to TCLAlgorithm's 'state'

    /** Create and return a TCPRenoStateVariables object. */
    virtual TCPStateVariables *createStateVariables() override
    {
        return new TCPRenoStateVariables();
    }

    /** Utility function to recalculate ssthresh */
    virtual void recalculateSlowStartThreshold();

    /** Redefine what should happen on retransmission */
    virtual void processRexmitTimer(TCPEventCode& event) override;

  public:
    /** Ctor */
    TCPReno();

    /** Redefine what should happen when data got acked, to add congestion window management */
    virtual void receivedDataAck(uint32 firstSeqAcked) override;

    /** Redefine what should happen when dupAck was received, to add congestion window management */
    virtual void receivedDuplicateAck() override;
};

} // namespace tcp

} // namespace inet

#endif // ifndef __INET_TCPRENO_H


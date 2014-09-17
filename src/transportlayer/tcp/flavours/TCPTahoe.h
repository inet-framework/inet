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

#ifndef __INET_TCPTAHOE_H
#define __INET_TCPTAHOE_H

#include "inet/common/INETDefs.h"

#include "inet/transportlayer/tcp/flavours/TCPTahoeRenoFamily.h"

namespace inet {

namespace tcp {

/**
 * State variables for TCPTahoe.
 */
typedef TCPTahoeRenoFamilyStateVariables TCPTahoeStateVariables;

/**
 * Implements Tahoe.
 */
class INET_API TCPTahoe : public TCPTahoeRenoFamily
{
  protected:
    TCPTahoeStateVariables *& state;    // alias to TCLAlgorithm's 'state'

  protected:
    /** Create and return a TCPTahoeStateVariables object. */
    virtual TCPStateVariables *createStateVariables()
    {
        return new TCPTahoeStateVariables();
    }

    /** Utility function to recalculate ssthresh */
    virtual void recalculateSlowStartThreshold();

    /** Redefine what should happen on retransmission */
    virtual void processRexmitTimer(TCPEventCode& event);

  public:
    /** Ctor */
    TCPTahoe();

    /** Redefine what should happen when data got acked, to add congestion window management */
    virtual void receivedDataAck(uint32 firstSeqAcked);

    /** Redefine what should happen when dupAck was received, to add congestion window management */
    virtual void receivedDuplicateAck();
};

} // namespace tcp

} // namespace inet

#endif // ifndef __INET_TCPTAHOE_H


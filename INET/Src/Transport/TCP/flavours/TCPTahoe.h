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

#ifndef __TCPTAHOE_H
#define __TCPTAHOE_H

#include <omnetpp.h>
#include "TCPTahoeRenoFamily.h"


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
    TCPTahoeStateVariables *&state; // alias to TCLAlgorithm's 'state'

  protected:
    /** Create and return a TCPTahoeStateVariables object. */
    virtual TCPStateVariables *createStateVariables() {
        return new TCPTahoeStateVariables();
    }

    /** Utility function to recalculate ssthresh */
    void recalculateSlowStartThreshold();

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

#endif



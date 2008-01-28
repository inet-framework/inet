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

#ifndef __TCPRENO_H
#define __TCPRENO_H

#include <omnetpp.h>
#include "TCPTahoeRenoFamily.h"


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
    TCPRenoStateVariables *&state; // alias to TCLAlgorithm's 'state'

    /** Create and return a TCPRenoStateVariables object. */
    virtual TCPStateVariables *createStateVariables() {
        return new TCPRenoStateVariables();
    }

    /** Utility function to recalculate ssthresh */
    void recalculateSlowStartThreshold();

    /** Redefine what should happen on retransmission */
    virtual void processRexmitTimer(TCPEventCode& event);

  public:
    /** Ctor */
    TCPReno();

    /** Redefine what should happen when data got acked, to add congestion window management */
    virtual void receivedDataAck(uint32 firstSeqAcked);

    /** Redefine what should happen when dupAck was received, to add congestion window management */
    virtual void receivedDuplicateAck();
};

#endif



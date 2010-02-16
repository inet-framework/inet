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

#ifndef __INET_DUMBTCP_OLD_H
#define __INET_DUMBTCP_OLD_H

#include <omnetpp.h>
#include "TCPAlgorithm_old.h"

namespace tcp_old {

/**
 * State variables for DumbTCP.
 */
class INET_API DumbTCPStateVariables : public TCPStateVariables
{
  public:
    //...
};


/**
 * A very-very basic TCPAlgorithm implementation, with hardcoded
 * retransmission timeout and no other sophistication. It can be
 * used to demonstrate what happened if there was no adaptive
 * timeout calculation, delayed acks, silly window avoidance,
 * congestion control, etc.
 */
class INET_API DumbTCP : public TCPAlgorithm
{
  protected:
    DumbTCPStateVariables *&state; // alias to TCLAlgorithm's 'state'

    cMessage *rexmitTimer;  // retransmission timer

  protected:
    /** Creates and returns a DumbTCPStateVariables object. */
    virtual TCPStateVariables *createStateVariables() {
        return new DumbTCPStateVariables();
    }

  public:
    /** Ctor */
    DumbTCP();

    virtual ~DumbTCP();

    virtual void initialize();

    virtual void established(bool active);

    virtual void connectionClosed();

    virtual void processTimer(cMessage *timer, TCPEventCode& event);

    virtual void sendCommandInvoked();

    virtual void receivedOutOfOrderSegment();

    virtual void receiveSeqChanged();

    virtual void receivedDataAck(uint32 firstSeqAcked);

    virtual void receivedDuplicateAck();

    virtual void receivedAckForDataNotYetSent(uint32 seq);

    virtual void ackSent();

    virtual void dataSent(uint32 fromseq);

    virtual void restartRexmitTimer();

};

}
#endif



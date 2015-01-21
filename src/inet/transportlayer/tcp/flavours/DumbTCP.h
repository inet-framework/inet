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

#ifndef __INET_DUMBTCP_H
#define __INET_DUMBTCP_H

#include "inet/common/INETDefs.h"

#include "inet/transportlayer/tcp/TCPAlgorithm.h"

namespace inet {

namespace tcp {

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
    DumbTCPStateVariables *& state;    // alias to TCLAlgorithm's 'state'

    cMessage *rexmitTimer;    // retransmission timer

  protected:
    /** Creates and returns a DumbTCPStateVariables object. */
    virtual TCPStateVariables *createStateVariables() override
    {
        return new DumbTCPStateVariables();
    }

  public:
    /** Ctor */
    DumbTCP();

    virtual ~DumbTCP();

    virtual void initialize() override;

    virtual void established(bool active) override;

    virtual void connectionClosed() override;

    virtual void processTimer(cMessage *timer, TCPEventCode& event) override;

    virtual void sendCommandInvoked() override;

    virtual void receivedOutOfOrderSegment() override;

    virtual void receiveSeqChanged() override;

    virtual void receivedDataAck(uint32 firstSeqAcked) override;

    virtual void receivedDuplicateAck() override;

    virtual void receivedAckForDataNotYetSent(uint32 seq) override;

    virtual void ackSent() override;

    virtual void dataSent(uint32 fromseq) override;

    virtual void segmentRetransmitted(uint32 fromseq, uint32 toseq) override;

    virtual void restartRexmitTimer() override;

    virtual void rttMeasurementCompleteUsingTS(uint32 echoedTS) override;
};

} // namespace tcp

} // namespace inet

#endif // ifndef __INET_DUMBTCP_H


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

#ifndef __INET_TCPNOCONGESTIONCONTROL_H
#define __INET_TCPNOCONGESTIONCONTROL_H

#include "inet/common/INETDefs.h"
#include "inet/transportlayer/tcp/flavours/TcpBaseAlg.h"

namespace inet {
namespace tcp {

/**
 * State variables for TcpNoCongestionControl.
 */
typedef TcpBaseAlgStateVariables TcpNoCongestionControlStateVariables;

/**
 * TCP with no congestion control (i.e. congestion window kept very large).
 * Can be used to demonstrate effect of lack of congestion control.
 */
class INET_API TcpNoCongestionControl : public TcpBaseAlg
{
  protected:
    TcpNoCongestionControlStateVariables *& state;    // alias to TcpAlgorithm's 'state'

    /** Create and return a TcpNoCongestionControlStateVariables object. */
    virtual TcpStateVariables *createStateVariables() override
    {
        return new TcpNoCongestionControlStateVariables();
    }

    /** Redefine what should happen on retransmission */
    virtual void processRexmitTimer(TcpEventCode& event) override;

  public:
    /** Ctor */
    TcpNoCongestionControl();

    /** Initialize state vars */
    virtual void initialize() override;

    /** Redefine what should happen when data got acked, to add congestion window management */
    virtual void receivedDataAck(uint32 firstSeqAcked) override;

    virtual void established(bool active) override;

    virtual bool sendData(bool sendCommandInvoked) override;
};

} // namespace tcp
} // namespace inet

#endif // ifndef __INET_TCPNOCONGESTIONCONTROL_H


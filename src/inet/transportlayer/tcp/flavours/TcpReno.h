//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPRENO_H
#define __INET_TCPRENO_H

#include "inet/transportlayer/tcp/flavours/TcpClassicAlgorithmBase.h"

namespace inet {
namespace tcp {

/**
 * State variables for TcpReno.
 */
typedef TcpClassicAlgorithmBaseStateVariables TcpRenoStateVariables;

/**
 * Implements TCP Reno.
 */
class INET_API TcpReno : public TcpClassicAlgorithmBase
{
  protected:
    TcpRenoStateVariables *& state; // alias to TCLAlgorithm's 'state'

    /** Create and return a TcpRenoStateVariables object. */
    virtual TcpStateVariables *createStateVariables() override
    {
        return new TcpRenoStateVariables();
    }

    virtual ITcpCongestionControl *createCongestionControl() override;
    virtual ITcpRecovery *createRecovery() override;

    /**
     * With SACK, Reno is in loss recovery while lossRecovery is set (RFC 6675). Without
     * it, Reno is in fast recovery from the third duplicate ACK to the next ACK of new data.
     */

  public:
    /** Ctor */
    TcpReno();

    /** Recovers with SACK by RFC 6675 (Rfc6675Recovery) when SACK is negotiated */
    virtual bool supportsSackRecovery() const override { return true; }
};

} // namespace tcp
} // namespace inet

#endif


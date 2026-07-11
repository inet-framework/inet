//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_TCPCUBIC_H
#define __INET_TCPCUBIC_H

#include "inet/transportlayer/tcp/flavours/TcpCubicState_m.h"
#include "inet/transportlayer/tcp/flavours/TcpReno.h"

namespace inet {
namespace tcp {

/**
 * Implements CUBIC congestion control (RFC 9438, ported from Linux tcp_cubic.c),
 * including the HyStart hybrid slow-start exit. CUBIC reuses TcpReno's
 * fast-retransmit / RFC 3517 SACK recovery machinery and overrides only the
 * ssthresh reduction (cubic multiplicative decrease with fast convergence) and
 * the window-growth law.
 */
class INET_API TcpCubic : public TcpReno
{
  protected:
    TcpCubicStateVariables *& state;

    /** Create and return a TcpCubicStateVariables object. */
    virtual TcpStateVariables *createStateVariables() override
    {
        return new TcpCubicStateVariables();
    }

    virtual void initialize() override;

    /** Resets the cubic epoch state (Linux bictcp_reset). */
    virtual void cubicReset();

    /** Resets the HyStart per-round state (Linux bictcp_hystart_reset). */
    virtual void hystartReset();

  public:
    /** Constructor */
    TcpCubic();

    virtual void established(bool active) override;

    /** CUBIC multiplicative decrease (beta) with fast convergence. */
    virtual void recalculateSlowStartThreshold() override;

    virtual void processRexmitTimer(TcpEventCode& event) override;
};

} // namespace tcp
} // namespace inet

#endif

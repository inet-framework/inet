//
// Copyright (C) 2020 Marcel Marek
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_DCTCP_H
#define __INET_DCTCP_H

#include "inet/transportlayer/tcp/flavours/DcTcpFamily.h"
#include "inet/transportlayer/tcp/flavours/TcpReno.h"

namespace inet {
namespace tcp {

/**
 * State variables for DcTcp.
 */
typedef DcTcpFamilyStateVariables DcTcpStateVariables;

/**
 * Implements DCTCP.
 */
class INET_API DcTcp : public TcpReno
{
  protected:
    DcTcpStateVariables *& state;

    static simsignal_t loadSignal; // will record load
    static simsignal_t calcLoadSignal; // will record total number of RTOs
    static simsignal_t markingProbSignal; // will record marking probability

    /** Create and return a DcTcpStateVariables object. */
    virtual TcpStateVariables *createStateVariables() override
    {
        return new DcTcpStateVariables();
    }

    virtual void initialize() override;

  public:
    /** Constructor */
    DcTcp();

    /** Redefine what should happen when data got acked, to add congestion window management */
    virtual void receivedDataAck(uint32_t firstSeqAcked) override;

    virtual bool shouldMarkAck() override;

    virtual void processEcnInEstablished() override;
};

} // namespace tcp
} // namespace inet

#endif


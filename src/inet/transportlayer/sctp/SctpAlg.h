//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2010-2012 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCTPALG_H
#define __INET_SCTPALG_H

#include "inet/transportlayer/sctp/SctpAlgorithm.h"

namespace inet {
namespace sctp {

/**
 * State variables for SctpAlg.
 */
class INET_API SctpAlgStateVariables : public SctpStateVariables
{
  public:
    // ...
};

class INET_API SctpAlg : public SctpAlgorithm
{
  protected:
    SctpAlgStateVariables *state;

  public:
    /**
     * Ctor.
     */
    SctpAlg();

    /**
     * Virtual dtor.
     */
    virtual ~SctpAlg();

    /**
     * Creates and returns a SctpStateVariables object.
     */
    virtual SctpStateVariables *createStateVariables() override;

    virtual void established(bool active) override;

    virtual void connectionClosed() override;

    virtual void processTimer(cMessage *timer, SctpEventCode& event) override;

    virtual void sendCommandInvoked(SctpPathVariables *path) override;

    virtual void receivedDataAck(uint32_t firstSeqAcked) override;

    virtual void receivedDuplicateAck() override;

    virtual void receivedAckForDataNotYetSent(uint32_t seq) override;

    virtual void sackSent() override;

    virtual void dataSent(uint32_t fromseq) override;
};

} // namespace sctp
} // namespace inet

#endif


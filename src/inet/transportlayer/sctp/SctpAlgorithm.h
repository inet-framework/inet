//
// Copyright (C) 2008 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCTPALGORITHM_H
#define __INET_SCTPALGORITHM_H

#include "inet/transportlayer/sctp/SctpAssociation.h"
#include "inet/transportlayer/sctp/SctpQueue.h"

namespace inet {
namespace sctp {

/**
 * Abstract base class for SCTP algorithms which encapsulate all behaviour
 * during data transfer state: flavour of congestion control, fast
 * retransmit/recovery, selective acknowledgement etc. Subclasses
 * may implement various sets and flavours of the above algorithms.
 */
class INET_API SctpAlgorithm : public cObject
{
  protected:
    SctpAssociation *assoc; // we belong to this association
    SctpQueue *transmissionQ;
    SctpQueue *retransmissionQ;

  public:
    /**
     * Ctor.
     */
    SctpAlgorithm() { assoc = nullptr; transmissionQ = nullptr; retransmissionQ = nullptr; }

    /**
     * Virtual dtor.
     */
    virtual ~SctpAlgorithm() {}

    void setAssociation(SctpAssociation *_assoc)
    {
        assoc = _assoc;
        transmissionQ = assoc->getTransmissionQueue();
        retransmissionQ = assoc->getRetransmissionQueue();
    }

    virtual void initialize() {}

    virtual SctpStateVariables *createStateVariables() = 0;

    virtual void established(bool active) = 0;

    virtual void connectionClosed() = 0;

    virtual void processTimer(cMessage *timer, SctpEventCode& event) = 0;

    virtual void sendCommandInvoked(SctpPathVariables *path) = 0;

    virtual void receivedDataAck(uint32_t firstSeqAcked) = 0;

    virtual void receivedDuplicateAck() = 0;

    virtual void receivedAckForDataNotYetSent(uint32_t seq) = 0;

    virtual void sackSent() = 0;

    virtual void dataSent(uint32_t fromseq) = 0;
};

} // namespace sctp
} // namespace inet

#endif


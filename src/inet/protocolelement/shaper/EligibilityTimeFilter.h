//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ELIGIBILITYTIMEFILTER_H
#define __INET_ELIGIBILITYTIMEFILTER_H

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/queueing/base/PacketFilterBase.h"

namespace inet {

using namespace inet::queueing;

extern template class ClockUserModuleMixin<PacketFilterBase>;

class INET_API EligibilityTimeFilter : public ClockUserModuleMixin<PacketFilterBase>, public TransparentProtocolRegistrationListener
{
  protected:
    clocktime_t maxResidenceTime = -1;

  protected:
    virtual void initialize(int stage) override;

    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;

    virtual bool matchesPacket(const Packet *packet) const override;

  public:
    // the verdict changes with time without an event, so a packet that the
    // provider offers in pull mode could expire without a drop
    virtual bool supportsPacketPulling(const cGate *gate) const override { return false; }
};

} // namespace inet

#endif


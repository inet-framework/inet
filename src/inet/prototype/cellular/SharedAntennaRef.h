//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SHAREDANTENNAREF_H
#define __INET_SHAREDANTENNAREF_H

#include "inet/common/Module.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IAntenna.h"

namespace inet {

namespace cellular {

using namespace inet::physicallayer;

/**
 * Workaround for C3 (several radios share one antenna): Radio always uses its own submodule
 * named "antenna", so each radio gets this module there, and it forwards to the shared antenna
 * that `antennaModule` names.
 */
class INET_API SharedAntennaRef : public Module, public IAntenna
{
  protected:
    mutable const IAntenna *antenna = nullptr;

  public:
    // resolved on first use, because radios ask for the mobility during their own initialization
    const IAntenna *getAntenna() const;

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual IMobility *getMobility() const override { return getAntenna()->getMobility(); }
    virtual bool isDirectional() const override { return getAntenna()->isDirectional(); }
    virtual Ptr<const IAntennaGain> getGain() const override { return getAntenna()->getGain(); }
    virtual int getNumAntennas() const override { return getAntenna()->getNumAntennas(); }
};

} // namespace cellular

} // namespace inet

#endif


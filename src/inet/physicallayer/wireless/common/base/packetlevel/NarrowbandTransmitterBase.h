//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NARROWBANDTRANSMITTERBASE_H
#define __INET_NARROWBANDTRANSMITTERBASE_H

#include "inet/physicallayer/wireless/common/base/packetlevel/TransmitterBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IModulation.h"

namespace inet {

namespace physicallayer {

class INET_API NarrowbandTransmitterBase : public TransmitterBase
{
  protected:
    const IModulation *modulation;
    Hz centerFrequency;
    Hz bandwidth;

  protected:
    virtual void initialize(int stage) override;

    virtual Hz computeCenterFrequency(const Packet *packet) const;
    virtual Hz computeBandwidth(const Packet *packet) const;

  public:
    NarrowbandTransmitterBase();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const IModulation *getModulation() const { return modulation; }
    virtual void setModulation(const IModulation *modulation) { this->modulation = modulation; }

    virtual Hz getCenterFrequency() const { return centerFrequency; }
    virtual void setCenterFrequency(Hz centerFrequency) { this->centerFrequency = centerFrequency; }

    virtual Hz getBandwidth() const { return bandwidth; }
    virtual void setBandwidth(Hz bandwidth) { this->bandwidth = bandwidth; }
};

} // namespace physicallayer

} // namespace inet

#endif


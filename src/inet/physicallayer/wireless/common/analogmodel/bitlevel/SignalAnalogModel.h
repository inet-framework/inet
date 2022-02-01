//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SIGNALANALOGMODEL_H
#define __INET_SIGNALANALOGMODEL_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalAnalogModel.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioSignal.h"

namespace inet {

namespace physicallayer {

class INET_API SignalAnalogModel : public virtual ISignalAnalogModel
{
  protected:
    const simtime_t duration;

  public:
    SignalAnalogModel(const simtime_t duration);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const simtime_t getDuration() const override { return duration; }
};

class INET_API NarrowbandSignalAnalogModel : public SignalAnalogModel, public virtual INarrowbandSignal
{
  protected:
    const Hz centerFrequency;
    const Hz bandwidth;

  public:
    NarrowbandSignalAnalogModel(const simtime_t duration, Hz centerFrequency, Hz bandwidth);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual Hz getCenterFrequency() const override { return centerFrequency; }
    virtual Hz getBandwidth() const override { return bandwidth; }
};

} // namespace physicallayer

} // namespace inet

#endif


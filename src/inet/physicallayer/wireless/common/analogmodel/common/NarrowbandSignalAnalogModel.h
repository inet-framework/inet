//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NARROWBANDSIGNALANALOGMODEL_H
#define __INET_NARROWBANDSIGNALANALOGMODEL_H

#include "inet/physicallayer/wireless/common/analogmodel/common/SignalAnalogModel.h"
#include "../../contract/packetlevel/INarrowbandSignalAnalogModel.h"

namespace inet {

namespace physicallayer {

class INET_API NarrowbandSignalAnalogModel : public SignalAnalogModel, public virtual INarrowbandSignalAnalogModel
{
  protected:
    const Hz centerFrequency;
    const Hz bandwidth;

  public:
    NarrowbandSignalAnalogModel(simtime_t preambleDuration, simtime_t headerDuration, simtime_t dataDuration, Hz centerFrequency, Hz bandwidth);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual Hz getCenterFrequency() const override { return centerFrequency; }
    virtual Hz getBandwidth() const override { return bandwidth; }
};

} // namespace physicallayer

} // namespace inet

#endif


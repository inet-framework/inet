//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ISIGNALANALOGMODEL_H
#define __INET_ISIGNALANALOGMODEL_H

#include "inet/common/IPrintableObject.h"

namespace inet {

namespace physicallayer {

/**
 * This purely virtual interface provides an abstraction for different radio
 * signal models in the analog domain.
 */
class INET_API ISignalAnalogModel : public IPrintableObject
{
  public:
    /**
     * Returns the duration of the PHY frame preamble part.
     */
    virtual const simtime_t getPreambleDuration() const = 0;

    /**
     * Returns the duration of the PHY frame header part.
     */
    virtual const simtime_t getHeaderDuration() const = 0;

    /**
     * Returns the duration of the PHY frame data part.
     */
    virtual const simtime_t getDataDuration() const = 0;

    /**
     * Returns the total duration of the PHY frame that includes the PHY frame header and data parts.
     */
    virtual const simtime_t getDuration() const = 0;
};

class INET_API ITransmissionAnalogModel : public virtual ISignalAnalogModel
{
};

class INET_API IReceptionAnalogModel : public virtual ISignalAnalogModel
{
};

} // namespace physicallayer

} // namespace inet

#endif


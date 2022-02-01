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


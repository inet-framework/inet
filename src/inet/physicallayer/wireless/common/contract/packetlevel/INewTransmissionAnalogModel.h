//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INEWTRANSMISSIONANALOGMODEL_H
#define __INET_INEWTRANSMISSIONANALOGMODEL_H

#include "inet/common/IPrintableObject.h"
#include "inet/common/Units.h"

namespace inet {

namespace physicallayer {

using namespace inet::units::values;

class INET_API INewTransmissionAnalogModel : public IPrintableObject
{
};

class INET_API INarrowbandTransmissionAnalogModel : public INewTransmissionAnalogModel
{
  public:
    virtual const Hz getBandwidth() const = 0;
    virtual const Hz getCenterFrequency() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif


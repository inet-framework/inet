//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IDIGITALANALOGCONVERTER_H
#define __INET_IDIGITALANALOGCONVERTER_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalAnalogModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalSampleModel.h"

namespace inet {
namespace physicallayer {

class INET_API IDigitalAnalogConverter : public IPrintableObject
{
  public:
    virtual const ITransmissionAnalogModel *convertDigitalToAnalog(const ITransmissionSampleModel *sampleModel) const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif


//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IANALOGDIGITALCONVERTER_H
#define __INET_IANALOGDIGITALCONVERTER_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalAnalogModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalSampleModel.h"

namespace inet {
namespace physicallayer {

class INET_API IAnalogDigitalConverter : public IPrintableObject
{
  public:
    virtual const IReceptionSampleModel *convertAnalogToDigital(const IReceptionAnalogModel *analogModel) const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif


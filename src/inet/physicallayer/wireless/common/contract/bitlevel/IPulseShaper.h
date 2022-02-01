//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPULSESHAPER_H
#define __INET_IPULSESHAPER_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalSampleModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalSymbolModel.h"

namespace inet {
namespace physicallayer {

class INET_API IPulseShaper : public IPrintableObject
{
  public:
    virtual const ITransmissionSampleModel *shape(const ITransmissionSymbolModel *symbolModel) const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif


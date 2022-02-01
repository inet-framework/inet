//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPULSEFILTER_H
#define __INET_IPULSEFILTER_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalSampleModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalSymbolModel.h"

namespace inet {
namespace physicallayer {

class INET_API IPulseFilter : public IPrintableObject
{
  public:
    virtual const IReceptionSymbolModel *filter(const IReceptionSampleModel *sampleModel) const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif


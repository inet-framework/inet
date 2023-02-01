//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_UNITDISKRECEPTIONANALOGMODEL_H
#define __INET_UNITDISKRECEPTIONANALOGMODEL_H

#include "inet/common/Units.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/INewReceptionAnalogModel.h"

namespace inet {

namespace physicallayer {

using namespace inet::units::values;

class INET_API UnitDiskReceptionAnalogModel : public INewReceptionAnalogModel
{
  public:
    UnitDiskReceptionAnalogModel() {}
};

} // namespace physicallayer

} // namespace inet

#endif


//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ENERGYSTORAGEBASE_H
#define __INET_ENERGYSTORAGEBASE_H

#include "inet/power/base/EnergySinkBase.h"
#include "inet/power/base/EnergySourceBase.h"
#include "inet/power/contract/IEnergyStorage.h"

namespace inet {
namespace power {

class INET_API EnergyStorageBase : public cSimpleModule, public EnergySourceBase, public EnergySinkBase, public virtual IEnergyStorage
{
};

} // namespace power
} // namespace inet

#endif


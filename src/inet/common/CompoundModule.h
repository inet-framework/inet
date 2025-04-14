//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_COMPOUNDMODULE_H
#define __INET_COMPOUNDMODULE_H

#include <type_traits>

#include "inet/common/ModuleMixin.h"

namespace inet {

class INET_API CompoundModule : public ModuleMixin<cModule>
{
};

} // namespace inet

#endif

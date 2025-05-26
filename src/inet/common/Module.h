//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MODULE_H
#define __INET_MODULE_H

#include <type_traits>

#include "inet/common/ModuleMixin.h"

namespace inet {

class INET_API Module : public ModuleMixin<cModule>
{
};

} // namespace inet

#endif

//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CLOCKUSERMODULEBASE_H
#define __INET_CLOCKUSERMODULEBASE_H

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/common/SimpleModule.h"

namespace inet {

extern template class ClockUserModuleMixin<SimpleModule>;

class INET_API ClockUserModuleBase : public ClockUserModuleMixin<SimpleModule>
{
};

} // namespace inet

#endif


//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_OPERATIONALBASE_H
#define __INET_OPERATIONALBASE_H

#include "inet/common/lifecycle/OperationalMixin.h"

namespace inet {

class INET_API OperationalBase : public OperationalMixin<cSimpleModule>
{
};

} // namespace inet

#endif


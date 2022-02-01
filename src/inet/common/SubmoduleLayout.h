//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SUBMODULELAYOUT_H
#define __INET_SUBMODULELAYOUT_H

#include "inet/common/INETDefs.h"

namespace inet {

INET_API void layoutSubmodulesWithoutGates(cModule *module, int dimensionIndex = 1, double spacing = 100);

INET_API void layoutSubmodulesWithGates(cModule *module, int dimensionIndex = 1, double moduleSpacing = 100);

INET_API void layoutSubmodules(cModule *module, int dimensionIndex = 1, double moduleSpacing = 100);

} // namespace inet

#endif


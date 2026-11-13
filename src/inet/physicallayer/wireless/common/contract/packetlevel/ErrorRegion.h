//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ERRORREGION_H
#define __INET_ERRORREGION_H

#include "inet/common/Units.h"

namespace inet {

namespace physicallayer {

using namespace inet::units::values;

/**
 * A region of the transmitted packet that an error decision of a receiver
 * found erroneous.
 */
struct INET_API ErrorRegion
{
    // the offset of the region in the transmitted packet
    b offset;
    // the length of the region
    b length;
    // the error rate of one bit of the region, at the SNIR of the time of the region
    double bitErrorRate;
};

} // namespace physicallayer

} // namespace inet

#endif


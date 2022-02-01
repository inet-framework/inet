//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/geometry/common/EulerAngles.h"

namespace inet {

const EulerAngles EulerAngles::ZERO = EulerAngles(rad(0.0), rad(0.0), rad(0.0));
const EulerAngles EulerAngles::NIL = EulerAngles(rad(NaN), rad(NaN), rad(NaN));

} // namespace inet


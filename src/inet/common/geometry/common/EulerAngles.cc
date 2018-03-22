//
// This program is property of its copyright holder. All rights reserved.
//

#include "inet/common/geometry/common/EulerAngles.h"

namespace inet {

const EulerAngles EulerAngles::ZERO = EulerAngles(rad(0.0), rad(0.0), rad(0.0));
const EulerAngles EulerAngles::NIL = EulerAngles(rad(NaN), rad(NaN), rad(NaN));

} // namespace inet


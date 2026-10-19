// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_TRANSMISSIONTIMINGSNAPSHOT_H
#define __INET_TRANSMISSIONTIMINGSNAPSHOT_H

#include <map>

#include "inet/common/INETDefs.h"

namespace inet {
namespace physicallayer {

/**
 * This value copies the physical endpoints of one transmission from the medium cache.
 * The endpoints retain the propagation result that the medium used for each radio.
 * No packet, transmission, arrival, or radio pointer remains in this value.
 * A transmissionId of -1 means that the medium no longer has the transmission.
 */
struct INET_API TransmissionTimingSnapshot
{
    int transmissionId = -1;
    int transmitterRadioId = -1;
    simtime_t startTime = -1; // First transmitted bit, in seconds.
    simtime_t endTime = -1; // Last transmitted bit, in seconds.
    std::map<int, simtime_t> arrivalEndTimes; // Radio ID to last received bit, in seconds.
};

} // namespace physicallayer
} // namespace inet

#endif

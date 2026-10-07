// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef INET_TXREQUESTID_H
#define INET_TXREQUESTID_H

#include "inet/common/INETDefs.h"

namespace inet::ieee80211 {

struct INET_API TxRequestId
{
    uint64_t epoch = 0;
    uint64_t serial = 0;
    [[nodiscard]] bool operator==(const TxRequestId& other) const { return epoch == other.epoch && serial == other.serial; }
    [[nodiscard]] bool operator!=(const TxRequestId& other) const { return !(*this == other); }
};

} // namespace inet::ieee80211

#endif

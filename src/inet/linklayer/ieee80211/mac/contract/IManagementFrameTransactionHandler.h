//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IMANAGEMENTFRAMETRANSACTIONHANDLER_H
#define __INET_IMANAGEMENTFRAMETRANSACTIONHANDLER_H

#include <cstdint>

#include "inet/common/INETDefs.h"

namespace inet {
namespace ieee80211 {

/**
 * Provides management-frame transaction lifecycle operations to management
 * protocols without exposing coordination-function implementation details.
 */
class INET_API IManagementFrameTransactionHandler
{
  public:
    virtual ~IManagementFrameTransactionHandler() {}

    /**
     * Cancels all locally queued fragments for the given management
     * transaction. An active transmission is retired at an ownership-safe
     * frame-sequence boundary.
     */
    virtual void cancelManagementTransaction(uint64_t transactionId) = 0;
};

}
}

#endif

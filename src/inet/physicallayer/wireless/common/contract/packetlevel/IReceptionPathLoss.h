//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IRECEPTIONPATHLOSS_H
#define __INET_IRECEPTIONPATHLOSS_H

#include "inet/common/IPrintableObject.h"
#include "inet/common/Ptr.h"
#include "inet/common/Units.h"

namespace inet {
namespace physicallayer {

using namespace inet::units::values;

/**
 * This interface represents the path loss of one reception, that is between
 * one transmission and one receiver, over frequency. It is created by
 * IPathLoss::computeReceptionPathLoss() when the reception is computed, and
 * it may be evaluated at any later time, any number of times.
 */
class INET_API IReceptionPathLoss : public virtual IPrintableObject
#if INET_PTR_IMPLEMENTATION == INET_INTRUSIVE_PTR
    , public IntrusivePtrCounter<IReceptionPathLoss>
#endif
{
  public:
    /**
     * Returns the loss factor at the given frequency. The value is in the
     * range [0, 1] where 1 means no loss at all and 0 means all power is
     * lost. For the same frequency the value is the same, however often and
     * however late it is asked for.
     */
    virtual double computePathLoss(Hz frequency) const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif

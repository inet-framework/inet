//
// Copyright (C) 2017 Raphael Riebl, TH Ingolstadt
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IANTENNAGAIN_H
#define __INET_IANTENNAGAIN_H

#include "inet/common/IPrintableObject.h"
#include "inet/common/Ptr.h"
#include "inet/common/geometry/common/Quaternion.h"

namespace inet {
namespace physicallayer {

/**
 * This interface represents the directional selectivity of an antenna.
 */
class INET_API IAntennaGain : public IPrintableObject
#if INET_PTR_IMPLEMENTATION == INET_INTRUSIVE_PTR
    , public IntrusivePtrCounter<IAntennaGain>
#endif
{
  public:
    /**
     * Returns the minimum possible antenna gain independent of any direction.
     */
    virtual double getMinGain() const = 0;

    /**
     * Returns the maximum possible antenna gain independent of any direction.
     */
    virtual double getMaxGain() const = 0;

    /**
     * Returns the antenna gain in the provided direction. The direction is
     * determined by rotating the X axis using the given quaternion. The direction
     * is to be interpreted in the local coordinate system of the radiation pattern.
     * This way the gain depends only on the antenna radion pattern characteristics,
     * and not on the antenna orientation determined by the antenna's mobility model.
     *
     * For transmissions, it determines how well the antenna converts input
     * power into radio waves headed in the specified direction. For receptions,
     * it determines how well the antenna converts radio waves arriving from
     * the specified direction.
     */
    virtual double computeGain(const Quaternion& direction) const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif


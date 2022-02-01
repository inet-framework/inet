//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GEOMETRICOBJECTBASE_H
#define __INET_GEOMETRICOBJECTBASE_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * This class represents a 3 dimensional geometric object positioned and oriented
 * in 3 dimensional space.
 */
class INET_API GeometricObjectBase
{
  public:
    GeometricObjectBase() {}
    virtual ~GeometricObjectBase() {}

    /**
     * Returns true if this geometric object is the same as the unspecified
     * singleton instance of this type.
     */
    virtual bool isNil() const = 0;

    /**
     * Returns true if this geometric object is not completely specified.
     */
    virtual bool isUnspecified() const = 0;
};

} // namespace inet

#endif

